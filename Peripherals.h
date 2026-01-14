#include "comdrivers/serialport.h"
#include "fabutils.h"
#include "canvas.h"
#include "displaycontroller.h"
#include "RealTimeClock.h"
#include "time.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "SerialBT.h"

#pragma once

#define RESET_PIN        39
#define RESET_PIN_ACTIVE  0   // 0 = reset when low, 1 = reset when high

// GPIO numbers for RTC
#define RTC_ONEWIRE                       14
#define RTC_INT                           36             

void memoryReport(const char *marker) {
  Serial.printf("[%s] Total free: %d, maximum allocatable: %d\n", marker, heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
}

struct Peripherals {

  static void handleStartupWithResetButtonPressed() {
    pinMode(RESET_PIN, INPUT);
    if (digitalRead(RESET_PIN) == RESET_PIN_ACTIVE) {
      serialPortPreferences.convertOrReset(true);
      terminalPreferences.convertOrReset(true);
      displayPreferences.convertOrReset(true);
      bluetoothPreferences.convertOrReset(true);
      dateTimePreferences.convertOrReset(true);

      // Spin until user releases the reset pin again.
      while(digitalRead(RESET_PIN) == RESET_PIN_ACTIVE) {
        vTaskDelay(250 / portTICK_PERIOD_MS);
      };

      // Display preferences were reset as well, so we must restart now.
      ESP.restart();
    }
  }

  static void initializePreferences() {
    relayManager.start();

    serialPortPreferences.start();
    terminalPreferences.start();
    displayPreferences.start();
    bluetoothPreferences.start();
    dateTimePreferences.start();
  }

  static void setupPS2Ports() {
    fabgl::Mouse::quickCheckHardware();
    ps2Controller.begin(PS2Preset::KeyboardPort0_MousePort1);
  }

  static void setupDisplayController() {

    memoryReport("Display setup starts");

    DisplayMode displayMode = displayPreferences.currentDisplayMode();

    if (displayMode.supportsBluetooth) {
      esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    }
    else {
      esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
    }
    
    Serial.printf("[Display] Setting resolution: (%d,%d), columns and rows: (%d,%d), colors: %d, statusbar: %s, bluetooth: %s, mode: %s\n", 
      displayMode.xRes,
      displayMode.yRes,
      displayMode.columns, 
      displayMode.rows, 
      displayMode.colors, 
      displayMode.enableStatusBar ? "YES" : "NO", 
      displayMode.supportsBluetooth ? "YES" : "NO", 
      displayMode.modeString);

    switch (displayMode.colors) {
      default:
      case 2:
        displayController = new fabgl::VGA2Controller;
        break;
      case 4:
        displayController = new fabgl::VGA4Controller;
        break;
      case 8:
        displayController = new fabgl::VGA8Controller;
        break;
      case 16:
        displayController = new fabgl::VGA16Controller;
        break;
    }

    displayController->begin();
    displayController->setResolution(displayMode.modeString, displayMode.xRes, displayMode.yRes, false);

    memoryReport("Display setup ends");
  }

  static void setupTerminal() {
    memoryReport("Terminal setup starts");

    DisplayMode displayMode = displayPreferences.currentDisplayMode();
    terminal.begin(displayController, displayMode.columns, displayMode.rows, ps2Controller.keyboard());

    terminal.setTerminalType(TermType::ANSI_VT);
    terminal.keyboard()->setLayout(&fabgl::USLayout);
    terminal.setBackgroundColor(Color::Black);
    terminal.setForegroundColor(Color::White);
    terminal.enableCursor(true);

    terminal.loadFont(displayMode.font);

    terminal.onVirtualKeyItem = [&](VirtualKeyItem * item) {
      if (item->down) {
        if (item->vk == VirtualKey::VK_F12) {
          if (settingsManager.isActive()) {
            settingsManager.finish();
          }
          else {
            settingsManager.show();
          }
          item->vk = VirtualKey::VK_NONE;
        } else if (item->vk == VirtualKey::VK_BREAK) {
          // BREAK (CTRL PAUSE) -> short break (TX low for 233 ms)
          // SHIFT BREAK (SHIFT CTRL PAUSE) -> long break (TX low for 3.5 s)
          serialPort.sendBreak(true);
          vTaskDelay((item->SHIFT ? 3500 : 233) / portTICK_PERIOD_MS);
          serialPort.sendBreak(false);
          item->vk = VirtualKey::VK_NONE;
        }
      }
    };

    terminalPreferences.apply();

    memoryReport("Terminal setup ends");
  }

  static void setupSerialPort() {
    serialPortPreferences.apply();
  }

  static void setupSerialPortTerminalConnector() {
    serialPortTerminalConnector.connect(&serialPort, &terminal);
  }

  static void setupBT() {
    memoryReport("Bluetooth setup starts");

    DisplayMode displayMode = displayPreferences.currentDisplayMode();

    // NOTE: Bluetooth support is connected with the display preferences. After changing display preference,
    //       the system is always rebooted. So we don't have to take care of tearing down Bluetooth, only
    //       setting up.
    if (displayMode.supportsBluetooth) {
      memoryReport("Before enabling Bluetooth");
      serialBT.setup();
      esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    }
    else {
      memoryReport("Before disabling Bluetooth");
      esp_bt_mem_release(ESP_BT_MODE_BTDM);
    }

    memoryReport("Bluetooth setup ends");
  }

  static void setupRealTimeClock() {
    realTimeClock.setup(RTC_ONEWIRE);
    realTimeClock.findDevice();
  }
};