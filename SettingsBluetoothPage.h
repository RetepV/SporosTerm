#include "fabutils.h"
#include "SettingsManagerPage.h"
#include "GlobalDefines.h"
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
        return gotoMainSettingsPage;
      case VirtualKey::VK_a:
      case VirtualKey::VK_A:
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
      case VirtualKey::VK_n:
        bluetoothPreferences.incSendDelayLine(10);
        render();
        break;
      case VirtualKey::VK_N:
        bluetoothPreferences.decSendDelayLine(10);
        render();
        break;
     }
     return noFurtherAction;
  }

private:

  void render() {

    DisplayMode currentDisplayMode  = displayPreferences.currentDisplayMode();
    char scratchBuf[16];
    
    terminal.write(EC_STX);

    terminal.write(EC_CLRTABS);
    terminal.write("\e[0;25H" EC_SETTAB);
    terminal.write("\e[0;40H" EC_SETTAB);

    terminal.write(EC_COF EC_CLS EC_CHM EC_DWI EC_ULN "SETTINGS - BLUETOOTH" EC_NOF EC_CRLF EC_CRLF);

    if (currentDisplayMode.supportsBluetooth) {
      terminal.write(EC_BLD "E" EC_NOF ". Send d" EC_BLD "E" EC_NOF "lay enabled\t");
      terminal.write(bluetoothPreferences.selectedSendDelayEnabled ? "Yes" : "No");
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
    }
    else {
      terminal.write("Bluetooth is not enabled for this display mode." EC_CRLF);
      terminal.write("Please choose a display mode that supports bluetooth to enable these settings.");
    }

    buildCursorPosCode(0,currentDisplayMode.rows - 4, scratchBuf);
    terminal.write(scratchBuf);

    terminal.write(EC_BLD "!" EC_NOF ".   Reset settings to defaults" EC_BLD "!" EC_NOF EC_CRLF);
    if (bluetoothPreferences.needsReset) {
      terminal.write(EC_BLD "A" EC_NOF ".   " EC_BLD "A" EC_NOF "pply changes and reset");
    }
    else {
      terminal.write(EC_BLD "A" EC_NOF ".   " EC_BLD "A" EC_NOF "pply changes and go back");
    }
    terminal.write(EC_CRLF);
    terminal.write(EC_BLD "ESC" EC_NOF ". " EC_BLD "Discard changes and go back" EC_NOF EC_CRLF);

    terminal.write(EC_CRLF "(unshifted letter selects next value, shifted letter selects previous)");

    terminal.write(EC_ETX);
  }
};