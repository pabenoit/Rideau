#include <stdio.h>
#include <string.h>

#include <Arduino.h>
#include <ArduinoMDNS.h>
#include <TimeLib.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <math.h>
//#include <sunset.h>
#include <INA219_WE.h>
#include <Wire.h>

#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>

#include "Configuration.h"
#include "motorController.h"
#include "utility.h"

// Hardware Connections
// -------------------
// Motor 1 (Left) connection
// Pin 2 - wire yellow - motor control
// Pin 3 - wire orange  - motor control
#define MOTOR1_PIN1 2
#define MOTOR1_PIN2 3

// Motor 2 (Right) connection
// Pin 4 - wire green  - motor control
// Pin 5 - wire blue   - motor control
#define MOTOR2_PIN1 4
#define MOTOR2_PIN2 5

void setup();
void loop();
void applyNoAction(int deviceId, int action);
void applyMotorAction(int deviceId, int action);
int getThreshold();
int readCurrent();

extern std::tuple<bool, int, int> handleHttpRequest();
extern time_t getNtpTime();

INA219_WE ina219 = INA219_WE();

Motor motors[] = {Motor(MOTOR1_PIN1, MOTOR1_PIN2, getThreshold, readCurrent),
                  Motor(MOTOR2_PIN1, MOTOR2_PIN2, getThreshold, readCurrent)};

ArduinoLEDMatrix matrix;
int status = WL_IDLE_STATUS;
WiFiServer server(80);
WiFiClient myWifiClient;

// Allow to reach the Arduino by http://rideau.local
WiFiUDP mdnsUDP;
MDNS mdns(mdnsUDP);

Configuration systemCfg;
int currentDay = -1;

int sunSetOfTheday = 7 * 60;    // Default value in case can't get the value from the internet
int sunRiseOfTheday = 18 * 60;  // Default value in case can't get the value from the internet

int getThreshold() { return systemCfg.cfg.thresholdMaxCurrent; }

int readCurrent()
{
  float current_mA = ina219.getCurrent_mA();
  return static_cast<int>(current_mA);

#if 0
  // TODO add current mesurement static_cast<int>(scale.read_average(2) / 500);
  static int tension = 1;
  tension = (tension + 1) % 120;
  return tension;
#endif
}

void setup()
{
  // Start serial communication for debugging
  Serial.begin(115200);
  Serial.println("Starting...");

  systemCfg.loadFromEEPROM();
  Serial.println(systemCfg.GetAllInfo().c_str());

#if 0
  systemCfg.cfg.wifi.ssid = "RFBP";
  systemCfg.cfg.wifi.password = "CE1736A5";
  systemCfg.cfg.wifi.mdns = "srideau";

  if (systemCfg.cfg.devices.size() == 0)
  {
    Device newDevice1("Rideau");
    Automation newAutomation1(static_cast<int>(Automation::Action::Open),
                              static_cast<int>(Automation::Type::Sun),
                              "6:30",
                              0,
                              static_cast<int>(Automation::Status::Enable));

    Automation newAutomation2(static_cast<int>(Automation::Action::Close),
                              static_cast<int>(Automation::Type::SpecificTime),
                              "23:00",
                              0,
                              static_cast<int>(Automation::Status::Enable));
    newDevice1.automations.insert(std::make_pair(1, newAutomation1));
    newDevice1.automations.insert(std::make_pair(2, newAutomation2));
    systemCfg.cfg.devices.insert(std::make_pair(0, newDevice1));

    Device newDevice2("Voilage");
    Automation newAutomation3(static_cast<int>(Automation::Action::Open),
                              static_cast<int>(Automation::Type::Sun),
                              "8:00",
                              0,
                              static_cast<int>(Automation::Status::Enable));

    Automation newAutomation4(static_cast<int>(Automation::Action::Close),
                              static_cast<int>(Automation::Type::SpecificTime),
                              "11:00",
                              0,
                              static_cast<int>(Automation::Status::Enable));
    newDevice2.automations.insert(std::make_pair(1, newAutomation3));
    newDevice2.automations.insert(std::make_pair(2, newAutomation4));
    systemCfg.cfg.devices.insert(std::make_pair(1, newDevice2));

    systemCfg.saveToEEPROM();
  }
#endif
  Serial.println(systemCfg.GetAllInfo().c_str());

  Wire.begin();
  if (!ina219.init())
  {
    Serial.println("INA219 not connected!");
    while (1)
      ;
  }
  ////   ina219.setPGain(PG_80);
  ina219.setBusRange(BRNG_16);
  ina219.setADCMode(SAMPLE_MODE_128);  // choose mode and uncomment for change of default

  matrix.begin();
  matrix.loadSequence(LEDMATRIX_ANIMATION_LOAD_CLOCK);
  matrix.play(true);

  motors[0].setup();
  motors[1].setup();

  // Check for the WiFi module
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  matrix.loadSequence(LEDMATRIX_ANIMATION_WIFI_SEARCH);
  matrix.play(true);

  // Attempt to connect to WiFi network
  while (status != WL_CONNECTED)
  {
    Serial.println("Attempting to connect to SSID: ");
    status = WiFi.begin(systemCfg.cfg.wifi.ssid.c_str(), systemCfg.cfg.wifi.password.c_str());

    // Wait 1 seconds for connection
    delay(1000);
  }

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize the mDNS library.
  mdns.begin(WiFi.localIP(), systemCfg.cfg.wifi.mdns.c_str());
  // Need for iPhone support
  String serviceName = String(systemCfg.cfg.wifi.mdns.c_str()) + "._http";
  mdns.addServiceRecord(serviceName.c_str(), 80, MDNSServiceTCP);

  // Retreive the time from internet
  // https://playground.arduino.cc/Code/Time/
  Serial.println("Request NTP");
  setSyncProvider(getNtpTime);
  setSyncInterval(60 * 60);

  String timeString = "Date: " + String(day()) + "/" + String(month()) + "/" + String(year()) +
                      " Time: " + String(hour()) + ":" + String(minute()) + ":" + String(second());
  Serial.println(timeString);

  matrix.loadSequence(LEDMATRIX_ANIMATION_HEARTBEAT);
  matrix.play(true);
}

