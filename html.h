#ifndef HTML_H
#define HTML_H

#include <string>
#include "configData.h"

#ifdef ARDUINO_UNOR4_WIFI
#include <WiFiS3.h>
#else
#include <WiFiNINA.h>
#endif


// Function declarations
bool htmlRun(Config *config, char *(*getTimeStr)(), int (*getSunrise)(), int (*getSunset)());

#endif // HTML_H