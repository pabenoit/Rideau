#include "utility.h"

// Create a time_t when sun will raise or set
// Converts a given time in minutes since midnight to a time_t value
// Uses the current day, month, and year as sun.calcSunset() returns the number of seconds since 00:00
// Ensures the resulting time_t value is in the future, not in the past
time_t int2time_t(int in)
{
  int today = day();
  time_t out;

  struct tm tmTime;
  tmTime.tm_sec = 0;
  tmTime.tm_min = in % 60;
  tmTime.tm_hour = in / 60;
  tmTime.tm_mday = today;
  tmTime.tm_mon = month() - 1;
  tmTime.tm_year = year() - 1900;
  tmTime.tm_isdst = false;

  // Date should be in future, never in past
  do
  {
    tmTime.tm_mday = today;
    out = mktime(&tmTime);
    today++;
  } while (out < now());

  return (out);
}


// Return the Date and time in string format
char *getTimeStr()
{
  static char timeString[255];
  sprintf(timeString, "%d/%d/%d  %d:%02d:%02d", day(), month(), year(), hour(), minute(), second());
  return timeString;
}

// Return the sunrise time
int getSunrise()
{
  sun.setCurrentDate(year(), month(), day());
  return (static_cast<int>(sun.calcSunrise()));
}

// return the sunSet time
int getSunset()
{
  sun.setCurrentDate(year(), month(), day());
  return (static_cast<int>(sun.calcSunset()));
}