#include "BluetoothSerialPatched.h"
#include "esp_spp_api.h"
#include "esp_bt.h"

#pragma once

// NOTE: BluetoothSerial also manages queues, and use send/recieve 32/512 sizes. Maybe we can make our queues
//       a lot smaller. They are only necessary because we need to cross thread boundaries.


#define SEND_TO_SERIAL_QUEUE_SIZE             1024
#define SEND_TO_BLUETOOTH_QUEUE_SIZE          1024

#define TASK_STACK_SIZE         4096          // At 1024 I saw frequent crashes. At 2048 I saw occasional crashes. They went away once I made it 4096. But to be honest, I don't know why it needs to be this big.
#define TASK_PRIORITY           5             // 5 is the same as keyboard and mouse scanning priority.

class SerialBT;
extern SerialBT serialBT;

class SerialBT {

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

public:
  TimerHandle_t sendTimer;

  QueueHandle_t sendToSerialQueueHandle;
  QueueHandle_t sendToBluetoothQueueHandle;

  TaskHandle_t sendToSerialTaskHandle;
  TaskHandle_t sendToBluetoothTaskHandle;

public:

  void startProxying() {

    sendToSerialQueueHandle = xQueueCreate(SEND_TO_SERIAL_QUEUE_SIZE, sizeof(uint8_t));
    sendToBluetoothQueueHandle = xQueueCreate(SEND_TO_BLUETOOTH_QUEUE_SIZE, sizeof(uint8_t));

    // terminal.userOnReceive is called whenever data is received on the serial port. We want to send it to Bluetooth.
    terminal.userOnReceive =  [&](uint8_t c) {
      // Serial.printf(">BQ %02X\n", c);
      xQueueSend(sendToBluetoothQueueHandle, &c, (TickType_t)10);
    };

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

  static void onBTDataReceived(const uint8_t *buffer, size_t size) {
    Serial.printf("onBTDataReceived %d bytes\n", size);
    for (int count = 0; count < size; count++ ) {
      uint8_t receivedByte = buffer[count];
      Serial.printf(">Q %c\n", receivedByte > 31 ? receivedByte : '.');
      xQueueSend(serialBT.sendToSerialQueueHandle, &receivedByte, (TickType_t)10);
    }
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
      
      // Do nothing. We consume the data in the onData() callback.

      // NOTE: If we don't use the onData() callback, the data that we receive here will have been put in the receiver
      //       queue of BluetoothSerial. And we need to somehow consume that data, or the queue will never empty. So
      //       even if we use the data here, we will still need to call 'read' repeatedly to empty the queue.
      //       As we already manage queues ourself, it's better to use the onData callback. As then the data will never
      //       be queued in BluetoothSerial.
    }
    else if (event == ESP_SPP_WRITE_EVT) {
      // Eat up write events, not doing anything with them.
    }
    else {
      // Serial.printf("BT: unhandled event: %d\n", event);
    }
  }

  static void sendBytesToSerialTask( void * pvParameters ) {
    char receivedByte;

    // Serial.printf("sendBytesToSerialTask, start listening to sendToSerialQueueHandle\n");

    for (;;) {
      if (xQueueReceive(serialBT.sendToSerialQueueHandle, &receivedByte, portMAX_DELAY) == pdPASS) {
        Serial.printf(">S %c\n", receivedByte > 31 ? receivedByte : '.');

        serialPort.send(receivedByte);

        if (bluetoothPreferences.isSendDelayEnabled()) {
          TickType_t delay = bluetoothPreferences.sendDelayForChar() / portTICK_PERIOD_MS;
          if (receivedByte == 0x0A) {     // LF
            // Long delay
            delay = bluetoothPreferences.sendDelayForLine() / portTICK_PERIOD_MS;
          }
          vTaskDelay(delay);
        }
      }
    }
  }

  static void sendBytesToBluetoothTask( void * pvParameters ) {
    char receivedByte;

    // Serial.printf("sendBytesToBluetoothTask, start listening to sendToBluetoothQueueHandle\n");

    for (;;) {
      if (xQueueReceive(serialBT.sendToBluetoothQueueHandle, &receivedByte, portMAX_DELAY) == pdPASS) {
        // Serial.printf(">BT %02X\n", receivedByte);
        bluetoothSerial.write(receivedByte);
      }
    }
  }

  // We're just taking over the sent pin code and replying confirmation. To be honest, I think this is simply
  // the same as legacy authentication. The reason we're doing it this way is that on some system I get a popup
  // asking to verify the pin code anyway, so we might as well make it "official" and be sure that we're doing
  // everything right.
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
    bluetoothSerial.onData(onBTDataReceived);
  }
};

 SerialBT       serialBT;
