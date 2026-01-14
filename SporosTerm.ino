
#include "fabgl.h"
#include "GlobalDefines.h"
#include "BluetoothSerialPatched.h"

#define VERSION_NUMBER    0.4

fabgl::PS2Controller                ps2Controller;
fabgl::BaseDisplayController        *displayController;
fabgl::Terminal                     terminal;
fabgl::TerminalController           TerminalController(&terminal);
fabgl::SerialPort                   serialPort;
fabgl::SerialPortTerminalConnector  serialPortTerminalConnector;

BluetoothSerialPatched              bluetoothSerial;

#include "SerialPortPreferences.h"
#include "TerminalPreferences.h"
#include "DisplayPreferences.h"
#include "BluetoothPreferences.h"

#include "SettingsManager.h"
SettingsManager settingsManager;

#include "Peripherals.h"
#include "StatusBar.h"

#include "SignonMessage.h"

// Here because of messed-up dependencies between StatusBar and Peripherals.
static void setupStatusBar() {
  DisplayMode displayMode = displayPreferences.currentDisplayMode();

  if (displayMode.enableStatusBar) {
    statusBar.start(displayMode.rows);
  }
}

void setup() {
  Serial.begin(115200);

  memoryReport("Initialization starts");

  disableCore0WDT();
  delay(100); // experienced crashes without this delay!
  disableCore1WDT();

  Peripherals::setupRealTimeClock();

  Peripherals::initializePreferences();

  Peripherals::handleStartupWithResetButtonPressed();   // If reset button was pressed, we won't go beyond here but restart.
  
  Peripherals::setupPS2Ports();
  Peripherals::setupDisplayController();
  Peripherals::setupTerminal();
  Peripherals::setupSerialPortTerminalConnector();
  disableTerminal();
  Peripherals::setupBT();
  Peripherals::setupSerialPort();

  setupStatusBar();

  memoryReport("Initialization finished");

  if (terminalPreferences.currentHideSignonLogo) {
    enableTerminal();
  }
  else {
    SignonMessage::renderSignon();
    enableTerminalOnKeypress();
  }
}

void disableTerminal() {
  serialPortTerminalConnector.disableSerialPortRX(true);
  terminal.enableLocalMode(true);
  
  terminal.clear(true);
  terminal.enableCursor(false);
}

void enableTerminalOnKeypress() {

  terminal.onLocalModeReceive = [&](uint8_t value) {
  };

  terminal.onLocalModeVirtualKeyItem = [&](VirtualKeyItem *item) {
    enableTerminal();
  };

  SignonMessage::renderPressKey();
}

void enableTerminal() {
  terminal.write(EC_SETSPRITE(0,"H",0,0,0));
  terminal.write(EC_ALLOCSPRITES(0));
  
  terminal.clear(true);
  terminal.enableCursor(true);

  terminal.enableLocalMode(false);
  
  terminal.onLocalModeVirtualKeyItem = [&](VirtualKeyItem *item) {
  };
  terminal.onLocalModeReceive = [&](uint8_t value) {
  };
  
  serialPortTerminalConnector.disableSerialPortRX(false);
}

void loop() {
  vTaskDelete(NULL);
}
