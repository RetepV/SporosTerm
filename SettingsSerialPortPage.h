#include "fabutils.h"
#include "SettingsManagerPage.h"
#include "TerminalEscapeCodeDefines.h"
#include "SerialPortPreferences.h"

#pragma once

class SettingsManager;

class SettingsSerialPortPage: public SettingsManagerPage {

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
        Serial.printf("SettingsSerialPortPage: Cancel . main page\n");
        return gotoMainSettingsPage;
      case VirtualKey::VK_a:
      case VirtualKey::VK_A:
        Serial.printf("SettingsSerialPortPage: Save and exit/reset . main page\n");
        serialPortPreferences.save();
        serialPortPreferences.apply();
        return gotoMainSettingsPage;
      case VirtualKey::VK_EXCLAIM:
        serialPortPreferences.convertOrReset(true);
        render();
        break;
      case VirtualKey::VK_b:
        serialPortPreferences.selectNextBaudRate();
        render();
        break;
      case VirtualKey::VK_B:
        serialPortPreferences.selectPrevBaudRate();
        render();
        break;
      case VirtualKey::VK_p:
        serialPortPreferences.selectNextParity();
        render();
        break;
      case VirtualKey::VK_P:
        serialPortPreferences.selectPrevParity();
        render();
        break;
      case VirtualKey::VK_d:
        serialPortPreferences.selectNextDataSize();
        render();
        break;
      case VirtualKey::VK_D:
        serialPortPreferences.selectPrevDataSize();
        render();
        break;
      case VirtualKey::VK_s:
        serialPortPreferences.selectNextStopBits();
        render();
        break;
      case VirtualKey::VK_S:
        serialPortPreferences.selectPrevStopBits();
        render();
        break;
      case VirtualKey::VK_f:
        serialPortPreferences.selectNextFlowControl();
        render();
        break;
      case VirtualKey::VK_F:
        serialPortPreferences.selectPrevFlowControl();
        render();
        break;
      case VirtualKey::VK_e:
      case VirtualKey::VK_E:
        serialPortPreferences.toggleSendDelayEnabled();
        render();
        break;
      case VirtualKey::VK_l:
        serialPortPreferences.incSendDelayChar(10);
        render();
        break;
      case VirtualKey::VK_L:
        serialPortPreferences.decSendDelayChar(10);
        render();
        break;
      case VirtualKey::VK_n:
        serialPortPreferences.incSendDelayLine(10);
        render();
        break;
      case VirtualKey::VK_N:
        serialPortPreferences.decSendDelayLine(10);
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

    terminal.write(EC_COF EC_CLS EC_CHM EC_DWI EC_ULN "SETTINGS - SERIAL PORT" EC_NOF EC_CRLF EC_CRLF);

    terminal.write(EC_BLD "B" EC_NOF ". " EC_BLD "B" EC_NOF "aud rate\t");
    terminal.write(serialPortPreferences.selectedBaudRateString());
    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "P" EC_NOF ". " EC_BLD "P" EC_NOF "arity\t");
    terminal.write(serialPortPreferences.selectedParityString());
    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "D" EC_NOF ". " EC_BLD "D" EC_NOF "ata size\t");
    terminal.write(serialPortPreferences.selectedDataSizeString());
    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "S" EC_NOF ". " EC_BLD "S" EC_NOF "top bits\t");
    terminal.write(serialPortPreferences.selectedStopBitsString());
    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "F" EC_NOF ". " EC_BLD "F" EC_NOF "low control\t");
    terminal.write(serialPortPreferences.selectedFlowControlString());
    terminal.write(EC_CRLF);

    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "!" EC_NOF ". reset to defaults" EC_BLD "!" EC_NOF "");
    terminal.write(EC_CRLF EC_CRLF);

    terminal.write(EC_CRLF EC_BLD "ESC" EC_NOF ". " EC_BLD "Cancel" EC_NOF EC_CRLF);
    if (serialPortPreferences.needsReset) {
      terminal.write(EC_BLD "A" EC_NOF ". s" EC_BLD "A" EC_NOF "ve and reset");
    }
    else {
      terminal.write(EC_BLD "A" EC_NOF ". s" EC_BLD "A" EC_NOF "ve and go back");
    }

    terminal.write(EC_CRLF EC_CRLF EC_CRLF "(unshifted letter selects next, shifted letter selects previous)");

    terminal.write(EC_ETX);
  }
};