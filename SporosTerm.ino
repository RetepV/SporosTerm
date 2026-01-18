
#include "fabgl.h"
#include "GlobalDefines.h"
#include "BluetoothSerialPatched.h"
#include "FS.h"
#include <LittleFS.h>

#define VERSION_NUMBER    0.5

// Some forward declarations to satisfy dependencies.
static void switchToLocalMode();
static void switchToNormalMode();
static void cursorDidChange(int cursorType);

fabgl::PS2Controller                ps2Controller;
fabgl::BaseDisplayController        *displayController;
fabgl::Terminal                     terminal;
fabgl::TerminalController           TerminalController(&terminal);
fabgl::SerialPort                   serialPort;
fabgl::SerialPortTerminalConnector  serialPortTerminalConnector;

BluetoothSerialPatched              bluetoothSerial;

bool                                prevCursorEnabled = false;

#include "GlyphsBufferSaver.h"

#include "SerialPortPreferences.h"
#include "TerminalPreferences.h"
#include "DisplayPreferences.h"
#include "BluetoothPreferences.h"

#include "SettingsManager.h"
SettingsManager settingsManager;

#include "Peripherals.h"
#include "StatusBar.h"

#include "SignonMessage.h"

// These are here because of messed-up dependencies between classes. Arduino suffers a lot from 'dependency hell'.

static void setupStatusBar() {
  DisplayMode displayMode = displayPreferences.currentDisplayMode();

  if (displayMode.enableStatusBar) {
    statusBar.start(displayMode.rows);
  }
}

static void switchToLocalMode() {

  terminal.saveCursorState();

  prevCursorEnabled = terminal.getCursorEnabled();

  serialPortTerminalConnector.disableSerialPortRX(true);
  terminal.enableLocalMode(true);

  GlyphsBufferSaver::saveGlyphsBuffer();

  terminal.clear(true);
  terminal.enableCursor(false);
}

static void switchToNormalMode() {
  terminal.write(EC_SETSPRITE(0,"H",0,0,0));
  terminal.write(EC_ALLOCSPRITES(0));

  terminal.clear(true);

  GlyphsBufferSaver::restoreGlyphsBuffer();

  terminal.restoreCursorState();
  terminal.enableCursor(prevCursorEnabled);

  terminal.onLocalModeVirtualKeyItem = [&](VirtualKeyItem *item) {
  };
  terminal.onLocalModeReceive = [&](uint8_t value) {
  };

  terminal.enableLocalMode(false);
  serialPortTerminalConnector.disableSerialPortRX(false);
}

static void cursorDidChange(int cursorType) {
  prevCursorEnabled = cursorType != 0;
}

void setup() {
  Serial.begin(115200);

  memoryReport("Initialization starts");

  disableCore0WDT();
  delay(100); // experienced crashes without this delay!
  disableCore1WDT();

  Peripherals::setupRealTimeClock();
  Peripherals::initializeLittleFS();

  Peripherals::initializePreferences();

  Peripherals::handleStartupWithResetButtonPressed();   // If reset button was pressed, we won't go beyond here but restart.
  
  Peripherals::setupPS2Ports();
  Peripherals::setupDisplayController();
  Peripherals::setupTerminal();
  Peripherals::setupSerialPortTerminalConnector();
  
  switchToLocalMode();
  
  Peripherals::setupBT();
  Peripherals::setupSerialPort();

  setupStatusBar();

  memoryReport("Initialization finished");

  if (terminalPreferences.currentHideSignonLogo) {
    switchToNormalMode();
  }
  else {
    SignonMessage::renderSignon();
    enableTerminalOnKeypress();
  }
}

void enableTerminalOnKeypress() {

  terminal.onLocalModeReceive = [&](uint8_t value) {
  };

  terminal.onLocalModeVirtualKeyItem = [&](VirtualKeyItem *item) {
    switchToNormalMode();
  };

  SignonMessage::renderPressKey();
}

void loop() {
  vTaskDelete(NULL);
}
