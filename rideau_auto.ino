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

#include "utility.h"

#include "configurationFile.h"

// Declaration of all module needed
HX711 scale;

WiFiServer server(80); // server socket
WiFiClient client = server.available();

// Allow to reach the Ardurino by http://rideau.local
WiFiUDP mdnsUDP;
MDNS mdns(mdnsUDP);

SunSet sun;

// Prevents the rideau from opening or closing indefinitely in case of an issue.
class AsyncTimer timer;

class ElectricCurtainAndSheerController controller(WIFI_SSID,
                                                   WIFI_PASSWORD);
class Config config;

int ropeTentionCur = 0;
int ropeTentionPrev = 0;

void setup()
{
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  controller.setup();

  // while (!Serial);
  Serial.println("Serial ready");

  wifiEnable();
  wifiConnect();
  server.begin();
  wifiPrintStatus();

  // Initialize the mDNS library.
  // You can now reach via the host name "rideau.local"
  // Always call this before any other method!
  mdns.begin(WiFi.localIP(), MDNS_NAME);
  Serial.println("Server mdns ready");
  Serial.print("http://");
  Serial.print(MDNS_NAME);
  Serial.println(".local");

  /* Get our time sync started */
  /* Set our position and a default timezone value */
  sun.setPosition(config.latitude, config.longitude, config.daylightSavingOffset);
  sun.setTZOffset(config.daylightSavingOffset);

  // Retreive the time from internet
  // https://playground.arduino.cc/Code/Time/
  Serial.println("Request NTP");
  setSyncProvider(getNtpTime);
  setSyncInterval(60 * 60);
  {
    char timeString[255];
    sprintf(timeString, "Date: %d/%d/%d", day(), month(), year());
    Serial.println(timeString);

    sprintf(timeString, "Heure: %d:%02d:%02d", hour(), minute(), second());
    Serial.println(timeString);
  }
}

void loop()
{
  // Read the rope tention
  if (scale.is_ready())
  {
    ropeTentionPrev = ropeTentionCur;
    ropeTentionCur = int(scale.get_value() / 500);
//    Serial.println(ropeTentionCur);
  }

  client = server.available();
  if (client)
  {
    Serial.println("Client connected");
    htmlRun(config, getTimeStr, getSunrise, getSunset);
  }

  // This actually runs the Bonjour module.
  // YOU HAVE TO CALL THIS PERIODICALLY, OR NOTHING WILL WORK!
  // Preferably, call it once per loop().
  mdns.run();

  controller.run();

  timer.handle(now());
}

void wifiPrintStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Display received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void wifiEnable()
{
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Error: Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0")
  {
    Serial.println("Please upgrade the firmware");
  }
}

void wifiConnect()
{
  // attempt to connect to Wifi network:
  while (controller.m_webStatus != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(controller.m_wifiSsid);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    controller.m_webStatus = WiFi.begin(controller.m_wifiSsid, controller.m_wifiPassword);

    // wait 10 seconds for connection:
    delay(10000);
  }
}

///////////////////////////////////////////////////////////////
// Rideau action callback  ////////////////////////////////////
// All action function to perform operations on the rideau
void closeRideauManuel(int idx)
{
  controller.motorRideau[idx].action(Motor::REVERSE, config.manualSpeed);
}

void openRideauManuel(int idx)
{
  controller.motorRideau[idx].action(Motor::FOWARD, config.manualSpeed);
}

void closeRideauAuto(int idx)
{
  timer.cancel(controller.closeTimerId[idx]);

  if (config.rideau[idx].isCloseAtSunset)
    controller.closeTimerId[idx] = timer.setTimeout([idx]()
                                                    {closeRideauManuel(idx); closeRideauAuto(idx); }, int2time_t(static_cast<int>(sun.calcSunset())));

  if (config.rideau[idx].isCloseAtTime)
    controller.closeTimerId[idx] = timer.setTimeout([idx]()
                                                    {closeRideauManuel(idx); closeRideauAuto(idx); }, int2time_t(config.rideau[idx].closeAtTime));
}

void openRideauAuto(int idx)
{
  timer.cancel(controller.openTimerId[idx]);

  if (config.rideau[idx].isOpenAtSunrise)
    controller.openTimerId[idx] = timer.setTimeout([idx]()
                                                   {openRideauManuel(idx); openRideauAuto(idx); }, int2time_t(static_cast<int>(sun.calcSunrise())));

  if (config.rideau[idx].isOpenAtTime)
    controller.openTimerId[idx] = timer.setTimeout([idx]()
                                                   {openRideauManuel(idx); openRideauAuto(idx); }, int2time_t(config.rideau[idx].openAtTime));
}
