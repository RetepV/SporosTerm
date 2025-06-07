#include "fabutils.h"
#include "SettingsManagerPage.h"
#include "TerminalEscapeCodeDefines.h"
#include "DateTimePreferences.h"

#pragma once

class SettingsManager;

class SettingsDateTimePage: public SettingsManagerPage {

public:
  SettingsManager *settingsManager;

  void show() override {

    dateTimePreferences.fetch();
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
        dateTimePreferences.save();
        dateTimePreferences.apply();
        return gotoMainSettingsPage;
      case VirtualKey::VK_EXCLAIM:
        dateTimePreferences.convertOrReset(true);
        render();
        break;
      case VirtualKey::VK_h:
        dateTimePreferences.selectNextHour();
        render();
        break;
      case VirtualKey::VK_H:
        dateTimePreferences.selectPrevHour();
        render();
        break;
      case VirtualKey::VK_m:
        dateTimePreferences.selectNextMinute();
        render();
        break;
      case VirtualKey::VK_M:
        dateTimePreferences.selectPrevMinute();
        render();
        break;
      case VirtualKey::VK_s:
        dateTimePreferences.selectNextSecond();
        render();
        break;
      case VirtualKey::VK_S:
        dateTimePreferences.selectPrevSecond();
        render();
        break;

      case VirtualKey::VK_d:
        dateTimePreferences.selectNextDay();
        render();
        break;
      case VirtualKey::VK_D:
        dateTimePreferences.selectPrevDay();
        render();
        break;
      case VirtualKey::VK_o:
        dateTimePreferences.selectNextMonth();
        render();
        break;
      case VirtualKey::VK_O:
        dateTimePreferences.selectPrevMonth();
        render();
        break;
      case VirtualKey::VK_y:
        dateTimePreferences.selectNextYear();
        render();
        break;
      case VirtualKey::VK_Y:
        dateTimePreferences.selectPrevYear();
        render();
        break;
    }
     return noFurtherAction;
  }

private:

  char scratchBuf[11];

  void render() {
    
    terminal.write(EC_STX);

    terminal.write(EC_CLRTABS);
    terminal.write("\e[0;25H" EC_SETTAB);
    terminal.write("\e[0;40H" EC_SETTAB);

    terminal.write(EC_COF EC_CLS EC_CHM EC_DWI EC_ULN "SETTINGS - DATE AND TIME" EC_NOF EC_CRLF EC_CRLF);

    terminal.write(EC_BLD "H" EC_NOF ". " EC_BLD "H" EC_NOF "ours  " EC_BLD "M" EC_NOF ". " EC_BLD "M" EC_NOF "inutes  "EC_BLD "S" EC_NOF ". " EC_BLD "S" EC_NOF "econds");
    terminal.write(EC_CRLF);
    sprintf(scratchBuf, "%02D:%02D:%02D", dateTimePreferences.selectedHours(), dateTimePreferences.selectedMinutes(), dateTimePreferences.selectedSeconds());
    terminal.write(EC_CRLF);
    terminal.write("          ");
    terminal.write(scratchBuf);
    terminal.write(EC_CRLF EC_CRLF);

    terminal.write(EC_BLD "D" EC_NOF ". " EC_BLD "D" EC_NOF "ay  "EC_BLD "O" EC_NOF ". m" EC_BLD "O" EC_NOF "nth  "EC_BLD "Y" EC_NOF ". " EC_BLD "Y" EC_NOF "ear");
    terminal.write(EC_CRLF);
    sprintf(scratchBuf, "%02D-%02D-%04D", dateTimePreferences.selectedDay(), dateTimePreferences.selectedMonth(), dateTimePreferences.selectedYear());
    terminal.write(EC_CRLF);
    terminal.write("          ");
    terminal.write(scratchBuf);
    terminal.write(EC_CRLF EC_CRLF);

    terminal.write(EC_BLD "Z" EC_NOF ". time " EC_BLD "Z" EC_NOF "one\t");
    terminal.write("UTC");
    terminal.write(EC_CRLF EC_CRLF);

    terminal.write(EC_BLD "A" EC_NOF ". s" EC_BLD "A" EC_NOF "ve and go back");
    terminal.write(EC_CRLF EC_CRLF);
    terminal.write(EC_BLD "ESC" EC_NOF ". " EC_BLD "Cancel" EC_NOF EC_CRLF);

    terminal.write(EC_CRLF EC_CRLF EC_CRLF "(unshifted letter selects next, shifted letter selects previous)");

    terminal.write(EC_ETX);
  }
};