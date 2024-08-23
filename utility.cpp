#include "utility.h"

/**
 * @brief Converts a given time in minutes since midnight to a time_t value.
 * 
 * This function creates a time_t value representing the time when the sun will rise or set.
 * It uses the current day, month, and year, and ensures the resulting time_t value is in the future, not in the past.
 * 
 * @param in The time in minutes since midnight.
 * @return The corresponding time_t value.
 */
time_t int2time_t(int in) {
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

  // Date should be in the future, never in the past
  do {
    tmTime.tm_mday = today;
    out = mktime(&tmTime);
    today++;
  } while (out < now());

  return out;
}


/**
 * @brief Returns the current date and time as a string.
 * 
 * This function formats the current date and time into a string in the format "day/month/year hour:minute:second".
 * 
 * @return A pointer to the formatted date and time string.
 */
char *getTimeStr() {
  static char timeStr[20];  // "dd/mm/yyyy hh:mm:ss" is 19 characters + null terminator
  sprintf(timeStr, "%02d/%02d/%04d %02d:%02d:%02d", day(), month(), year(), hour(), minute(), second());
  return timeStr;
}

/**
 * @brief Returns the sunrise time in minutes since midnight.
 * 
 * This function calculates the sunrise time for the current date and returns it in minutes since midnight.
 * 
 * @return The sunrise time in minutes since midnight.
 */
int getSunrise() {
  sun.setCurrentDate(year(), month(), day());
  return static_cast<int>(sun.calcSunrise());
}

/**
 * @brief Returns the sunset time in minutes since midnight.
 * 
 * This function calculates the sunset time for the current date and returns it in minutes since midnight.
 * 
 * @return The sunset time in minutes since midnight.
 */
int getSunset() {
  sun.setCurrentDate(year(), month(), day());
  return static_cast<int>(sun.calcSunset());
}