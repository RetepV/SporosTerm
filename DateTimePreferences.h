#include <Preferences.h>
#include "MacAbsoluteTime.h"
#include "RealTimeClock.h"

#pragma once

class DateTimePreferences;
extern DateTimePreferences dateTimePreferences;

class DateTimePreferences {

private:

  // Note: No need to keep current time.
  MacAbsoluteTime   selectedTime = MacAbsoluteTime(1, 0);
  
public:
  void start() {
    selectedTime.update(realTimeClock.readTime(), 0);
    // If the clock is not running, assume that this is the first start, or that it has lost power.
    // Start the clock running with whatever it's current timestamp is.
    if (!realTimeClock.isClockRunning()) {
      realTimeClock.startStopClock(true);
    }

    fetch();
  }

  void end() {
  }

  void fetch() {
    selectedTime.update(realTimeClock.readTime(), 0);
  }

  void save() {
    realTimeClock.writeTime(selectedTime.timestampInSeconds);
    if (!realTimeClock.isClockRunning()) {
      realTimeClock.startStopClock(true);
    }
  }

  void convertOrReset(bool reset) {
    // Just fetch from RTC. Nothing to convert or reset. The RTC always defaults to 0, and that is fine.
    fetch();
  }

  void apply() {
    fetch();
  }

  int selectedYear() {
    return selectedTime.components.tm_year + 1900;
  }

  int selectedMonth() {
    return selectedTime.components.tm_mon + 1;
  }

  int selectedDay() {
    return selectedTime.components.tm_mday;
  }

  int selectedHours() {
    return selectedTime.components.tm_hour;
  }

  int selectedMinutes() {
    return selectedTime.components.tm_min;
  }

  int selectedSeconds() {
    return selectedTime.components.tm_sec;
  }

  void selectNextSecond() {
    selectedTime.incSeconds();
  }

  void selectPrevSecond() {
    selectedTime.decSeconds();
  }

  void selectNextMinute() {
    selectedTime.incMinutes();
  }

  void selectPrevMinute() {
    selectedTime.decMinutes();
  }

  void selectNextHour() {
    selectedTime.incHours();
  }

  void selectPrevHour() {
    selectedTime.decHours();
  }

  void selectNextDay() {
    selectedTime.incDays();
  }

  void selectPrevDay() {
    selectedTime.decDays();
  }

  void selectNextMonth() {
    selectedTime.incMonths();
  }

  void selectPrevMonth() {
    selectedTime.decMonths();
  }

  void selectNextYear() {
    selectedTime.incYears();
  }

  void selectPrevYear() {
    selectedTime.decYears();
  }
};

DateTimePreferences dateTimePreferences;
