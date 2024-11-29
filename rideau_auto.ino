#include <functional>
#include <Arduino.h>
#include <ArduinoMDNS.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <sunset.h>
#include <SolarCalculator.h>
#include <HX711.h>
#include <Wire.h>
#include <INA219_WE.h>


#ifdef ARDUINO_UNOR4_WIFI
#include <WiFiS3.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
ArduinoLEDMatrix matrix;
#else
#include <WiFiNINA.h>
#endif

#include "ntp.h"
#include "configData.h"
#include "configurationFile.h"
#include "curtainController.h"
#include "html.h"
#include "curtainMotor.h"
#include "utility.h"

INA219_WE ina219 = INA219_WE();


WiFiServer server(80);
WiFiClient client;
WiFiUDP mdnsUDP;
MDNS mdns(mdnsUDP);
SunSet sun;
HX711 scale;
Config config;
ControllerCurtain curtainCtrl(WIFI_SSID, WIFI_PASSWORD, &scale);

#define MAX_ACTION 20
#define ACTION_EMPTY -1
int timeOpenCurtain[MAX_ACTION];
int timeCloseCurtain[MAX_ACTION];
int currentDay = -1;

void setup()
{
    Serial.begin(57600);


  Wire.begin();
  if(!ina219.init()){
    Serial.println("INA219 not connected!");
    while(1);
  }
////   ina219.setPGain(PG_80);
   ina219.setBusRange(BRNG_16);
   ina219.setADCMode(SAMPLE_MODE_128); // choose mode and uncomment for change of default

#ifdef ARDUINO_UNOR4_WIFI
    matrix.begin();
    
    matrix.loadSequence(LEDMATRIX_ANIMATION_TETRIS_INTRO);
//    matrix.loadSequence(LEDMATRIX_ANIMATION_STARTUP);
    matrix.play(true);
#endif

    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  // Read configuration value from the EEPROM values.
    config.readCfg2Epprom();
    curtainCtrl.setup();

    while (!Serial);
   Serial.println("Serial ready");
    wifiEnable();
    listNetworks();
    wifiConnect();
    server.begin();
    wifiPrintStatus();

  // Initialize the mDNS library.
  // This allows the device to be reached via the host name "rideau.local".
  // Always call this before any other mDNS method!
    mdns.begin(WiFi.localIP(), MDNS_NAME);
    Serial.println("mDNS server ready");
    Serial.print("Access the server at: http://");
    Serial.print(MDNS_NAME);
    Serial.println(".local");
  // Need for iphone support
    mdns.addServiceRecord(MDNS_NAME "._http", 80, MDNSServiceTCP);

  // Start time synchronization
  // Set the geographical position (latitude and longitude) and the default timezone offset
    sun.setPosition(config.m_latitude, config.m_longitude, config.m_daylightSavingOffset);
    sun.setTZOffset(config.m_daylightSavingOffset);

  // Retreive the time from internet
  // https://playground.arduino.cc/Code/Time/
    Serial.println("Request NTP");
    setSyncProvider(getNtpTime);
    setSyncInterval(60 * 60);

    char timeString[255];
    sprintf(timeString, "Date: %d/%d/%d", day(), month(), year());
    Serial.println(timeString);
    sprintf(timeString, "Time: %d:%02d:%02d", hour(), minute(), second());
    Serial.println(timeString);

    updateListActionOfTheDay();
    clearAllEventUpToNow();
}

void updateListActionOfTheDay()
{
    currentDay = day();
    memset(timeOpenCurtain, ACTION_EMPTY, sizeof(timeOpenCurtain));
    memset(timeCloseCurtain, ACTION_EMPTY, sizeof(timeCloseCurtain));

    int openIdx = 0, closeIdx = 0;
    for (int curtainNb = 0; curtainNb < 2; curtainNb++)
    {
        auto curtainCfg = config.getCurtainCfg(curtainNb);
        if (curtainCfg->isEnabled)
        {
            if (curtainCfg->isOpenAtSunrise)
                timeOpenCurtain[openIdx++] = getSunrise();
            if (curtainCfg->isOpenAtTime)
                timeOpenCurtain[openIdx++] = curtainCfg->openAtTime;
            if (curtainCfg->isCloseAtSunset)
                timeCloseCurtain[closeIdx++] = getSunset();
            if (curtainCfg->isCloseAtTime)
                timeCloseCurtain[closeIdx++] = curtainCfg->closeAtTime;
        }
    }
}

