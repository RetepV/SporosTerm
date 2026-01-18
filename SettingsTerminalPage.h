#include "fabutils.h"
#include "SettingsManagerPage.h"
#include "GlobalDefines.h"
#include "TerminalPreferences.h"

#pragma once

class SettingsManager;

class SettingsTerminalPage: public SettingsManagerPage {

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
        terminalPreferences.save();
        terminalPreferences.apply();
        return gotoMainSettingsPage;
      case VirtualKey::VK_EXCLAIM:
        terminalPreferences.convertOrReset(true);
        render();
        break;
      case VirtualKey::VK_t:
        terminalPreferences.selectNextTerminalMode();
        render();
        break;
      case VirtualKey::VK_T:
        terminalPreferences.selectPrevTerminalMode();
        render();
        break;
      case VirtualKey::VK_e:
      case VirtualKey::VK_E:
        terminalPreferences.toggleLocalEcho();
        render();
        break;
      case VirtualKey::VK_n:
      case VirtualKey::VK_N:
        terminalPreferences.toggleNewLineMode();
        render();
        break;
      case VirtualKey::VK_s:
      case VirtualKey::VK_S:
        terminalPreferences.toggleSmoothScroll();
        render();
        break;
      case VirtualKey::VK_k:
      case VirtualKey::VK_K:
        terminalPreferences.toggleAutoRepeat();
        render();
        break;
      case VirtualKey::VK_l:
      case VirtualKey::VK_L:
        terminalPreferences.toggleCursorBlink();
        render();
        break;
      case VirtualKey::VK_y:
        terminalPreferences.selectNextCursorStyle();
        render();
        break;
      case VirtualKey::VK_Y:
        terminalPreferences.selectPrevCursorStyle();
        render();
        break;
      case VirtualKey::VK_b:
      case VirtualKey::VK_B:
        terminalPreferences.toggleBackspaceStyle();
        render();
        break;
      case VirtualKey::VK_w:
      case VirtualKey::VK_W:
        terminalPreferences.toggleWrapAround();
        render();
        break;
      case VirtualKey::VK_r:
      case VirtualKey::VK_R:
        terminalPreferences.toggleReverseWrapAround();
        render();
        break;
      case VirtualKey::VK_h:
      case VirtualKey::VK_H:
        terminalPreferences.toggleHideSignonLogo();
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

    terminal.write(EC_COF EC_CLS EC_CHM EC_DWI EC_ULN "SETTINGS - TERMINAL" EC_NOF EC_CRLF EC_CRLF);

    terminal.write(EC_BLD "T" EC_NOF ". " EC_BLD "T" EC_NOF "erminal mode\t");
    terminal.write(terminalPreferences.selectedTerminalModeName());
    terminal.write(EC_CRLF EC_CRLF);

    terminal.write(EC_BLD "E" EC_NOF ". Local " EC_BLD "E" EC_NOF "cho\t");
    terminal.write(terminalPreferences.selectedLocalEcho ? "On" : "Off");
    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "N" EC_NOF ". " EC_BLD "N" EC_NOF "ewline mode\t");
    terminal.write(terminalPreferences.selectedNewLineMode ? "CR+LF" : "CR");
    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "S" EC_NOF ". " EC_BLD "S" EC_NOF "croll\t");
    terminal.write(terminalPreferences.selectedSmoothScroll ? "Smooth" : "Jump");
    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "K" EC_NOF ". " EC_BLD "K" EC_NOF "ey auto repeat\t");
    terminal.write(terminalPreferences.selectedAutoRepeat ? "Repeat" : "Don't repeat");
    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "L" EC_NOF ". cursor b" EC_BLD "L" EC_NOF "ink\t");
    terminal.write(terminalPreferences.selectedCursorBlink ? "Blink" : "Don't blink");
    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "Y" EC_NOF ". cursor st" EC_BLD "Y" EC_NOF "le\t");
    terminal.write(terminalPreferences.selectedCursorStyleString());
    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "B" EC_NOF ". " EC_BLD "B" EC_NOF "ackspace mode\t");
    terminal.write(terminalPreferences.selectedBackspaceStyle ? "Backspace" : "Delete");
    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "W" EC_NOF ". " EC_BLD "W" EC_NOF "rap around\t");
    terminal.write(terminalPreferences.selectedWrapAround ? "Wrap" : "Don't wrap");
    terminal.write(EC_CRLF);

    terminal.write(EC_BLD "R" EC_NOF ". " EC_BLD "R" EC_NOF "everse wrap around\t");
    terminal.write(terminalPreferences.selectedReverseWrapAround ? "Wrap" : "Don't wrap");
    terminal.write(EC_CRLF EC_CRLF);

    terminal.write(EC_BLD "H" EC_NOF ". " EC_BLD "H" EC_NOF "ide signon screen at startup\t");
    terminal.write(terminalPreferences.selectedHideSignonLogo ? "Hide signon screen" : "Don't hide signon screen");
    terminal.write(EC_CRLF EC_CRLF);

    buildCursorPosCode(0,currentDisplayMode.rows - 4, scratchBuf);
    terminal.write(scratchBuf);

    terminal.write(EC_BLD "!" EC_NOF ".   Reset settings to defaults" EC_BLD "!" EC_NOF "");
    terminal.write(EC_CRLF);

    if (terminalPreferences.needsReset) {
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