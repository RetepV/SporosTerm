#include "BluetoothSerial.h"
#include "esp_spp_api.h"
#include "esp_bt.h"

#pragma once

// NOTE: BluetoothSerial also manages queues, and use send/recieve 32/512 sizes. Maybe we can make our queues
//       a lot smaller. They are only necessary because we need to cross thread boundaries.

// The send queue can be small, as we can expect our BT connected device to be fast enough to pull the data from the queue.
#define SERIAL_QUEUE_SIZE         64
#define SERIAL_QUEUE_LOW_WATER    (int)((SERIAL_QUEUE_SIZE / 100) * 10)         // 10% low water mark
#define SERIAL_QUEUE_HIGH_WATER   (int)((SERIAL_QUEUE_SIZE / 100) * 90)         // 90% high water mark
// The receive queue, though, should be a bunch larger as on low baud rates, we will not consume the queue fast enough.
// TODO: If Xon and Xoff actually works on the Bluetooth serial connection, then this can be a lot shorter.
#define BLUETOOTH_QUEUE_SIZE 1024
#define BLUETOOTH_QUEUE_LOW_WATER    (int)((BLUETOOTH_QUEUE_SIZE / 100) * 10)   // 10% low water mark
#define BLUETOOTH_QUEUE_HIGH_WATER   (int)((BLUETOOTH_QUEUE_SIZE / 100) * 90)   // 90% low water mark

#define TASK_STACK_SIZE         2048      // I really have no clue how large it should be, make it 1024. Note that this is 2KB, i.e. 1024 words.
#define TASK_PRIORITY           1         // Idle task is 0, so set to 1

class SerialBT;
extern SerialBT serialBT;

class SerialBT {

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

public:
  QueueHandle_t serialReceivedQueue;
  QueueHandle_t bluetoothReceivedQueue;

  TaskHandle_t sendToSerialTask;
  TaskHandle_t sendToBluetoothTask;

  // Attempt to use Xon/Xoff to manage Bluetooth connection data overruns
  static const byte btXon = 0x17;
  static const byte btXoff = 0x19;

  bool serialPaused;
  bool bluetoothPaused;

public:

  void startProxying() {

    serialReceivedQueue = xQueueCreate(SERIAL_QUEUE_SIZE, sizeof(uint8_t));
    bluetoothReceivedQueue = xQueueCreate(BLUETOOTH_QUEUE_SIZE, sizeof(uint8_t));

    // terminal.userOnReceive is called whenever data is received on the serial port. We want to send it over Bluetooth.
    terminal.userOnReceive =  [&](uint8_t c) {

      // NOTE: If the queue is not emptied fast enough, for now we will drop the characters.
      //       Nicer would be if we would pause the sender (serial device) until the queue is empty again.
      //       But that's for later.

      if (!serialBT.serialPaused && (uxQueueSpacesAvailable(serialBT.serialReceivedQueue) < SERIAL_QUEUE_LOW_WATER)) {
        // Low water mark reached, pause the sender.
        serialPort.flowControl(false);
        serialBT.serialPaused = true;
      }

      xQueueSend(serialReceivedQueue, &c, (TickType_t)10);
  };

    serialPaused = false;
    bluetoothPaused = false;

    xTaskCreate(sendBytesToSerialTask, "SND2SER", TASK_STACK_SIZE, this, TASK_PRIORITY, &sendToSerialTask);
    xTaskCreate(sendBytesToBluetoothTask, "SND2BT", TASK_STACK_SIZE, this, TASK_PRIORITY, &sendToBluetoothTask);
  }

  void stopProxying() {

      terminal.userOnReceive =  [&](uint8_t c) {};

      vTaskDelete(sendToSerialTask);
      vTaskDelete(sendToBluetoothTask);

      vQueueDelete(serialReceivedQueue);
      vQueueDelete(bluetoothReceivedQueue);

      serialReceivedQueue = NULL;
      bluetoothReceivedQueue = NULL;
  }

