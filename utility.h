#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <time.h>
#include <TimeLib.h>

#include <sunset.h>

time_t int2time_t(int in);

char *getTimeStr();

int getSunrise();
int getSunset();

extern SunSet sun;

#endif