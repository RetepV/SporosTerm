#include "fabutils.h"
#include "SettingsManagerPage.h"
#include "TerminalEscapeCodeDefines.h"

#pragma once

class SettingsManager;

class SettingsMainPage: public SettingsManagerPage {

public:
  SettingsManager *settingsManager;

  void show() override {

    terminal.write(EC_STX);

    terminal.write(EC_COF EC_CLS EC_CHM EC_DWI EC_ULN "SETTINGS - MAIN MENU" EC_NOF EC_CRLF EC_CRLF);

    terminal.write(EC_BLD "P" EC_NOF ". serial " EC_BLD "P" EC_NOF "ort" EC_CRLF);
    terminal.write(EC_BLD "T" EC_NOF ". " EC_BLD "T" EC_NOF "erminal" EC_CRLF);
    terminal.write(EC_BLD "D" EC_NOF ". " EC_BLD "D" EC_NOF "isplay" EC_CRLF);
    terminal.write(EC_BLD "A" EC_NOF ". d" EC_BLD "A" EC_NOF "te and time" EC_CRLF);
    if (displayPreferences.currentDisplayMode().supportsBluetooth) {
      terminal.write(EC_BLD "B" EC_NOF ". " EC_BLD "B" EC_NOF "luetooth" EC_CRLF);
    }
    terminal.write(EC_BLD "S" EC_NOF ". " EC_BLD "S" EC_NOF "how capabilities (test page)");
  
    terminal.write(EC_CRLF  EC_CRLF);
    terminal.write(EC_BLD "ESC" EC_NOF ". " EC_BLD "Cancel (close)" EC_NOF);

    terminal.write(EC_CRLF EC_CRLF EC_CRLF EC_CRLF EC_DWI EC_ULN "ABOUT" EC_NOF EC_CRLF EC_CRLF);

    terminal.write("SporosTerm v" EC_STR(VERSION_NUMBER) EC_CRLF);
    terminal.write("Copyright " EC_COPR " 2025 Sporos Tech, Peter de Vroomen" EC_CRLF EC_CRLF);
    terminal.write("Using the FabGL library, Copyright " EC_COPR " 2019-2022 Fabrizio Di Vittorio." EC_CRLF EC_CRLF);
    terminal.write("Many thanks to Fabrizio Di Vittorio and Just4Fun!");

    terminal.write(EC_ETX);
  }

  SettingsPageAction handleChoice(uint8_t choice) override {

    // NOTE: While rendering the screen, this gets called and an 'x' or 'X' will finish the settings screen
    // NOTE: again. We should ignore everything that is sent to us (see terminal.onUserReceive()) until rendering
    // NOTE: finished. How can we do that without hacking more into FabGL?

    switch (choice) {
      case VirtualKey::VK_ESCAPE:
        return endSettings;
      case VirtualKey::VK_p:
      case VirtualKey::VK_P:
        return gotoSerialPortSettingsPage;
      case VirtualKey::VK_t:
      case VirtualKey::VK_T:
        return gotoTerminalSettingsPage;
      case VirtualKey::VK_d:
      case VirtualKey::VK_D:
        return gotoDisplaySettingsPage;
      case VirtualKey::VK_a:
      case VirtualKey::VK_A:
        return gotoDateTimeSettingsPage;
      case VirtualKey::VK_b:
      case VirtualKey::VK_B:
        if (displayPreferences.currentDisplayMode().supportsBluetooth) {
          return gotoBluetoothSettingsPage;
        }
        else {
          return noFurtherAction;
        }
      case VirtualKey::VK_s:
      case VirtualKey::VK_S:
        return gotoTestSettingsPage;
      default:
        return noFurtherAction;
     }
  }
};