  static void onBTEventReceived(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {

    if (event == ESP_SPP_START_EVT) {
      Serial.printf("BT: SPP initialized event\n");
    }
    else if (event == ESP_SPP_SRV_OPEN_EVT ) {
      Serial.printf("BT: client connected event, status: %d\n", param->srv_open.status);
      serialBT.startProxying();
    }
    else if (event == ESP_SPP_CLOSE_EVT) {
      Serial.printf("BT: client disconnected event status: %d, port_status: %d, %s\n", param->close.status, param->close.port_status, param->close.async ? "async" : "sync");
      serialBT.stopProxying();
    }
    else if (event == ESP_SPP_DATA_IND_EVT) {
      for (int index = 0;index < param->data_ind.len; index++) {

        uint8_t c = param->data_ind.data[index];

        if (!serialBT.bluetoothPaused && (uxQueueSpacesAvailable(serialBT.bluetoothReceivedQueue) < BLUETOOTH_QUEUE_LOW_WATER)) {
          // Low water mark reached, pause the sender.
          bluetoothSerial.write(btXoff);
          serialBT.bluetoothPaused = true;
        }
        xQueueSend(serialBT.bluetoothReceivedQueue, &c, (TickType_t)10);
      }
    }
    else if (event == ESP_SPP_WRITE_EVT) {
      // Eat up write events.
    }
    else {
      // Serial.printf("BT: unhandled event: %d\n", event);
    }
  }

  static void sendBytesToSerialTask( void * pvParameters ) {

    for (;;) {
      char receivedByte;

      // Sleep indefinitely until we have received something from Bluetooth.
      // TODO: Is serialPort.send() thread safe?

      if (xQueueReceive(serialBT.bluetoothReceivedQueue, &receivedByte, portMAX_DELAY) == pdPASS) {

        if (serialBT.serialPaused && (uxQueueSpacesAvailable(serialBT.serialReceivedQueue) > SERIAL_QUEUE_HIGH_WATER)) {
          serialPort.flowControl(false);
          serialBT.serialPaused = false;
        }

        serialPort.send(receivedByte);
      }
    }
  }


  static void sendBytesToBluetoothTask( void * pvParameters ) {

    for (;;) {
      char receivedByte;
      TickType_t waitPeriod;

      // Sleep indefinitely until we have received something from Bluetooth.
      // TODO: Is serialPort.send() thread safe?

      if (xQueueReceive(serialBT.serialReceivedQueue, &receivedByte, portMAX_DELAY) == pdPASS) {

        if (serialBT.bluetoothPaused && (uxQueueSpacesAvailable(serialBT.bluetoothReceivedQueue) > BLUETOOTH_QUEUE_HIGH_WATER)) {
          // Low water mark reached, pause the sender.
          bluetoothSerial.write(btXon);
          serialBT.bluetoothPaused = false;
        }

        bluetoothSerial.write(receivedByte);

        // Check if we need to do a send delay.

        if (bluetoothPreferences.isSendDelayEnabled()) {
                    // If the byte is a carriage return, we have to wait the long wait, otherwise the short wait.
          if (receivedByte == 0x0D) {
            waitPeriod = pdMS_TO_TICKS(bluetoothPreferences.sendDelayForLine());
          }
          else {
            waitPeriod = pdMS_TO_TICKS(bluetoothPreferences.sendDelayForChar());
          }

          vTaskDelay(waitPeriod);
        }
      }
    }
  }

  static void onConfirmRequest(uint32_t num_val) {
    // Just confirm the request.
    bluetoothSerial.confirmReply(true);
  }

  static void onAuthComplete(bool success) {
  }
  void setup() {

    bluetoothSerial.onConfirmRequest(onConfirmRequest);
    bluetoothSerial.onAuthComplete(onAuthComplete);
    bluetoothSerial.enableSSP();
    bluetoothSerial.begin("nTerm2-S");
    bluetoothSerial.register_callback(onBTEventReceived);
  }
};

 SerialBT       serialBT;