void clearAllEventUpToNow()
{
    int currentTime = hour() * 60 + minute();
    for (int idx = 0; idx < MAX_ACTION; idx++)
    {
        if (timeOpenCurtain[idx] != ACTION_EMPTY && timeOpenCurtain[idx] < currentTime)
            timeOpenCurtain[idx] = ACTION_EMPTY;
        if (timeCloseCurtain[idx] != ACTION_EMPTY && timeCloseCurtain[idx] < currentTime)
            timeCloseCurtain[idx] = ACTION_EMPTY;
    }
}

void loop()
{
  if (currentDay != day())
    updateListActionOfTheDay();

#ifdef MESURE_COURANT
    static float average[20] = {0};
    static int index = 0;
    static float avg = 0;

    float value = analogRead(CURRENT_MEASUREMENT_PIN);
    avg -= average[index];
    avg += value;
    average[index++] = value;
    index %= 20;

    ropeTensionCur = abs((avg / 20) - 510);
    if (index == 0)
        Serial.println(ropeTensionCur);
#endif

    client = server.available();
    if (client)
    {
        Serial.println("Client connected");
        Config tmpConfig = config;

        if (htmlRun(&tmpConfig, getTimeStr, getSunrise, getSunset))
        {
            for (int idx = 0; idx < 2; idx++)
            {
                auto curtainCfg = tmpConfig.getCurtainCfg(idx);

                switch (curtainCfg->manualAction)
                {
                case CurtainCfg::OPEN:
                    curtainCtrl.getMotor(idx)->run(CurtainMotor::FORWARD);
                    curtainCfg->manualAction = CurtainCfg::NONE;
                    break;
                case CurtainCfg::CLOSE:
                    curtainCtrl.getMotor(idx)->run(CurtainMotor::REVERSE);
                    curtainCfg->manualAction = CurtainCfg::NONE;
                    break;
                default:
                    break;
                }
            }

            config = tmpConfig;
            config.writeCfg2Epprom();
            updateListActionOfTheDay();
            clearAllEventUpToNow();
        }

        int currentTime = hour() * 60 + minute();
        for (int idx = 0; idx < MAX_ACTION; idx++)
        {
            if (timeOpenCurtain[idx] != ACTION_EMPTY && timeOpenCurtain[idx] <= currentTime)
            {
                curtainCtrl.getMotor(idx)->run(CurtainMotor::FORWARD);
                timeOpenCurtain[idx] = ACTION_EMPTY;
            }
            if (timeCloseCurtain[idx] != ACTION_EMPTY && timeCloseCurtain[idx] <= currentTime)
            {
                curtainCtrl.getMotor(idx)->run(CurtainMotor::REVERSE);
                timeCloseCurtain[idx] = ACTION_EMPTY;
            }
        }
    }

  // This actually runs the Bonjour module.
  // YOU HAVE TO CALL THIS PERIODICALLY, OR NOTHING WILL WORK!
  // Preferably, call it once per loop().
    mdns.run();
}

void wifiPrintStatus()
{
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.print("To see this page in action, open a browser to http://");
    Serial.println(WiFi.localIP());
}

void wifiEnable()
{
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Error: Communication with WiFi module failed!");
        while (true)
            ;
    }

    if (WiFi.firmwareVersion() < "1.0.0")
    {
        Serial.println("Please upgrade the firmware");
    }
}

void listNetworks()
{
#if 0
  // scan for nearby networks:
    Serial.println("** Scan Networks test**");
    Serial.println("** Scan Networks test**");
    Serial.println("** Scan Networks test**");
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1)
    {
        Serial.println("Couldn't get a WiFi connection");
//        while (true)
            ;
    }

    Serial.print("number of available networks:");
    Serial.println(numSsid);

    for (int thisNet = 0; thisNet < numSsid; thisNet++)
    {
        Serial.print(thisNet);
        Serial.print(") ");
        Serial.print(WiFi.SSID(thisNet));
        Serial.print("\tSignal: ");
        Serial.print(WiFi.RSSI(thisNet));
        Serial.println(" dBm");
    }
    #endif
}

void wifiConnect()
{
    while (curtainCtrl.getWebStatus() != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(curtainCtrl.getWifiSsid());
        curtainCtrl.setWebStatus(WiFi.begin(curtainCtrl.getWifiSsid(), curtainCtrl.getWifiPassword()));
    // wait 10 seconds for connection:
        delay(10000);
    }

#ifdef ARDUINO_UNOR4_WIFI
    matrix.stroke(0, 255, 0);
    matrix.loadFrame(LEDMATRIX_EMOJI_HAPPY);
#endif
}

int getThreshold()
{
    return config.m_threshold;
}

void setThreshold(int value)
{
    config.m_threshold = value;
    config.writeCfg2Epprom();
}
