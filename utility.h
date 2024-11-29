#ifndef UTILITY_H
#define UTILITY_H

#include <time.h>
#include <sunset.h>

// Function prototypes
time_t int2time_t(int in);
char *getTimeStr();
int getSunrise();
int getSunset();

extern SunSet sun;

#endif  // UTILITY_H