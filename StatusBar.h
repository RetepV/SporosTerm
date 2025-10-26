#include "comdrivers/serialport.h"
#include "SerialPortPreferences.h"
#include "RealTimeClock.h"

#pragma once

#define STATUSBAR_DELIM                 ' '

class StatusBar;
extern StatusBar statusBar;

class StatusBar {

private:

  TimerHandle_t updateTimer;

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

  static void statusBarUpdate(xTimerHandle pxTimer) {

    static bool updateTimeNow = true;     // Used to limit the amount of reads per second from the RTC a bit.
    static MacAbsoluteTime currentTime = MacAbsoluteTime(1, 0);  

    if (updateTimeNow) {
      currentTime.update(realTimeClock.readTime(), 0);
    }
    updateTimeNow = ~updateTimeNow;

    char buffer[128];
    float stopBits = serialPortPreferences.currentStopBits();

    sprintf(buffer,
      "%03d,%03d"
      "%c%s"
      "%c%s"
      "%c%s"
      "              "      // 24 spaces (can we do this nicer?)
      "%c%3s"
      "%c%3s"
      "%c%3s"
      "%c%6d,%c,%1d,%1d.%1d"
      "%c%02d-%02d-%04d"
      "%c%02d:%02d:%02d"
      ,
      terminal.cursorCol(), terminal.cursorRow(),
      STATUSBAR_DELIM, terminal.keyboard()->isCapsLock() ? "CAP" : "cap",
      STATUSBAR_DELIM, terminal.keyboard()->isNumLock() ? "NUM" : "num",
      STATUSBAR_DELIM, terminal.keyboard()->isScrollLock() ? "SCR" : "scr",
      STATUSBAR_DELIM, serialPort.BRKStatus() ? "BRK" : "   ",
      STATUSBAR_DELIM, serialPortPreferences.currentModemTypeShortString(),
      STATUSBAR_DELIM, serialPortPreferences.currentFlowControlShortString(),
      STATUSBAR_DELIM,
        serialPortPreferences.currentBaudRate(),
        serialPortPreferences.currentParity(),
        serialPortPreferences.currentDataSize(),
        int(stopBits), int(10 * (stopBits - int(stopBits))),
      STATUSBAR_DELIM, currentTime.components.tm_mday, currentTime.components.tm_mon + 1, currentTime.components.tm_year + 1900,
      STATUSBAR_DELIM, currentTime.components.tm_hour, currentTime.components.tm_min, currentTime.components.tm_sec
    );

    fabgl::FontInfo font = terminal.font();
    
    fabgl::RGB888 penColor = fabgl::RGB888(255, 255, 255);
    fabgl::RGB888 brushColor = fabgl::RGB888(0, 0, 0);
    
    fabgl::GlyphOptions glyphOptions;
    glyphOptions.value = 0;
    glyphOptions.fillBackground = 1;
    glyphOptions.invert = 1;

    terminal.canvas()->drawTextWithOptions(&font, 0, statusBar.statusBarRow * font.height, buffer, false, glyphOptions, penColor, brushColor);
  }
 };

 StatusBar       statusBar;
