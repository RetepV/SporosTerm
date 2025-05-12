#include "SerialPortPreferences.h"

#pragma once

#define STATUSBAR_DELIM     ' '

class StatusBar;
extern StatusBar statusBar;

class StatusBar {

public:
  int statusBarRow;

  void start(int row) {

    statusBarRow = row;
    
    updateTimer = xTimerCreate("statusbar", pdMS_TO_TICKS(100), pdTRUE, (void*)0, statusBarUpdate);
    xTimerStart(updateTimer, 0);
  }

  void end() {
    xTimerStop(updateTimer, 0);
    xTimerDelete(updateTimer, 0);
  }

private:

  TimerHandle_t updateTimer;

  void statusBarWriteRaw(int positionX, int positionY, char *text) {

    fabgl::FontInfo font = terminal.font();
    fabgl::CanvasState savedState = terminal.canvas()->getCanvasState();

    fabgl::GlyphOptions glyphOptions;
    glyphOptions.value = 0;
    glyphOptions.fillBackground = 1;
    glyphOptions.invert = 1;
    terminal.canvas()->setGlyphOptions(glyphOptions);

    terminal.canvas()->drawText(&font, positionX * font.width, positionY * font.height, text);
    terminal.canvas()->setCanvasState(savedState);
  }

  static void statusBarUpdate(xTimerHandle pxTimer) {
    
    time_t currentTime;
    struct tm *currentLocalTime;
    time(&currentTime);
    currentLocalTime = localtime(&currentTime);

    char buffer[81];
    memset(buffer, ' ', 81);
    buffer[80] = 0;
    
    sprintf(&buffer[0], "%03d,%03d", terminal.cursorCol(), terminal.cursorRow());

    sprintf(&buffer[7], "%c%s", STATUSBAR_DELIM, terminal.keyboard()->isCapsLock() ? "CAP" : "cap");
    sprintf(&buffer[11], "%c%s", STATUSBAR_DELIM, terminal.keyboard()->isNumLock() ? "NUM" : "num");
    sprintf(&buffer[15], "%c%s", STATUSBAR_DELIM, terminal.keyboard()->isScrollLock() ? "SCR" : "scr");
    buffer[19] = ' ';

    sprintf(&buffer[48], "%c%3s", STATUSBAR_DELIM, "NUL");
    sprintf(&buffer[52], "%c%3s", STATUSBAR_DELIM, "RTS");

    float stopBits = serialPortPreferences.currentStopBits();
    sprintf(&buffer[56], "%c%6d,%c,%1d,%1d.%1d", STATUSBAR_DELIM,
                                                 serialPortPreferences.currentBaudRate(),
                                                 serialPortPreferences.currentParity(),
                                                 serialPortPreferences.currentDataSize(),
                                                 int(stopBits), int(10 * (stopBits - int(stopBits))));
    sprintf(&buffer[71], "%c%02d:%02d:%02d", STATUSBAR_DELIM, currentLocalTime->tm_hour, currentLocalTime->tm_min, currentLocalTime->tm_sec);

    statusBar.statusBarWriteRaw(0, statusBar.statusBarRow, buffer);
  }
 };

 StatusBar       statusBar;
