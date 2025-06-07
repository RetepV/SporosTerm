#include "BluetoothSerial.h"
#include "esp_spp_api.h"
#include "esp_bt.h"
#include "RingBuffer.h"

#pragma once

class SerialBT;
extern SerialBT serialBT;

class SerialBT {

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

public:
  TimerHandle_t sendTimer;

  RingBuffer btRingBuffer = RingBuffer(1024, 512, 768);

  static const byte btXon = 0x17;
  static const byte btXoff = 0x19;

public:

  static void startProxying() {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;

      serialBT.sendTimer = xTimerCreate("serialBT", pdMS_TO_TICKS(100), pdFALSE, (void*)0, trySendByteToSerial);

      xTimerStartFromISR(serialBT.sendTimer, &xHigherPriorityTaskWoken);

      terminal.userOnReceive =  [&](uint8_t c) {
        bluetoothSerial.write(c);
      };
  }

  static void stopProxying() {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;

      xTimerStopFromISR(serialBT.sendTimer, &xHigherPriorityTaskWoken);
      xTimerDelete(serialBT.sendTimer, 0);
      serialBT.sendTimer = NULL;

      terminal.userOnReceive =  [&](uint8_t c) {};
  }

  static void onBTEventReceived(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    if (event == ESP_SPP_START_EVT) {
      Serial.printf("BT: SPP initialized event\n");
    }
    else if (event == ESP_SPP_SRV_OPEN_EVT ) {
      Serial.printf("BT: client connected event, status: %d\n", param->srv_open.status);
      startProxying();
    }
    else if (event == ESP_SPP_CLOSE_EVT) {
      Serial.printf("BT: client disconnected event status: %d, port_status: %d, %s\n", param->close.status, param->close.port_status, param->close.async ? "async" : "sync");
      stopProxying();
    }
    else if (event == ESP_SPP_DATA_IND_EVT) {

      for (int index = 0;index < param->data_ind.len; index++) {
        serialBT.btRingBuffer.put(param->data_ind.data[index], []() {
          // Ask BT sender to wait.
          bluetoothSerial.write(btXoff);
        });
      }
    }
    else if (event == ESP_SPP_WRITE_EVT) {
      // Eat up write events.
    }
    else {
      // Serial.printf("BT: unhandled event: %d\n", event);
    }
  }

  static void trySendByteToSerial(xTimerHandle pxTimer) {

    TickType_t newPeriod;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (serialBT.btRingBuffer.dataSize() == 0) {
      // When idling, try 10 times per second.
      newPeriod = pdMS_TO_TICKS(100);
    }
    else {
      byte byteToSend;

      if (!bluetoothPreferences.isSendDelayEnabled()) {
        // Consume whole buffer in one go.
        bool haveByte = serialBT.btRingBuffer.get(&byteToSend, []() {
          bluetoothSerial.write(btXon);
        });
        while (haveByte) {
          serialPort.send(byteToSend);
          haveByte = serialBT.btRingBuffer.get(&byteToSend, []() {
            bluetoothSerial.write(btXon);
          });
        }
        // Do next check as fast as possible, but give some breathing time? 
        newPeriod = pdMS_TO_TICKS(10);
      }
      else {
        // Consume buffer as slowly as told to by the user.
        bool haveByte = serialBT.btRingBuffer.get(&byteToSend, []() {
          bluetoothSerial.write(btXon);
        });
        if (haveByte) {
          serialPort.send(byteToSend);
          // If the byte is a carriage return, we have to wait the long wait, otherwise the short wait.
          if (byteToSend == 0x0D) {
            newPeriod = pdMS_TO_TICKS(bluetoothPreferences.sendDelayForLine());
          }
          else {
            newPeriod = pdMS_TO_TICKS(bluetoothPreferences.sendDelayForChar());
          }
          if (newPeriod < pdMS_TO_TICKS(10)) {
            newPeriod = pdMS_TO_TICKS(10);
          }
        }
        else {
          newPeriod = pdMS_TO_TICKS(100);
        }
      }
    }
    xTimerChangePeriodFromISR(pxTimer, newPeriod, &xHigherPriorityTaskWoken);
    xTimerStartFromISR(pxTimer, &xHigherPriorityTaskWoken);
  }

  void setup() {
    
    bluetoothSerial.begin("nTerm2-S");
    bluetoothSerial.register_callback(SerialBT::onBTEventReceived);
  }
};

 SerialBT       serialBT;
