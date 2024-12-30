#include <stdio.h>
#include <string.h>

#include <TimeLib.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include <ESPmDNS.h>
#include <EEPROM.h>
#include <math.h>
//#include <sunset.h>

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
int getThreshold();
int readCurrent();

extern std::tuple<bool, int, int> handleHttpRequest();
extern time_t getNtpTime();

INA219_WE ina219 = INA219_WE();

Motor motors[] = {Motor(MOTOR1_PIN1, MOTOR1_PIN2, MOTOR1_SPEED, getThreshold, readCurrent),
                  Motor(MOTOR2_PIN1, MOTOR2_PIN2, MOTOR2_SPEED, getThreshold, readCurrent)};
int status = WL_IDLE_STATUS;
WiFiServer server(80);
WiFiClient myWifiClient;

Configuration systemCfg;
int currentDay = -1;

int sunSetOfTheday = 7 * 60;    // Default value in case can't get the value from the internet
int sunRiseOfTheday = 18 * 60;  // Default value in case can't get the value from the internet

int getThreshold() { return systemCfg.cfg.thresholdMaxCurrent; }

int readCurrent()
{
  float current_mA = abs(ina219.getCurrent_mA());
  return static_cast<int>(current_mA);
}

void setup()
{
  // Start serial communication for debugging
  Serial.begin(115200);

  //Increment boot count
  systemCfg.IncrementBootCount();

  Serial.println("Starting...");

  systemCfg.loadFromFLASH();
  Serial.println(systemCfg.GetAllInfo().c_str());

  Wire.begin();
  if (!ina219.init())
  {
    Serial.println("INA219 not connected!");
    while (1);
  }
  
  ////   ina219.setPGain(PG_80);
  ina219.setBusRange(BRNG_16);
  ina219.setADCMode(SAMPLE_MODE_64);  // choose mode and uncomment for change of default



  motors[0].setup();
  motors[1].setup();

  // Attempt to connect to WiFi network
  WiFi.begin(systemCfg.cfg.wifi.ssid.c_str(), systemCfg.cfg.wifi.password.c_str());

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
      delay(200);
      Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
    
  // Start the server
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
    if (!MDNS.begin(systemCfg.cfg.wifi.mdns.c_str())) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
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
