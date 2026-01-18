#include "fabutils.h"
#include "SettingsManagerPage.h"
#include "SettingsMainPage.h"
#include "SettingsSerialPortPage.h"
#include "SettingsTerminalPage.h"
#include "SettingsDisplayPage.h"
#include "SettingsBluetoothPage.h"
#include "SettingsCapabilitiesPage.h"
#include "SettingsDateTimePage.h"

#include "GlyphsBufferSaver.h"

#pragma once

class SettingsManager {

public:
  SettingsManager() {    
  }

  ~SettingsManager() {
  }

  bool isActive() {
    return currentPage != NULL;
  }

  void show() {

    switchToLocalMode();

    originalBackgroundColor = terminal.getBackgroundColor();
    
    handleAction(gotoMainSettingsPage);

    terminal.onLocalModeVirtualKeyItem = [&](VirtualKeyItem *item) {
      if (interactive && item->down) {
        handleAction(currentPage->handleChoice(item->vk));
      }
    };

    terminal.onLocalModeReceive = [&](uint8_t value) {
      handleSpecialCharacters(value);
    };
  }

  void finish() {

    terminal.setBackgroundColor(originalBackgroundColor);

    if (currentPage != NULL) {
      delete currentPage;
      currentPage = NULL;
    }

    switchToNormalMode();
  }

private:

  SettingsManagerPage *currentPage = NULL;
  Color               originalBackgroundColor = Color::Black;
  bool                interactive = true;

  bool handleSpecialCharacters(uint8_t value) {

    switch (value) {
      case '\x02':        // STX Start of Text
        interactive = false;
        break;
      case '\x03':        // ETX End of Text
        interactive = true;
        break;
    }

    return (currentPage != NULL) && interactive;
  }

  void handleAction(SettingsPageAction action) {
    switch (action) {
      case noFurtherAction:
        return;
      case endSettings:
        finish();
        return;
      default:
        if (currentPage != NULL) {
          delete currentPage;
          currentPage = NULL;
        }

        currentPage = pageForAction(action);

        if (currentPage != NULL) {
          currentPage->show();
        }
        return;
    }
  }

  SettingsManagerPage *pageForAction(SettingsPageAction action) {
    switch (action) {
      case gotoMainSettingsPage:
        return new SettingsMainPage;
      case gotoSerialPortSettingsPage:
        return new SettingsSerialPortPage;
      case gotoTerminalSettingsPage:
        return new SettingsTerminalPage;
      case gotoDisplaySettingsPage:
        return new SettingsDisplayPage;
      case gotoBluetoothSettingsPage:
        return new SettingsBluetoothPage;
      case gotoCapabilitiesPage:
        return new SettingsCapabilitiesPage;
      case gotoDateTimeSettingsPage:
        return new SettingsDateTimePage;
      default:
        return NULL;
    }
  }
};