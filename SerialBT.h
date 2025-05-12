#include "BluetoothSerial.h"
#include "esp_spp_api.h"
#include "esp_bt.h"

#pragma once


struct SerialBT {

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

  static void onBTEventReceived(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    if (event == ESP_SPP_START_EVT) {
      Serial.println("BT: SPP initialized event");
    }
    else if (event == ESP_SPP_SRV_OPEN_EVT ) {
      Serial.println("BT: client connected event");
    }
    else if (event == ESP_SPP_CLOSE_EVT) {
      Serial.println("BT: client disconnected event");
    }
    else if (event == ESP_SPP_DATA_IND_EVT) {
      for (int index = 0;index < param->data_ind.len; index++) {
        serialPort.send(param->data_ind.data[index]);
        if (bluetoothPreferences.isSendDelayEnabled()) {
          if (param->data_ind.data[index] == 0x13) {
            delay(bluetoothPreferences.sendDelayForLine());
          }
          else {
            delay(bluetoothPreferences.sendDelayForChar());
          }
        }
      }
    }
    else {
      Serial.printf("BT: unhandled event: %0X\n", event);
    }
  }
  static void setup() {

    bluetoothSerial.begin("nTerm2-S");
    
    bluetoothSerial.register_callback(SerialBT::onBTEventReceived);

    terminal.userOnReceive =  [&](uint8_t c) {
      Serial.printf("BT: to btSerial: %02X\n", c);
      bluetoothSerial.write(c);
    };
  }
};