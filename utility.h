#ifndef UTILITY_H
#define UTILITY_H

#include <time.h>



time_t int2time_t(int in);
bool getSunriseSunset(double longitude, double latitude, int year, int month, int day, int timeZoneOffset, int &sunriseTime, int &sunsetTime);

#endif  // UTILITY_H