void loop()
{
  if (currentDay != day())
  {
    Serial.println("New day");

    int l_sunriseTime;
    int l_sunsetTime;
    if (getSunriseSunset(systemCfg.cfg.location.latitude,
                         systemCfg.cfg.location.longitude,
                         year(),
                         month(),
                         day(),
                         systemCfg.cfg.timeZoneOffset,
                         l_sunriseTime,
                         l_sunsetTime))
    {
      sunSetOfTheday = l_sunsetTime;
      sunRiseOfTheday = l_sunriseTime;

      String sunriseText = String(sunRiseOfTheday / 60) + ":" + String(sunRiseOfTheday % 60);
      Serial.print("Sunrise : ");
      Serial.println(sunriseText.c_str());
      String sunsetText = String(sunSetOfTheday / 60) + ":" + String(sunSetOfTheday % 60);
      Serial.print("Sunset  : ");
      Serial.println(sunsetText.c_str());
    }
    else
    {
      Serial.println("Error: Unable to get sunrise and sunset time");
      // Keep previous value as sunset and sunrise do n0t change a lot from day to day
    }

    systemCfg.resetListAction();

    // It is a startup, Clear all the automation flag up to now
    if (currentDay == -1)
      systemCfg.applyListAction(applyNoAction);

    currentDay = day();
  }

  systemCfg.applyListAction(applyMotorAction);

  auto [httpRc, httpDeviceId, httpAction] = handleHttpRequest();
  if (httpRc == true)
  {
    applyMotorAction(httpDeviceId, httpAction);
  }

  // This actually runs the Bonjour module.
  // you need to call this function regularly.
  // Preferably, call it once per loop().
  mdns.run();
}

void applyNoAction(int deviceId, int action)
{
  (void)deviceId;
  (void)action;
}

void applyMotorAction(int deviceId, int action)
{
  if ((deviceId != 0) || (deviceId != 1))
    deviceId = 0;

  switch (action)
  {
    case 0:
      Serial.print("Opening Motor ");
      Serial.println(deviceId);
      motors[deviceId].run(Motor::FORWARD);
      break;

    case 1:
      Serial.print("Closing Motor ");
      Serial.println(deviceId);
      motors[deviceId].run(Motor::REVERSE);
      break;

    default:
      break;
  }
}
