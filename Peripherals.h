#include "comdrivers/serialport.h"
#include "fabutils.h"
#include "canvas.h"
#include "displaycontroller.h"
#include "time.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "SerialBT.h"

#pragma once

// Ports for relay switches
#define RELAY_REVERSE_RX_TX               12
#define RELAY_USE_RTS_CTS_OR_DTR_DSR      16
#define RELAY_REVERSE_RTS_CTS_OR_DTR_DSR  17

#define AMBER_COLOR RGB888(255,192,0)

void memoryReport(const char *marker) {
  Serial.printf("[%s] Total free: %d, maximum allocatable: %d\n", marker, heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
}

struct Peripherals {

  static void restorePreferences() {
    memoryReport("restorePreferences");

    serialPortPreferences.start();
    terminalPreferences.start();
    displayPreferences.start();
    bluetoothPreferences.start();
  }

  static void setupPS2Ports() {
    memoryReport("setupPS2Ports");

    fabgl::Mouse::quickCheckHardware();

    ps2Controller.begin(PS2Preset::KeyboardPort0_MousePort1);

    if (ps2Controller.keyboard()->isKeyboardAvailable()) {
      Serial.printf("Found keyboard\n");
    }
    if (ps2Controller.mouse()->isMouseAvailable()) {
      Serial.printf("Found mouse\n");
    }

    Serial.printf("ps2Controller set up\n");
  }

  static void setupDisplayController() {
    memoryReport("setupDisplayController");

    DisplayMode displayMode = displayPreferences.currentDisplayMode();

    if (displayMode.supportsBluetooth) {
      esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    }
    else {
      esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
    }

    memoryReport("setupDisplayController after release BT memory");

    Serial.printf("Display mode colors: %d\n", displayMode.colors);
    Serial.printf("Display mode modestring: %s\n", displayMode.modeString);
    
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

    Serial.printf("Screen size   : %d x %d\n", displayController->getScreenWidth(), displayController->getScreenHeight());
    Serial.printf("Viewport size : %d x %d\n", displayController->getViewPortWidth(), displayController->getViewPortHeight());

    Serial.printf("displayController set up\n");
  }

  static void setupTerminal() {
    memoryReport("setupTerminal");

    DisplayMode displayMode = displayPreferences.currentDisplayMode();

    Serial.printf("Display mode cols x rows): %dx%d\n", displayMode.columns, displayMode.rows);

    terminal.begin(displayController, displayMode.columns, displayMode.rows, ps2Controller.keyboard());

    terminal.setTerminalType(TermType::ANSI_VT);
    terminal.keyboard()->setLayout(&fabgl::USLayout);
    terminal.setBackgroundColor(Color::Black);
    terminal.setForegroundColor(Color::Yellow);
    terminal.enableCursor(true);

    Serial.printf("Display mode font: %dx%d\n", displayMode.font->width, displayMode.font->height);

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
        }
      }
    };

    terminalPreferences.apply();

    Serial.printf("terminal set up\n");
  }

  static void setupSerialPort() {
    memoryReport("setupSerialPort");

    serialPortPreferences.apply();

    Serial.printf("serialPort set up\n");
  }

  static void setupSerialPortTerminalConnector() {
    memoryReport("setupSerialPortTerminalConnector");

    serialPortTerminalConnector.connect(&serialPort, &terminal);

    Serial.printf("serialPortTerminalConnector set up\n");
  }
    
  static void setupBT() {

    memoryReport("SerialBT");

    DisplayMode displayMode = displayPreferences.currentDisplayMode();

    if (displayMode.supportsBluetooth) {
      SerialBT::setup();
      esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    }
    else {
      esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
    }

    Serial.printf("SerialBT set up\n");
  }

  static void setupStatusBar() {
    memoryReport("Status Bar");

    DisplayMode displayMode = displayPreferences.currentDisplayMode();

    Serial.printf("Display enable status bar: %d\n", displayMode.enableStatusBar);
    if (displayMode.enableStatusBar) {
      statusBar.start(displayMode.rows);
    }

    Serial.printf("statusBar set up\n");
  }

  static void setupRelays(int nullModem, int dtrdsr) {
    memoryReport("setupRelays");

    pinMode(RELAY_REVERSE_RX_TX, OUTPUT);
    pinMode(RELAY_USE_RTS_CTS_OR_DTR_DSR, OUTPUT);
    pinMode(RELAY_REVERSE_RTS_CTS_OR_DTR_DSR, OUTPUT);
  
    // Reset all
  
    digitalWrite(RELAY_REVERSE_RX_TX, HIGH);
    digitalWrite(RELAY_USE_RTS_CTS_OR_DTR_DSR, HIGH);
    digitalWrite(RELAY_REVERSE_RTS_CTS_OR_DTR_DSR, HIGH);
    
    vTaskDelay(150 / portTICK_PERIOD_MS);
    
    digitalWrite(RELAY_REVERSE_RX_TX, LOW);
    digitalWrite(RELAY_USE_RTS_CTS_OR_DTR_DSR, LOW);
    digitalWrite(RELAY_REVERSE_RTS_CTS_OR_DTR_DSR, LOW);
    
    vTaskDelay(150 / portTICK_PERIOD_MS);

    // Program state from settings
  
    if (nullModem == 1) {
      digitalWrite(RELAY_REVERSE_RX_TX, HIGH);
      digitalWrite(RELAY_REVERSE_RTS_CTS_OR_DTR_DSR, HIGH);
    }
  
    if (dtrdsr == 1) {
      digitalWrite(RELAY_USE_RTS_CTS_OR_DTR_DSR, HIGH);
    }

    Serial.printf("relays set up\n");
  }

  // static void setupWiFi() {
  //   memoryReport("WiFi begin");
  //   WiFi.mode(WIFI_STA);
  //   WiFi.begin("Ziggo9293728", "8t7hyhuvQkkt");
  //   while ( WiFi.status() != WL_CONNECTED) {
  //     delay(1000);
  //   }
  // }
};