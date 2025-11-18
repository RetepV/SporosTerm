#include "fabutils.h"
#include "SettingsManagerPage.h"
#include "GlobalDefines.h"
#include "DisplayPreferences.h"

#pragma once

class SettingsManager;

class SettingsDisplayPage: public SettingsManagerPage {

public:
  SettingsManager *settingsManager;

  void show() override {
    
    render();
  }

  SettingsPageAction handleChoice(uint8_t choice) override {

    if (askingForRestart) {
      if (choice == VirtualKey::VK_SPACE) {
        askingForRestart = false;
        displayPreferences.save();
        displayPreferences.apply();
        return gotoMainSettingsPage;
      }
      else {
        askingForRestart = false;
        render();
      }
    }
    else {
      switch (choice) {
        case VirtualKey::VK_ESCAPE:
          return gotoMainSettingsPage;
        case VirtualKey::VK_a:
        case VirtualKey::VK_A:
          if (displayPreferences.hasChanges()) {
            askingForRestart = true;
            render();
          }
          break;
        case VirtualKey::VK_EXCLAIM:
          displayPreferences.convertOrReset(true);
          render();
          break;
        case VirtualKey::VK_d:
          displayPreferences.selectNextDisplayMode();
          render();
          break;
        case VirtualKey::VK_D:
          displayPreferences.selectPrevDisplayMode();
          render();
          break;
      }
    }
    
    return noFurtherAction;
  }

private:

  bool needsRestart = false;
  bool askingForRestart = false;

  void render() {
    
    DisplayMode currentDisplayMode  = displayPreferences.currentDisplayMode();
    char scratchBuf[64];

    terminal.write(EC_STX);

    terminal.write(EC_CLRTABS);
    terminal.write("\e[0;30H" EC_SETTAB);
    terminal.write("\e[0;40H" EC_SETTAB);

    terminal.write(EC_COF EC_CLS EC_CHM EC_DWI EC_ULN "SETTINGS - DISPLAY" EC_NOF EC_CRLF EC_CRLF);

    DisplayMode selectedDisplayMode = displayPreferences.selectedDisplayMode();

    terminal.write(EC_BLD "D" EC_NOF ". " EC_BLD "D" EC_NOF "isplay mode\t");
    terminal.write(EC_CRLF EC_CRLF);

    terminal.write("  Display resolution\t");
    terminal.write(selectedDisplayMode.resolution);
    terminal.write(EC_CRLF);

    terminal.write("  Display characters\t");
    sprintf(scratchBuf, "%dx%d (%dx%d font)", selectedDisplayMode.columns, selectedDisplayMode.rows, selectedDisplayMode.font->width, selectedDisplayMode.font->height);
    terminal.write(scratchBuf);
    terminal.write(EC_CRLF);

    terminal.write("  Display colors\t");
    sprintf(scratchBuf, "%d", selectedDisplayMode.colors);
    terminal.write(scratchBuf);
    terminal.write(EC_CRLF);

    terminal.write("  Status bar\t");
    terminal.write(selectedDisplayMode.enableStatusBar ? "on" : "off");
    terminal.write(EC_CRLF);

    terminal.write("  Supports Bluetooth\t");
    terminal.write(selectedDisplayMode.supportsBluetooth ? "yes" : "no");
    terminal.write(EC_CRLF);

    if (askingForRestart) {
      buildCursorPosCode(6,18, scratchBuf);
      terminal.write(scratchBuf);

      terminal.write(">>> " EC_ULN EC_BLK "PRESS SPACE TO ACCEPT AND RESTART, ANY OTHER KEY TO CANCEL" EC_NOF " <<<");
    }
    else {
      buildCursorPosCode(0,currentDisplayMode.rows - 4, scratchBuf);
      terminal.write(scratchBuf);

      terminal.write(EC_BLD "!" EC_NOF   ".   Reset settings to defaults" EC_BLD "!" EC_NOF EC_CRLF);
      if (displayPreferences.hasChanges()) {
        terminal.write(EC_BLD "A" EC_NOF ". " EC_BLD "  A" EC_NOF "pply changes and reset" EC_CRLF);
      }
      else {
        terminal.write(EC_CRLF);
      }
      terminal.write(EC_BLD "ESC" EC_NOF ". " EC_BLD "Discard changes and go back" EC_NOF EC_CRLF);
      terminal.write(EC_CRLF "(unshifted letter selects next value, shifted letter selects previous)");
    }

    terminal.write(EC_ETX);
  }
};