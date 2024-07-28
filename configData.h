#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include "Motor.h"
#include "configurationFile.h"

class ConfigRideau
{
public:
  ConfigRideau()
      : isEnabled(false),
        isOpenAtSunrise(true),
        isOpenAtTime(false),
        openAtTime(DEFAULT_OPEN_AT_TIME),
        isCloseAtSunset(true),
        isCloseAtTime(false),
        closeAtTime(DEFAULT_CLOSE_AT_TIME),
        openingSpeed(OPENING_SPEED),
        closingSpeed(CLOSING_SPEED)
  {
  }

  bool operator==(const ConfigRideau &other) const
  {
    return ((this->isEnabled == other.isEnabled) &&
            (this->isOpenAtSunrise == other.isOpenAtSunrise) &&
            (this->isCloseAtSunset == other.isCloseAtSunset) &&
            (this->isOpenAtTime == other.isOpenAtTime) &&
            (this->isCloseAtTime == other.isCloseAtTime) &&
            (this->openAtTime == other.openAtTime) &&
            (this->closeAtTime == other.closeAtTime) &&
            (this->openingSpeed == other.openingSpeed) &&
            (this->closingSpeed == other.closingSpeed));
  };

  bool operator!=(const ConfigRideau &c) const
  {
    return (!(*this == c));
  };

  bool isEnabled;

  bool isOpenAtSunrise;
  bool isOpenAtTime;
  int openAtTime;

  bool isCloseAtSunset;
  bool isCloseAtTime;
  int closeAtTime;

  int openingSpeed;
  int closingSpeed;
};

class Config
{
public:
  Config()
      : manualSpeed(MANUAL_SPEED),
        daylightSavingOffset(DAYLIGHT_SAVING_OFFSET),
        latitude(LATITUDE),
        longitude(LONGITUDE)
  {
  }

  bool operator==(const Config &other) const
  {
    return ((this->rideau[0] == other.rideau[0]) &&
            (this->rideau[1] == other.rideau[1]) &&
            (this->manualSpeed == other.manualSpeed));
  };

  bool operator!=(const Config &other) const
  {
    return !(*this == other);
  };

  class ConfigRideau rideau[2];

  int manualSpeed;

  // Geographical and timezone information
  int daylightSavingOffset;
  double latitude;
  double longitude;
};

class ElectricCurtainAndSheerController
{
public:
  ElectricCurtainAndSheerController(char *wifiSsid, char *wifiPassword)
      : openTimerId{0, 0},  // Timers not configured use 0
        closeTimerId{0, 0}, // Timers not configured use 0
        motorRideau({Motor(MOTOR_1_PIN_ENA,
                           MOTOR_1_PIN1,
                           MOTOR_1_PIN2,
                           MAX_TENTION_TRESHOLD),
                     Motor(MOTOR_2_PIN_ENA,
                           MOTOR_2_PIN1,
                           MOTOR_2_PIN2,
                           MAX_TENTION_TRESHOLD)}),
        m_webStatus(WL_IDLE_STATUS)
  {
    strncpy(m_wifiSsid, wifiSsid, sizeof(m_wifiSsid));
    strncpy(m_wifiPassword, wifiPassword, sizeof(m_wifiPassword));
  };

  void setup(void)
  {
    motorRideau[0].setup();
    motorRideau[1].setup();
  };

  void run(void)
  {
    motorRideau[0].run();
    motorRideau[1].run();
  }

  unsigned short openTimerId[2];  // timer not configure use 0
  unsigned short closeTimerId[2]; // timer not configure use 0
  Motor motorRideau[2];

  // WEB Server
  int m_webStatus; // connection status

  char m_wifiSsid[48];     // network SSID (name)
  char m_wifiPassword[48]; // network password
};

#endif