#include "fabutils.h"
#include "SettingsManagerPage.h"
#include "TerminalEscapeCodeDefines.h"
#include "BluetoothPreferences.h"

#pragma once

class SettingsManager;

class SettingsBluetoothPage: public SettingsManagerPage {

public:
  SettingsManager *settingsManager;

  void show() override {

    render();
  }

  SettingsPageAction handleChoice(uint8_t choice) override {

    // NOTE: While rendering the screen, this gets called and an 'x' or 'X' will finish the settings screen
    // NOTE: again. We should ignore everything that is sent to us (see terminal.onUserReceive()) until rendering
    // NOTE: finished. How can we do that without hacking more into FabGL?

    switch (choice) {
      case VirtualKey::VK_ESCAPE:
        Serial.printf("SettingsBluetoothPage: Cancel, goto main page\n");
        return gotoMainSettingsPage;
      case VirtualKey::VK_a:
      case VirtualKey::VK_A:
        Serial.printf("SettingsBluetoothPage: Save and exit/reset . main page\n");
        bluetoothPreferences.save();
        bluetoothPreferences.apply();
        return gotoMainSettingsPage;
      case VirtualKey::VK_EXCLAIM:
        bluetoothPreferences.convertOrReset(true);
        render();
        break;
      case VirtualKey::VK_e:
      case VirtualKey::VK_E:
        bluetoothPreferences.toggleSendDelayEnabled();
        render();
        break;
      case VirtualKey::VK_d:
        bluetoothPreferences.incSendDelayChar(10);
        render();
        break;
      case VirtualKey::VK_D:
        bluetoothPreferences.decSendDelayChar(10);
        render();
        break;
      case VirtualKey::VK_l:
        bluetoothPreferences.incSendDelayLine(10);
        render();
        break;
      case VirtualKey::VK_L:
        bluetoothPreferences.decSendDelayLine(10);
        render();
        break;
     }
     return noFurtherAction;
  }

private:

  char scratchBuf[6];

  void render() {
    
    terminal.write(EC_STX);

    terminal.write(EC_CLRTABS);
    terminal.write("\e[0;25H" EC_SETTAB);
    terminal.write("\e[0;40H" EC_SETTAB);

    terminal.write(EC_COF EC_CLS EC_CHM EC_DWI EC_ULN "SETTINGS - BLUETOOTH" EC_NOF EC_CRLF EC_CRLF);

    terminal.write(EC_BLD "E" EC_NOF ". Send d" EC_BLD "E" EC_NOF "lay enabled\t");
    terminal.write(bluetoothPreferences.selectedSendDelayEnabled ? "YES" : "NO");
    terminal.write(EC_CRLF);

    if (bluetoothPreferences.selectedSendDelayEnabled) {
      terminal.write(EC_BLD "D" EC_NOF ". " EC_BLD "D" EC_NOF "elay after character (ms)\t");
      sprintf(scratchBuf, "%d", bluetoothPreferences.selectedSendDelayCharMilliseconds);
      terminal.write(scratchBuf);
      terminal.write(EC_CRLF);
      terminal.write(EC_BLD "N" EC_NOF ". delay after " EC_BLD "N" EC_NOF "ewline (ms)\t");
      sprintf(scratchBuf, "%d", bluetoothPreferences.selectedSendDelayLineMilliseconds);
      terminal.write(scratchBuf);
      terminal.write(EC_CRLF);
    }

    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "!" EC_NOF ". reset to defaults" EC_BLD "!" EC_NOF "");
    terminal.write(EC_CRLF EC_CRLF);

    terminal.write(EC_BLD "ESC" EC_NOF ". " EC_BLD "C" EC_NOF "ancel" EC_CRLF);

    if (bluetoothPreferences.needsReset) {
      terminal.write(EC_BLD "A" EC_NOF ". s" EC_BLD "A" EC_NOF "ve and reset");
    }
    else {
      terminal.write(EC_BLD "A" EC_NOF ". s" EC_BLD "A" EC_NOF "ve and go back");
    }

    terminal.write(EC_CRLF EC_CRLF EC_CRLF "(unshifted letter selects next, shifted letter selects previous)");

    terminal.write(EC_ETX);
  }
};