#include "fabutils.h"
#include "SettingsManagerPage.h"
#include "TerminalEscapeCodeDefines.h"
#include "BluetoothPreferences.h"

#pragma once

class SettingsManager;

class SettingsTestPage: public SettingsManagerPage {

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
     }
     return noFurtherAction;
  }

private:

  char scratchBuf[133];

  void render() {
    
    terminal.write(EC_STX);

    terminal.write(EC_COF EC_CLS EC_CHM);

    DisplayMode currentDisplayMode  = displayPreferences.currentDisplayMode();

    for (int row = 0; row < (currentDisplayMode.rows - 1); row++) {
      if (row == 0) {
        for (int col = 0; col < currentDisplayMode.columns; col++) {
          sprintf(scratchBuf, "%1d", col % 10);
          terminal.write(scratchBuf);
        }
      }
      else if (row == 1) {
        for (int col = 0; col < currentDisplayMode.columns; col++) {
          if (col % 10 == 0) {
            if (col == 0) {
              sprintf(scratchBuf, "1");     // Exception because it looks better to have a 1 here.
            }
            else {
              sprintf(scratchBuf, "%1d", col / 10);
            }
            terminal.write(scratchBuf);
          }
          else {
            terminal.write(" ");
          }
        }
      }
      else {
        sprintf(scratchBuf, "%1d", row);
        terminal.write(scratchBuf);
      }

      if (row == 2) {
        terminal.write("   ");
        for (uint8_t ch = 0; ch < 64; ch++) {
          sprintf(scratchBuf, "%c", printableChar(ch));
          terminal.write(scratchBuf);          
        }
      }
      else if (row == 3) {
        terminal.write("   ");
        for (uint8_t ch = 64; ch < 128; ch++) {
          sprintf(scratchBuf, "%c", printableChar(ch));
          terminal.write(scratchBuf);          
        }
      }
      else if (row == 4) {
        terminal.write("   ");
        for (uint8_t ch = 128; ch < 192; ch++) {
          sprintf(scratchBuf, "%c", printableChar(ch));
          terminal.write(scratchBuf);          
        }
      }
      else if (row == 5) {
        terminal.write("   ");
        for (uint8_t ch = 192; ch < 255; ch++) {
          sprintf(scratchBuf, "%c", printableChar(ch));
          terminal.write(scratchBuf);          
        }
      }
      else if (row == 7) {
        sprintf(scratchBuf, "   Resolution: %dx%d (%dx%d characters)", currentDisplayMode.xRes, currentDisplayMode.yRes, currentDisplayMode.columns, currentDisplayMode.rows);
        terminal.write(scratchBuf);
      }
      else if (row == 9) {
        terminal.write("    Colors:");
      }
      else if (row == 10) {
        terminal.write("      ");
        for (uint8_t color = 30; color <= 37; color++) {
          sprintf(scratchBuf, EC_NOF "[\e[%dm" EC_REV "%2d" EC_NOF "] ", color, color);
          terminal.write(scratchBuf);          
        }
        sprintf(scratchBuf, "\e[%dm", 32);        
        terminal.write(scratchBuf);          
      }
      else if (row == 11) {
        terminal.write("      ");
        for (uint8_t color = 90; color <= 97; color++) {
          sprintf(scratchBuf, EC_NOF "[\e[%dm" EC_REV "%2d" EC_NOF "] ", color, color);
          terminal.write(scratchBuf);          
        }
        sprintf(scratchBuf, "\e[%dm", 32);        
        terminal.write(scratchBuf);          
      }

      terminal.write(EC_CRLF);
    }

    terminal.write("\e_GPEN255;255;255$");
    terminal.write("\e_GPENW1$");
    sprintf(scratchBuf, "\e_GRECT0;0;%d;%d$", currentDisplayMode.xRes - 1, currentDisplayMode.yRes - 1);
    // sprintf(scratchBuf, "\e_GRECT0;0;100;100$", currentDisplayMode.xRes - 1, currentDisplayMode.yRes - 1);
    terminal.write(scratchBuf);


    sprintf(scratchBuf, "%1d", (displayPreferences.currentDisplayMode().rows - 1));
    terminal.write(scratchBuf);

    terminal.write(EC_ETX);
  }

  char printableChar(char c) {
    return (((c & 0xFF) < 32) || ((c % 0xFF) == 127)) ? '.' : c;
  }

};