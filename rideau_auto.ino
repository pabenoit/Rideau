#include <Arduino.h>

#include <WiFiNINA.h>
#include <ArduinoMDNS.h>
#include <WiFiUdp.h>

#include <TimeLib.h>
#include <sunset.h>

#include "Motor.h"
#include "ntp.h"
#include "html.h"
#include "configData.h"
#include "AsyncTimer.h"
#include "HX711.h"

HX711 scale;
uint8_t dataPin = 8;  // Used by HX711 to mesure the tention
uint8_t clockPin = 9; // Used by HX711 to mesure the tention

class Data data;
class Config config;



WiFiServer server(80);  // server socket
WiFiClient client = server.available();
WiFiUDP mdnsUDP;
MDNS mdns(mdnsUDP);
SunSet sun;

class AsyncTimer timer;





// utility function for digital clock display: prints leading 0
String twoDigits(int digits) {
  if (digits < 10) {
    String i = '0' + String(digits);
    return i;
  } else {
    return String(digits);
  }
}

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

  // Date should be in future, never in past
  do {
    tmTime.tm_mday = today;
    out = mktime(&tmTime);
    today++;
  } while (out < now());

  return (out);
}

void closeRideauManuel(int idx) {
  data.motorRideau[idx].action(Motor::REVERSE, config.manualSpeed);
}

void openRideauManuel(int idx) {
  data.motorRideau[idx].action(Motor::FOWARD, config.manualSpeed);
}

void closeRideauAuto(int idx) {
  timer.cancel(data.closeTimerId[idx]);

  if (config.rideau[idx].isCloseAtSunset)
    data.closeTimerId[idx] = timer.setTimeout([idx]() {closeRideauManuel(idx); closeRideauAuto(idx);}, int2time_t(static_cast<int>(sun.calcSunset())));

  if (config.rideau[idx].isCloseAtTime)
    data.closeTimerId[idx] = timer.setTimeout([idx]() {closeRideauManuel(idx); closeRideauAuto(idx);}, int2time_t(config.rideau[idx].closeAtTime));
}

void openRideauAuto(int idx) {
  timer.cancel(data.openTimerId[idx]);

  if (config.rideau[idx].isOpenAtSunrise)
    data.openTimerId[idx] = timer.setTimeout([idx]() {openRideauManuel(idx); openRideauAuto(idx);}, int2time_t(static_cast<int>(sun.calcSunrise())));

  if (config.rideau[idx].isOpenAtTime)
    data.openTimerId[idx] = timer.setTimeout([idx]() {openRideauManuel(idx); openRideauAuto(idx);}, int2time_t(config.rideau[idx].openAtTime));
}


char* getTimeStr() {
  static char timeString[255];
  sprintf(timeString, "%d/%d/%d  %d:%02d:%02d", day(), month(), year(), hour(), minute(), second());
  return timeString;
}

int getSunrise() {
  sun.setCurrentDate(year(), month(), day());
  return (static_cast<int>(sun.calcSunrise()));
}

int getSunset() {
  sun.setCurrentDate(year(), month(), day());
  return (static_cast<int>(sun.calcSunset()));
}



void setup() {
  Serial.begin(9600);
  scale.begin(dataPin, clockPin);

  data.motorRideau[0].setup();

  //while (!Serial);
  //Serial.println("Serial ready");

  wifiEnable();
  wifiConnect();
  server.begin();
  wifiPrintStatus();

  // Initialize the mDNS library. You can now reach via the host name "rideau.local"
  // Always call this before any other method!
  mdns.begin(WiFi.localIP(), "rideau");
  Serial.println("Server mdns ready");


  /* Get our time sync started */
  /* Set our position and a default timezone value */
  sun.setPosition(config.latitude, config.longitude, config.daylightSavingOffset);
  sun.setTZOffset(config.daylightSavingOffset);

  // https://playground.arduino.cc/Code/Time/
  Serial.println("Request NTP");
  setSyncProvider(getNtpTime);
  setSyncInterval(60 * 60);

  char timeString[255];
  sprintf(timeString, "Date: %d/%d/%d", day(), month(), year());
  Serial.println(timeString);

  sprintf(timeString, "Heure: %d:%02d:%02d", hour(), minute(), second());
  Serial.println(timeString);
}

void loop() {

  // Read the rope tention
  Serial.println(scale.read());


  client = server.available();
  if (client) {
    htmlRun(config, data, getTimeStr, getSunrise, getSunset);
  }

  // This actually runs the Bonjour module. YOU HAVE TO CALL THIS PERIODICALLY,
  // OR NOTHING WILL WORK! Preferably, call it once per loop().
  mdns.run();

  data.motorRideau[0].run();
  data.motorRideau[1].run();

  static time_t prevTime;

  timer.handle(now());
}

void wifiPrintStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void wifiEnable() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
}

void wifiConnect() {
  // attempt to connect to Wifi network:
  while (data.webStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(config.ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    data.webStatus = WiFi.begin(config.ssid, config.password);

    // wait 10 seconds for connection:
    delay(10000);
  }
}
