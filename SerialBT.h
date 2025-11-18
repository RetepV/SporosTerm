#include "BluetoothSerialPatched.h"
#include "esp_spp_api.h"
#include "esp_bt.h"

#pragma once

// NOTE: BluetoothSerial also manages queues, and use send/recieve 32/512 sizes. Maybe we can make our queues
//       a lot smaller. They are only necessary because we need to cross thread boundaries.


#define SEND_TO_SERIAL_QUEUE_SIZE             1024
#define SEND_TO_SERIAL_QUEUE_LOW_WATER        (int)((SEND_TO_SERIAL_QUEUE_SIZE / 100) * 20)       // 20% low water mark
#define SEND_TO_SERIAL_QUEUE_HIGH_WATER       (int)((SEND_TO_SERIAL_QUEUE_SIZE / 100) * 80)       // 80% high water mark

#define SEND_TO_BLUETOOTH_QUEUE_SIZE          1024
#define SEND_TO_BLUETOOTH_QUEUE_LOW_WATER     (int)((SEND_TO_BLUETOOTH_QUEUE_SIZE / 100) * 10)    // 20% low water mark
#define SEND_TO_BLUETOOTH_QUEUE_HIGH_WATER    (int)((SEND_TO_BLUETOOTH_QUEUE_SIZE / 100) * 90)    // 80% low water mark

#define TASK_STACK_SIZE         4096          // At 1024 I saw frequent crashes. At 2048 I saw occasional crashes. They went away once I made it 4096. But to be honest, I don't know why it needs to be this big.
#define TASK_PRIORITY           5             // 5 is the same as keyboard and mouse scanning priority.

class SerialBT;
extern SerialBT serialBT;

class SerialBT {

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

public:
  QueueHandle_t sendToSerialQueueHandle;
  QueueHandle_t sendToBluetoothQueueHandle;

  TaskHandle_t sendToSerialTaskHandle;
  TaskHandle_t sendToBluetoothTaskHandle;

  bool serialHostPaused;
  bool bluetoothHostPaused;

  // Attempt to use Xon/Xoff to manage Bluetooth connection data overruns
  static const byte btXon = 0x17;
  static const byte btXoff = 0x19;

public:

  void startProxying() {

    sendToSerialQueueHandle = xQueueCreate(SEND_TO_SERIAL_QUEUE_SIZE, sizeof(uint8_t));
    sendToBluetoothQueueHandle = xQueueCreate(SEND_TO_BLUETOOTH_QUEUE_SIZE, sizeof(uint8_t));

    // terminal.userOnReceive is called whenever data is received on the serial port. We want to send it to Bluetooth.
    terminal.userOnReceive =  [&](uint8_t c) {

      if (!serialBT.serialHostPaused && (uxQueueSpacesAvailable(serialBT.sendToBluetoothQueueHandle) < SEND_TO_BLUETOOTH_QUEUE_LOW_WATER)) {
        // Serial.printf(">BQ: LW %d<%d pause SER rcv\n", uxQueueSpacesAvailable(serialBT.sendToBluetoothQueueHandle), SEND_TO_BLUETOOTH_QUEUE_LOW_WATER);
        // Low water mark is reached on the sendToBluetoothQueue. Pause the incoming data (serial port) until the sendToBluetoothQueue becomes available again.
        serialPort.flowControl(false);
        serialBT.serialHostPaused = true;
      }

      // Serial.printf(">BQ %02X\n", c);
      xQueueSend(sendToBluetoothQueueHandle, &c, (TickType_t)10);
    };

    serialHostPaused = false;
    bluetoothHostPaused = false;

    xTaskCreate(sendBytesToSerialTask, "SND2SER", TASK_STACK_SIZE, this, TASK_PRIORITY, &sendToSerialTaskHandle);
    xTaskCreate(sendBytesToBluetoothTask, "SND2BT", TASK_STACK_SIZE, this, TASK_PRIORITY, &sendToBluetoothTaskHandle);
  }

  void stopProxying() {

      terminal.userOnReceive =  [&](uint8_t c) {};

      vTaskDelete(sendToSerialTaskHandle);
      vQueueDelete(sendToSerialQueueHandle);
      sendToSerialQueueHandle = NULL;

      vTaskDelete(sendToBluetoothTaskHandle);
      vQueueDelete(sendToBluetoothQueueHandle);
      sendToBluetoothQueueHandle = NULL;
  }

