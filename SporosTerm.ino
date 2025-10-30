
#include "fabgl.h"
#include "TerminalEscapeCodeDefines.h"
#include "BluetoothSerial.h"

#define VERSION_NUMBER    0.2

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

  memoryReport("Initialization finished");

  if (terminalPreferences.currentHideSignonLogo) {
    enableTerminal();
  }
  else {
    disableTerminal();
    renderSignon();
    renderLogoAnimation();
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

  terminal.write(EC_CURPOS(28,18) EC_BLK "-= Press a key to start =-" EC_NOF);
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

void renderSignon() {
  terminal.write(EC_CURPOS(14,8) EC_DWI "SporosTerm v" EC_STR(VERSION_NUMBER) EC_NOF);
  terminal.write(EC_CURPOS(18,12) "Copyright " EC_COPR " 2025 Sporos Tech, Peter de Vroomen");
  terminal.write(EC_CURPOS(6,14) "Using the FabGL library, Copyright " EC_COPR " 2019-2022 Fabrizio Di Vittorio.");
  terminal.write(EC_CURPOS(17,15) "Many thanks to Fabrizio Di Vittorio and Just4Fun!");
}

void renderLogoAnimation() {
  terminal.write(EC_ALLOCSPRITES(1));
  terminal.write(EC_DEFSPRITECOL(0,64,20,"M",255,255,255,SPOROS_TECH_MONO_LOGO_DATA));

  int animXValues[124];
  for (double y = 0; y < 124; y++) {
    animXValues[123 - (int)y] = (int)(288 + sin(y / 9.0) * 20);
  }

  char scratch[32];
  for (int y = 0; y < 124; y++) {
    sprintf(scratch, "\e_GSPRITESET0;V;0;%d;%d$", animXValues[y], y);
    terminal.write(scratch);
    vTaskDelay(20);
  }
  terminal.write(EC_SETSPRITE(0,"V",0,288,124));  
}

static bool drawTerminalTest = false;

void loop() {
  vTaskDelete(NULL);
}
