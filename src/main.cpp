#include <stdio.h>
#include <string.h>

#include <TimeLib.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>

#include <ESPmDNS.h>
#include <EEPROM.h>
#include <math.h>
// #include <sunset.h>

#include <INA219_WE.h>

#include <Wire.h>

#include "Configuration.h"
#include "motorController.h"
#include "utility.h"

#define EEPROM_SIZE 12

// Hardware Connections
// -------------------
// Motor 1 (Left) connection
// Pin 2 - wire yellow - motor control
// Pin 3 - wire orange  - motor control
#define MOTOR1_PIN1 26
#define MOTOR1_PIN2 27
#define MOTOR1_SPEED 14

// Motor 2 (Right) connection
// Pin 4 - wire green  - motor control
// Pin 5 - wire blue   - motor control
#define MOTOR2_PIN1 33
#define MOTOR2_PIN2 25
#define MOTOR2_SPEED 32

void setup();
void loop();
void applyNoAction(int deviceId, int action);
void applyMotorAction(int deviceId, int action);
int readCurrent();

int getMotorThresholdMax();
int getMotorThreshold();
int getMotorBlindTime();
int getMotorRunTimeLimit();
int getMotorSpeed();

extern time_t getNtpTime();

INA219_WE ina219 = INA219_WE();

Configuration systemCfg;

Motor motors[] = {Motor(MOTOR1_PIN1, MOTOR1_PIN2, MOTOR1_SPEED,
                        getMotorThresholdMax,
                        getMotorThreshold,
                        getMotorBlindTime,
                        getMotorRunTimeLimit,
                        getMotorSpeed,
                        readCurrent),

                  Motor(MOTOR2_PIN1, MOTOR2_PIN2, MOTOR2_SPEED,
                        getMotorThresholdMax,
                        getMotorThreshold,
                        getMotorBlindTime,
                        getMotorRunTimeLimit,
                        getMotorSpeed,
                        readCurrent)};

int status = WL_IDLE_STATUS;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int currentDay = -1;

int sunSetOfTheday = 7 * 60;   // Default value in case can't get the value from the internet
int sunRiseOfTheday = 18 * 60; // Default value in case can't get the value from the internet

int readCurrent()
{
  float current_mA = abs(ina219.getCurrent_mA());
  return static_cast<int>(current_mA);
}

int getMotorThresholdMax() { return systemCfg.cfg.motorThresholdMax; };
int getMotorThreshold() { return systemCfg.cfg.motorThreshold; };
int getMotorBlindTime() { return systemCfg.cfg.motorBlindTime; };
int getMotorRunTimeLimit() { return systemCfg.cfg.motorRunTimeLimit; };
int getMotorSpeed() { return systemCfg.cfg.motorSpeed; };

// WebServer
void setupWebServer();

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type,
               void * arg, uint8_t * data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.println("Client connected");
      } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("Client disconnected");
    } else if (type == WS_EVT_DATA) {
        Serial.println("Data received");
        // Handle incoming data
    }
}

void setup()
{
  // Start serial communication for debugging
  Serial.begin(115200);

  // Increment boot count
  systemCfg.IncrementBootCount();

  Serial.println("Starting...");

  // Init EEPROM
  // EEPROM.begin(EEPROM_SIZE);

#if 1
  systemCfg.cfg.wifi.ssid = "RFBP";
  systemCfg.cfg.wifi.password = "CE1736A5";
  systemCfg.cfg.wifi.mdns = "esp32";

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

    systemCfg.saveToFLASH();
  }

#else
  systemCfg.loadFromFLASH();
#endif
  Serial.println(systemCfg.GetAllInfo().c_str());

  Wire.begin();
  // if (!ina219.init())
  // {
  //   Serial.println("INA219 not connected!");
  //   while (1)
  //     ;
  // }
  // ////   ina219.setPGain(PG_80);
  // ina219.setBusRange(BRNG_16);
  // ina219.setADCMode(SAMPLE_MODE_64); // choose mode and uncomment for change of default

  motors[0].setup();
  motors[1].setup();

  // Attempt to connect to WiFi network
  WiFi.begin(systemCfg.cfg.wifi.ssid.c_str(), systemCfg.cfg.wifi.password.c_str());

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  setupWebServer();

  // Start the server
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.begin();
  Serial.println("Server started"); 

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize the mDNS library.
  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp32.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin(systemCfg.cfg.wifi.mdns.c_str()))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Start TCP (HTTP) server
  server.begin();
  Serial.println("TCP server started");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);

  // Retreive the time from internet
  // https://playground.arduino.cc/Code/Time/
  Serial.println("Request NTP");
  setSyncProvider(getNtpTime);
  setSyncInterval(60 * 60);

  String timeString = "Date: " + String(day()) + "/" + String(month()) + "/" + String(year()) +
                      " Time: " + String(hour()) + ":" + String(minute()) + ":" + String(second());
  Serial.println(timeString);
}

void loop()
{
//   ws.textAll( systemCfg.GetAllInfo().c_str());
//   delay(5000); // Delay for demonstration purposes


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

  // applyMotorAction(httpDeviceId, httpAction);

  // Handle mDNS queries
  // MDNS.update();
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
