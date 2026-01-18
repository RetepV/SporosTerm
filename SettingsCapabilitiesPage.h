#include "fabutils.h"
#include "SettingsManagerPage.h"
#include "GlobalDefines.h"
#include "BluetoothPreferences.h"

#pragma once

class SettingsManager;

class SettingsCapabilitiesPage: public SettingsManagerPage {

public:
  SettingsManager *settingsManager;

  void show() override {

    render();
  }

  SettingsPageAction handleChoice(uint8_t choice) override {
    switch (choice) {
      case VirtualKey::VK_ESCAPE:
        terminal.canvas()->clear();
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

    terminal.write("\e[0;0H");

    for (int row = 0; row < currentDisplayMode.rows; row++) {
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
              sprintf(scratchBuf, "1");     // Exception because it just looks better to have a 1 at column 0.
            }
            else {
              // In 132 cols mode, column can become 10 or larger, needing 2 characters. And then we
              // write too many characters to the screen causing weird display. We also don't want to
              // sacrifice another row. So just show the column divided by 10 and mod 10.
              sprintf(scratchBuf, "%1d", (col / 10) % 10);       
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

      if (row == 3) {
        sprintf(scratchBuf, "   Resolution: %dx%d (%dx%d characters)", currentDisplayMode.xRes, currentDisplayMode.yRes, currentDisplayMode.columns, currentDisplayMode.rows);
        terminal.write(scratchBuf);
      }
      else if (row == 5) {
        terminal.write("   ");
        for (uint8_t ch = 0; ch < 64; ch++) {
          sprintf(scratchBuf, "%c", printableChar(ch));
          terminal.write(scratchBuf);          
        }
      }
      else if (row == 6) {
        terminal.write("   ");
        for (uint8_t ch = 64; ch < 128; ch++) {
          sprintf(scratchBuf, "%c", printableChar(ch));
          terminal.write(scratchBuf);          
        }
      }
      else if (row == 7) {
        terminal.write("   ");
        for (uint8_t ch = 128; ch < 192; ch++) {
          sprintf(scratchBuf, "%c", printableChar(ch));
          terminal.write(scratchBuf);          
        }
      }
      else if (row == 8) {
        terminal.write("   ");
        for (uint8_t ch = 192; ch < 255; ch++) {
          sprintf(scratchBuf, "%c", printableChar(ch));
          terminal.write(scratchBuf);          
        }
      }
      else if (row == 10) {
        terminal.write("   " EC_BLD "BOLD" EC_NOF "  " EC_LOI "Low intensity" EC_NOF "  " EC_ULN "Underline" EC_NOF "  " EC_BLK "Blinking" EC_NOF "  " EC_REV "Reverse" EC_NOF "  " EC_INV "Invisible" EC_NOF "<-Invisible");
      }
       else if (row == 12) {
         terminal.write("\tTAB\tTAB\tTAB\tTAB");
       }
      else if (row == 14) {
        terminal.write(EC_DWI " Double width" EC_NOF);
      }
      else if (row == 15) {
        terminal.write(EC_DHI_T "  Double height" EC_NOF);
      }
      else if (row == 16) {
        terminal.write(EC_DHI_B "  Double height" EC_NOF);
      }

      else if (row == 18) {
        sprintf(scratchBuf, "   Colors (%d):", currentDisplayMode.colors);
        terminal.write(scratchBuf);          
      }
      else if (row == 19) {
        terminal.write("      ");
        for (uint8_t color = 30; color <= 37; color++) {
          sprintf(scratchBuf, EC_NOF "[\e[%dm" EC_REV "%2d" EC_NOF "] ", color, color);
          terminal.write(scratchBuf);          
        }
      }
      else if (row == 20) {
        terminal.write("      ");
        for (uint8_t color = 90; color <= 97; color++) {
          sprintf(scratchBuf, EC_NOF "[\e[%dm" EC_REV "%2d" EC_NOF "] ", color, color);
          terminal.write(scratchBuf);          
        }
      }
      else if (row == (currentDisplayMode.rows - 2)) {
        terminal.write("   " EC_BLD "ESC" EC_NOF ". " EC_BLD "Go back" EC_NOF);
      }

      if (row != (currentDisplayMode.rows - 1)) {
        terminal.write(EC_CRLF);
      }
    }

    // Give terminal a little time to render everything before drawing the rectangle. Otherwise the letters will
    // overwrite the rectangle's lines.
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // Draw a bright white rectangle around the screen. This helps with auto resizing of a screen.
    terminal.canvas()->setPenColor(fabgl::BrightWhite);
    terminal.canvas()->drawRectangle(0, 0, currentDisplayMode.xRes - 1, currentDisplayMode.yRes - 1);

    terminal.write(EC_ETX);
  }

  char printableChar(char c) {
    return (((c & 0xFF) < 32) || ((c % 0xFF) == 127)) ? '.' : c;
  }

};