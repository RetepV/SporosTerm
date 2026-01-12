#include "stdlib.h"

static const unsigned short int daysBeforeMonths[2][13] = {
  /* Normal years.  */
  { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
  /* Leap years.  */
  { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};
static const unsigned short int daysInMonths[2][13] = {
  /* Normal years.  */
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  /* Leap years.  */
  { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

#define SECS_PER_MINUTE (60)
#define MINS_PER_HOUR (60)
#define SECS_PER_HOUR (MINS_PER_HOUR * SECS_PER_MINUTE)
#define HOURS_PER_DAY (24)
#define SECS_PER_DAY (HOURS_PER_DAY * SECS_PER_HOUR)
#define DAYS_PER_WEEK (7)
#define MONTHS_PER_YEAR (12)

#define TIME_BASE_SEC (0)
#define TIME_BASE_MIN (0)
#define TIME_BASE_YEAR (0)
#define TIME_BASE_DAY (1)
#define TIME_BASE_MONTH (1)
#define TIME_BASE_YEAR_TM (101)
#define TIME_BASE_WDAY (1)

#define TIME_BASE_YEAR (2001)
#define TIME_BASE_YEARS_SINCE_LEAP (1)
#define TIME_BASE_YEARS_SINCE_CENTURY (1)
#define TIME_BASE_YEARS_SINCE_LEAP_CENTURY (1)

#define TM_BASE_YEAR (1900)

#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(year) (DIV(year, 4) - DIV(year, 100) + DIV(year, 400))
#define ISLEAP(year) ((year) % 4 == 0 && ((year) % 100 != 0 || (year + TM_BASE_YEAR) % 400 == 0))
#define DAYS_IN_YEAR(year) (ISLEAP(year) ? 366 : 365)

// Timestamp base of a Mac Absolute Time is 1-1-2001 00:00:00 UTC+0
//
class MacAbsoluteTime {

public:

  time_t secondsOffsetFromUTC = 0;
  time_t timestampInSeconds = 0;
  bool isDaylightSaving = false;
  struct tm components;

  MacAbsoluteTime(time_t timestamp, time_t secondsFromUTC = 0) {
    update(timestamp, secondsFromUTC);
  }

  void update(time_t timestamp, time_t secondsFromUTC = 0) {
    secondsOffsetFromUTC = secondsFromUTC;
    timestampInSeconds = timestamp;
    toComponents(&components);
  }

  void update(struct tm *tmBuffer, time_t secondsFromUTC = 0) {
    secondsOffsetFromUTC = secondsFromUTC;
    components = *tmBuffer;
    timestampInSeconds = fromComponents(&components);
  }

  void incSeconds() {
    components.tm_sec++;
    if (components.tm_sec >= SECS_PER_MINUTE) {
      components.tm_sec = 0;
      incMinutes();
    } else {
      timestampInSeconds = fromComponents(&components);
      toComponents(&components);
    }
  }

  void decSeconds() {
    components.tm_sec--;
    if (components.tm_sec < 0) {
      components.tm_sec = SECS_PER_MINUTE - 1;
      decMinutes();
    } else {
      timestampInSeconds = fromComponents(&components);
      toComponents(&components);
    }
  }

  void incMinutes() {
    components.tm_min++;
    if (components.tm_min >= MINS_PER_HOUR) {
      components.tm_min = 0;
      incHours();
    } else {
      timestampInSeconds = fromComponents(&components);
      toComponents(&components);
    }
  }

  void decMinutes() {
    components.tm_min--;
    if (components.tm_min < 0) {
      components.tm_min = MINS_PER_HOUR - 1;
      decHours();
    } else {
      timestampInSeconds = fromComponents(&components);
      toComponents(&components);
    }
  }

  void incHours() {
    components.tm_hour++;
    if (components.tm_hour >= HOURS_PER_DAY) {
      components.tm_hour = 0;
      incDays();
    } else {
      timestampInSeconds = fromComponents(&components);
      toComponents(&components);
    }
  }

  void decHours() {
    components.tm_hour--;
    if (components.tm_hour < 0) {
      components.tm_hour = HOURS_PER_DAY - 1;
      decDays();
    } else {
      timestampInSeconds = fromComponents(&components);
      toComponents(&components);
    }
  }

  void incDays() {
    components.tm_mday++;
    components.tm_yday++;
    // Did we pass a month?
    if (components.tm_mday > daysInMonths[ISLEAP(components.tm_year)][components.tm_mon - 1]) {  // Note: day and month are 1-based
      components.tm_mday = 1;
      if (components.tm_yday > DAYS_IN_YEAR(components.tm_year)) {
        components.tm_yday = 1;
      }
      // Will increment the year if necessary.
      incMonths();
    } else {
      timestampInSeconds = fromComponents(&components);
      toComponents(&components);
    }
  }

  void decDays() {
    components.tm_mday--;
    components.tm_yday--;
    if (components.tm_mday < 1) {
      components.tm_mday = daysInMonths[ISLEAP(components.tm_year)][components.tm_mon - 1] - 1;
      if (components.tm_yday < 1) {
        components.tm_yday = DAYS_IN_YEAR(components.tm_year);
      }
      decMonths();
    } else {
      timestampInSeconds = fromComponents(&components);
      fromComponents(&components);
    }
  }

  void incMonths() {
    components.tm_mon++;
    if (components.tm_mon >= MONTHS_PER_YEAR) {
      components.tm_mon = 0;
      incYears();
    } else {
      timestampInSeconds = fromComponents(&components);
      toComponents(&components);
    }
  }

  void decMonths() {
    components.tm_mon--;
    if (components.tm_mon < 0) {
      components.tm_mon = MONTHS_PER_YEAR - 1;
      decYears();
    } else {
      timestampInSeconds = fromComponents(&components);
      toComponents(&components);
    }
  }

  void incYears() {
    components.tm_year++;
    timestampInSeconds = fromComponents(&components);
    toComponents(&components);
  }

  void decYears() {
    components.tm_year--;
    timestampInSeconds = fromComponents(&components);
    toComponents(&components);
  }

private:

  // NOTE: This function is basically the glibc POSIX __offtime function, but rewritten for readability.
  //       https://github.com/lattera/glibc/blob/master/time/offtime.c
  bool toComponents(struct tm *tmBuffer) {

    if (tmBuffer == NULL) {
      errno = ENOBUFS;
      return false;
    }

    time_t days = timestampInSeconds / SECS_PER_DAY;
    time_t remainder = (timestampInSeconds % SECS_PER_DAY) + secondsOffsetFromUTC;

    while (remainder < 0) {
      remainder += SECS_PER_DAY;
      days--;
    }
    while (remainder > SECS_PER_DAY) {
      remainder -= SECS_PER_DAY;
      days++;
    }

    tmBuffer->tm_hour = remainder / SECS_PER_HOUR;

    remainder %= SECS_PER_HOUR;
    tmBuffer->tm_min = remainder / SECS_PER_MINUTE;
    tmBuffer->tm_sec = remainder % SECS_PER_MINUTE;

    // 1-1-2001 was a monday.
    tmBuffer->tm_wday = (TIME_BASE_WDAY + days) % DAYS_PER_WEEK;
    if (tmBuffer->tm_wday < 0) {
      tmBuffer->tm_wday += DAYS_PER_WEEK;
    }

    time_t year = TIME_BASE_YEAR;

    while (days < 0 || days >= DAYS_IN_YEAR(year)) {
      time_t correctedYear = year + days / 365 - (days % 365 < 0);
      days -= ((correctedYear - year) * 365
               + LEAPS_THRU_END_OF(correctedYear - 1)
               - LEAPS_THRU_END_OF(year - 1));
      year = correctedYear;
    }

    tmBuffer->tm_year = year - TM_BASE_YEAR;
    if (tmBuffer->tm_year != year - TM_BASE_YEAR) {
      errno = EOVERFLOW;
      return false;
    }

    tmBuffer->tm_yday = days + 1;

    const unsigned short int *monthStartDays = daysBeforeMonths[ISLEAP(year) ? 1 : 0];
    time_t month;
    for (month = 11; days < (long int)monthStartDays[month]; --month) {
      continue;
    }
    days -= monthStartDays[month];
    tmBuffer->tm_mon = month;

    tmBuffer->tm_mday = days + 1;

    return true;
  }

  // Note: this function is basically from Newlib.
  //       https://github.com/bminor/newlib/blob/master/newlib/libc/time/mktime.c
  time_t fromComponents(struct tm *tmBuffer) {
    __tzinfo_type *tz = __gettzinfo();

    time_t timestamp = 0;
    long days = 0;
    int year;
    int isdst = 0;
    __tzinfo_type *timezoneInfo = __gettzinfo();

    /* validate structure */
    validateTMBuffer(tmBuffer);

    /* compute hours, minutes, seconds */
    timestamp += tmBuffer->tm_sec + (tmBuffer->tm_min * SECS_PER_MINUTE) + (tmBuffer->tm_hour * SECS_PER_HOUR);

    /* compute days in year */
    days += tmBuffer->tm_mday - 1;
    days += daysBeforeMonths[ISLEAP(tmBuffer->tm_year) ? 1 : 0][tmBuffer->tm_mon];

    tmBuffer->tm_yday = days;

    if (tmBuffer->tm_year > 10000 || tmBuffer->tm_year < -10000) {
      return (time_t)-1;
    }

    /* Compute days in other years than epoch year 1-1-2001 (tm_year == 101) */
    if ((year = tmBuffer->tm_year) > TIME_BASE_YEAR_TM) {
      for (year = TIME_BASE_YEAR_TM; year < tmBuffer->tm_year; year++) {
        days += DAYS_IN_YEAR(year);
      }
    } else if (year < TIME_BASE_YEAR_TM) {
      for (year = 100; year > tmBuffer->tm_year; year--) {
        days -= DAYS_IN_YEAR(year);
      }
      days -= DAYS_IN_YEAR(year);
    }

    timestamp += (time_t)days * SECS_PER_DAY;

    if (isDaylightSaving) {

      int tm_isdst;
      int year = tmBuffer->tm_year + TM_BASE_YEAR;

      /* Convert user positive into 1 */
      tm_isdst = tmBuffer->tm_isdst > 0 ? 1 : tmBuffer->tm_isdst;
      isdst = tm_isdst;

      if (year == timezoneInfo->__tzyear || __tzcalc_limits(year)) {

        time_t startdst_dst = tz->__tzrule[0].change - (time_t)tz->__tzrule[1].offset;
        time_t startstd_dst = tz->__tzrule[1].change - (time_t)tz->__tzrule[1].offset;
        time_t startstd_std = tz->__tzrule[1].change - (time_t)tz->__tzrule[0].offset;

        if (timestamp >= startstd_std && timestamp < startstd_dst)
          ; /* we let user decide or leave as -1 */
        else {
          isdst = (tz->__tznorth
                     ? (timestamp >= startdst_dst && timestamp < startstd_std)
                     : (timestamp >= startdst_dst || timestamp < startstd_std));
          if (tm_isdst >= 0 && (isdst ^ tm_isdst) == 1) {
            int diff = (int)(tz->__tzrule[0].offset - tz->__tzrule[1].offset);
            if (!isdst) {
              diff = -diff;
            }
            tmBuffer->tm_sec += diff;
            timestamp += diff; /* we also need to correct our current time calculation */

            int mday = tmBuffer->tm_mday;
            validateTMBuffer(tmBuffer);
            mday = tmBuffer->tm_mday - mday;

            if (mday) {
              if (mday > 1) {
                mday = -1;
              } else if (mday < -1) {
                mday = 1;
              }

              days += mday;

              if ((tmBuffer->tm_yday += mday) < 0) {
                --year;
                tmBuffer->tm_yday = DAYS_IN_YEAR(year) - 1;
              } else {
                mday = DAYS_IN_YEAR(year);
                if (tmBuffer->tm_yday > (mday - 1)) {
                  tmBuffer->tm_yday -= mday;
                }
              }
            }
          }
        }
      }
    }

    if (isdst == 1) {
      timestamp += (time_t)timezoneInfo->__tzrule[1].offset;
    } else {
      timestamp += (time_t)timezoneInfo->__tzrule[0].offset;
    }

    tmBuffer->tm_isdst = isdst;

    if ((tmBuffer->tm_wday = (days + 4) % 7) < 0) {
      tmBuffer->tm_wday += 7;
    }

    return timestamp;
  }

  static void validateTMBuffer(struct tm *tmBuffer) {

    div_t result;
    int days_in_feb = 28;

    /* calculate time & date to account for out of range values */
    if (tmBuffer->tm_sec < 0 || tmBuffer->tm_sec > 59) {
      result = div(tmBuffer->tm_sec, 60);
      tmBuffer->tm_min += result.quot;
      if ((tmBuffer->tm_sec = result.rem) < 0) {
        tmBuffer->tm_sec += 60;
        --tmBuffer->tm_min;
      }
    }

    if (tmBuffer->tm_min < 0 || tmBuffer->tm_min > 59) {
      result = div(tmBuffer->tm_min, 60);
      tmBuffer->tm_hour += result.quot;
      if ((tmBuffer->tm_min = result.rem) < 0) {
        tmBuffer->tm_min += 60;
        --tmBuffer->tm_hour;
      }
    }

    if (tmBuffer->tm_hour < 0 || tmBuffer->tm_hour > 23) {
      result = div(tmBuffer->tm_hour, 24);
      tmBuffer->tm_mday += result.quot;
      if ((tmBuffer->tm_hour = result.rem) < 0) {
        tmBuffer->tm_hour += 24;
        --tmBuffer->tm_mday;
      }
    }

    if (tmBuffer->tm_mon < 0 || tmBuffer->tm_mon > 11) {
      result = div(tmBuffer->tm_mon, 12);
      tmBuffer->tm_year += result.quot;
      if ((tmBuffer->tm_mon = result.rem) < 0) {
        tmBuffer->tm_mon += 12;
        --tmBuffer->tm_year;
      }
    }

    if (ISLEAP(tmBuffer->tm_year)) {
      days_in_feb = 29;
    }

    if (tmBuffer->tm_mday <= 0) {
      while (tmBuffer->tm_mday <= 0) {
        if (--tmBuffer->tm_mon == -1) {
          tmBuffer->tm_year--;
          tmBuffer->tm_mon = 11;
          days_in_feb = (ISLEAP(tmBuffer->tm_year) ? 29 : 28);
        }
        tmBuffer->tm_mday += daysInMonths[ISLEAP(tmBuffer->tm_year) ? 1 : 0][tmBuffer->tm_mon];
      }
    } else {
      while (tmBuffer->tm_mday > daysInMonths[ISLEAP(tmBuffer->tm_year) ? 1 : 0][tmBuffer->tm_mon]) {
        tmBuffer->tm_mday -= daysInMonths[ISLEAP(tmBuffer->tm_year) ? 1 : 0][tmBuffer->tm_mon];
        if (++tmBuffer->tm_mon == 12) {
          tmBuffer->tm_year++;
          tmBuffer->tm_mon = 0;
          days_in_feb = (ISLEAP(tmBuffer->tm_year) ? 29 : 28);
        }
      }
    }
  }

  int __tzcalc_limits(int year) {
    int days;
    int year_days;
    int years;
    int i;
    int j;
    __tzinfo_type *tz = __gettzinfo();

    if (year < TIME_BASE_YEAR)
      return 0;

    tz->__tzyear = year;

    years = (year - TIME_BASE_YEAR);

    year_days = years * 365 + (years - 1 + TIME_BASE_YEARS_SINCE_LEAP) / 4 - (years - 1 + TIME_BASE_YEARS_SINCE_CENTURY) / 100 + (years - 1 + TIME_BASE_YEARS_SINCE_LEAP_CENTURY) / 400;

    for (i = 0; i < 2; ++i) {
      if (tz->__tzrule[i].ch == 'J') {
        days = year_days + tz->__tzrule[i].d + (ISLEAP(year) && tz->__tzrule[i].d >= 60);
        --days;
      } else if (tz->__tzrule[i].ch == 'D') {
        days = year_days + tz->__tzrule[i].d;
      } else {
        int m_day, m_wday, wday_diff;
        const short unsigned int *ip = daysInMonths[ISLEAP(year) ? 1 : 0];

        days = year_days;

        for (j = 1; j < tz->__tzrule[i].m; ++j) {
          days += ip[j - 1];
        }

        m_wday = (TIME_BASE_WDAY + days) % DAYS_PER_WEEK;

        wday_diff = tz->__tzrule[i].d - m_wday;
        if (wday_diff < 0) {
          wday_diff += DAYS_PER_WEEK;
        }
        m_day = (tz->__tzrule[i].n - 1) * DAYS_PER_WEEK + wday_diff;

        while (m_day >= ip[j - 1]) {
          m_day -= DAYS_PER_WEEK;
        }

        days += m_day;
      }

      tz->__tzrule[i].change = days * SECS_PER_DAY + tz->__tzrule[i].s + tz->__tzrule[i].offset;
    }

    tz->__tznorth = (tz->__tzrule[0].change < tz->__tzrule[1].change);

    return 1;
  }
};