  static void onBTEventReceived(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {

    if (event == ESP_SPP_START_EVT) {
      Serial.printf("BT event: SPP initialized\n");
    }
    else if (event == ESP_SPP_SRV_OPEN_EVT ) {
      Serial.printf("BT event: client connected, status: %d\n", param->srv_open.status);
      serialBT.startProxying();
    }
    else if (event == ESP_SPP_CLOSE_EVT) {
      Serial.printf("BT event: client disconnected status: %d, port_status: %d, %s\n", param->close.status, param->close.port_status, param->close.async ? "async" : "sync");
      serialBT.stopProxying();
    }
    else if (event == ESP_SPP_DATA_IND_EVT) {

      // Received characters from Bluetooth, put them in the sendToSerialQueue.

      for (int index = 0;index < param->data_ind.len; index++) {

        uint8_t c = param->data_ind.data[index];

        // If the sendToSerialQueue is close to full, pause the Bluetooth.
        if (!serialBT.bluetoothHostPaused && (uxQueueSpacesAvailable(serialBT.sendToSerialQueueHandle) < SEND_TO_SERIAL_QUEUE_LOW_WATER)) {
          // Serial.printf(">SQ: LW %d<%d pause BT rcv\n", uxQueueSpacesAvailable(serialBT.sendToSerialQueueHandle), SEND_TO_SERIAL_QUEUE_LOW_WATER);
          // Low water mark is reached on the sendToSerialQueue. Pause the incoming data (bluetooth) until the sendToSerialQueue becomes available again.
          bluetoothSerial.write(btXoff);
          serialBT.bluetoothHostPaused = true;
        }

        // Serial.printf(">SQ %02X\n", c);
        xQueueSend(serialBT.sendToSerialQueueHandle, &c, (TickType_t)10);
      }
    }
    else if (event == ESP_SPP_WRITE_EVT) {
      // Eat up write events, not doing anything with them.
    }
    else {
      // Serial.printf("BT: unhandled event: %d\n", event);
    }
  }

  // Read events from the sendToSerialQueueHandle, send data to serial port.
  static void sendBytesToSerialTask( void * pvParameters ) {
    char receivedByte;
    TickType_t waitPeriod;

    // Serial.printf("sendBytesToSerialTask, start listening to sendToSerialQueueHandle\n");

    for (;;) {

      // Sleep indefinitely until we have received something from Bluetooth.
      // TODO: Is serialPort.send() thread safe?

      if (xQueueReceive(serialBT.sendToSerialQueueHandle, &receivedByte, portMAX_DELAY) == pdPASS) {

        if (serialBT.bluetoothHostPaused && (uxQueueSpacesAvailable(serialBT.sendToSerialQueueHandle) > SEND_TO_SERIAL_QUEUE_HIGH_WATER)) {
          // Serial.printf(">SP: HW %d>%d resume BT rcv\n", uxQueueSpacesAvailable(serialBT.sendToSerialQueueHandle), SEND_TO_SERIAL_QUEUE_HIGH_WATER);
          // High water mark is reached after we had reached the low water mark earlier, so unpause Bluetooth.
          bluetoothSerial.write(btXon);
          serialBT.bluetoothHostPaused = false;
        }

        // Serial.printf(">SP %02X\n", receivedByte);
        serialPort.send(receivedByte);

        // After we sent the byte, we might want to do a send delay to give really old systems without
        // flow control some time to process.
        // There is a long wait and a short wait.
        // The long wait is executed after we sent a CR. The idea here is that after a CR the host system
        // probably wants to do some processing and will need time for that.
        // The short wait is executed after any other letter. The idea here is that the system will not have
        // to take so much time to process what a user would type.

        if (bluetoothPreferences.isSendDelayEnabled()) {
          // If the byte is a carriage return, we wait the long wait, otherwise the short wait.
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

  static void sendBytesToBluetoothTask( void * pvParameters ) {
    char receivedByte;

    // Serial.printf("sendBytesToBluetoothTask, start listening to sendToBluetoothQueueHandle\n");

    for (;;) {
      if (xQueueReceive(serialBT.sendToBluetoothQueueHandle, &receivedByte, portMAX_DELAY) == pdPASS) {

        // If sendToBluetoothPaused, it means that the serial incoming data is paused because previously our sendToBluetoothQueue was almost full.
        if (serialBT.serialHostPaused && (uxQueueSpacesAvailable(serialBT.sendToBluetoothQueueHandle) > SEND_TO_BLUETOOTH_QUEUE_HIGH_WATER)) {
          // Serial.printf(">BT: HW %d>%d resume SER rcv\n", uxQueueSpacesAvailable(serialBT.sendToBluetoothQueueHandle), SEND_TO_BLUETOOTH_QUEUE_HIGH_WATER);
          // High water mark is reached after we had reached the low water mark earlier, so unpause serial.
          serialPort.flowControl(true);
          serialBT.serialHostPaused = false;
        }

        // Serial.printf(">BT %02X\n", receivedByte);
        bluetoothSerial.write(receivedByte);
      }
    }
  }

  // We're just taking over the sent pin code and replying confirmation. To be honest, I think this is simply
  // the same a legacy authentication.
  static void onConfirmRequest(uint32_t num_val) {
    char pinCode[17];
    sprintf(pinCode, "%lu", num_val);
    //Serial.printf("onConfirmRequest(%d) '%s' (%d)\n", num_val, pinCode, strlen(pinCode));
    bluetoothSerial.setPin(pinCode, strlen(pinCode));
    bluetoothSerial.confirmReply(true);
  }

  static void onKeyRequest(void) {
    //Serial.printf("onKeyRequest\n");
    bluetoothSerial.respondPasskey(1234);
  }

  void setup() {

    // bluetoothSerial.disableSSP();
    bluetoothSerial.onConfirmRequest(onConfirmRequest);
    bluetoothSerial.onKeyRequest(onKeyRequest);
    bluetoothSerial.enableSSP(true, true);
    bluetoothSerial.begin("nTerm2-S");
    bluetoothSerial.register_callback(onBTEventReceived);
  }
};

 SerialBT       serialBT;
