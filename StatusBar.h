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
      "                  "      // 28 spaces, can we do this nicer?
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

    // We must write to the canvas in 'raw' mode, using drawText. But that does not support escape-codes. So we
    // must set the glyphOptions ourself. However, the canvas has only one set of glyphOptions, and one renderqueue.
    // So we must:
    // 1. Stop the rendering queue, so that we take full control.
    // 2. Finish any rendering stuff that is still in the queue.
    // 3. Save the current glyphOptions.
    // 4. Set our own glyphOptions.
    // 5. Render our stuff.
    // 6. Finish rendering our stuff.
    // 7. Restore the saved glyphOptions.
    // 8. Re-enable the queue.
    //
    // This is all quite elaborate. But the worst part is that it doesn't really work, I am seeing glyphOptions of
    // the status bar appearing in the normal text area of the screen.
    //
    // I think the issue is that we don't want to queue to stop being processed, but want to *lock the queue for
    // ourselves*, so that nothing can enter it (e.g. from a higher priority process) while the status bar is being
    // rendered.
    // But we probably can't lock it, as that might halt the other processes, leading to missed data. This would again
    // be visible on the screen.
    // Maybe the solution is to hack somewhat into the queue system. Maybe we can add a priority system, which causes
    // higher priority things to be rendered before lower priority things.
    // The status bar would then stop and empty the queue. Then add its stuff with a high priority. Anything added
    // in the mean time (e.g. from an interrupt function), will go in the queue but not be rendered.
    // Then restart processing the queue. It will execute all high priority stuff first, and then continue with 
    // normal priority stuff.

    // Pause receiving of data for a short while. This is necessary because we are writing multiple Primitives
    // to the primitive queue of the DisplayController. We change the GlyphOptions temporarily here, and we don't
    // want to have any received data add Primitives in between because they will use the temporary GlyphOptions.
    // The ESP32's UART has a 128 bytes receive buffer, so if we are fast enough here, we won't affect receive
    //  speed (i.e. receive buffer will not fill and not cause actual pauses), or dropped characters due to buffer
    // overflow. Hopefully.
    serialPort.flowControl(false);
    // Stop processing the Primitive queue
    terminal.canvas()->beginUpdate();
    // Render the Primitive queue clean.
    terminal.canvas()->waitCompletion(false);

    fabgl::GlyphOptions glyphOptions;
    glyphOptions.value = 0;
    glyphOptions.fillBackground = 1;
    glyphOptions.invert = 1;

    fabgl::CanvasState savedState = terminal.canvas()->getCanvasState();
    terminal.canvas()->setGlyphOptions(glyphOptions);
    terminal.canvas()->drawText(&font, 0, statusBar.statusBarRow * font.height, buffer);
    terminal.canvas()->setCanvasState(savedState);

    // Re-enable receiving of data.
    serialPort.flowControl(true);
    // Re-enable processing the Primitive queue
    terminal.canvas()->endUpdate();
  }
 };

 StatusBar       statusBar;
