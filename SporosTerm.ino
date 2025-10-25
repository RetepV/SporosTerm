
#include "fabgl.h"
#include "BluetoothSerial.h"

fabgl::PS2Controller                ps2Controller;
fabgl::BaseDisplayController        *displayController;
fabgl::Terminal                     terminal;
fabgl::TerminalController           TerminalController(&terminal);
fabgl::SerialPort                   serialPort;
fabgl::SerialPortTerminalConnector  serialPortTerminalConnector;

BluetoothSerial                     bluetoothSerial;

#include "SerialPortPreferences.h"
#include "TerminalPreferences.h"
#include "DisplayPreferences.h"
#include "BluetoothPreferences.h"

#include "SettingsManager.h"
SettingsManager settingsManager;

#include "StatusBar.h"

#include "Peripherals.h"

void setup() {
  Serial.begin(115200);

  memoryReport("Initialization starts");

  disableCore0WDT();
  delay(100); // experienced crashes without this delay!
  disableCore1WDT();

  Peripherals::setupRealTimeClock();

  Peripherals::initializePreferences();

  Peripherals::setupPS2Ports();
  Peripherals::setupDisplayController();
  Peripherals::setupTerminal();
  Peripherals::setupSerialPortTerminalConnector();
  Peripherals::setupBT();
  Peripherals::setupStatusBar();
  Peripherals::setupSerialPort();

  terminal.write(EC_CON EC_CLS EC_CHM EC_NOF);

  memoryReport("Initialization finished");
}

static bool drawTerminalTest = false;

void loop() {
  vTaskDelete(NULL);
}
