
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

  // if (drawTerminalTest) {
  //   for (int x = 10; x <= terminalModes[terminalMode].columns; x += 10) {
  //     terminal->printf("         %1d", (x / 10) % 10);
  //   }
  //   terminal->printf("\r\n");
  //   for (int x = 1; x <= terminalModes[terminalMode].columns; x++) {
  //     terminal->printf("%1d", x % 10);
  //   }
  //   terminal->printf("\r\n");

  //   for (int y = 3; y < terminalModes[terminalMode].rows; y++) {
  //     terminal->printf("%2d        ", y);  
  //     if (y < 11) {
  //       for (int x = 0; x < 32; x++) {
  //         char ch = (y - 3) * 32 + x;
  //         terminal->print(printableChar(ch));
  //       }
  //     }

  //     terminal->printf("\r\n");
  //   }

  //   terminal->printf("%2d Normal\x1b[1mBold\x1b[0mNormal\x1b[7mInverse", terminalModes[terminalMode].rows); 

  //   drawTerminalTest = false;
  // }

  vTaskDelete(NULL);

  // delay(20);
}
