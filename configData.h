
#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include "Motor.h"

class ConfigRideau {
public:
  ConfigRideau()
    : isEnabled(false),
      isOpenAtSunrise(true),
      isOpenAtTime(false),
      openAtTime(420), // Default to 7:00 AM (7*60 minutes)
      isCloseAtSunset(true),
      isCloseAtTime(false),
      closeAtTime(1320), // Default to 10:00 PM (22*60 minutes)
      openingSpeed (120),
      closingSpeed (120)
      {}

  bool operator==(const ConfigRideau& other) const {
    return ((this->isEnabled == other.isEnabled) && 
            (this->isOpenAtSunrise == other.isOpenAtSunrise) && 
            (this->isCloseAtSunset == other.isCloseAtSunset) && 
            (this->isOpenAtTime == other.isOpenAtTime) && 
            (this->isCloseAtTime == other.isCloseAtTime) && 
            (this->openAtTime == other.openAtTime) && 
            (this->closeAtTime == other.closeAtTime) && 
            (this->openingSpeed  == other.openingSpeed ) && 
            (this->closingSpeed  == other.closingSpeed ));
  };

  bool operator!=(const ConfigRideau& c) const {
    return (!(*this == c));
  };

  bool isEnabled;

  bool isOpenAtSunrise;
  bool isOpenAtTime;
  int openAtTime;

  bool isCloseAtSunset;
  bool isCloseAtTime;
  int closeAtTime;

  int openingSpeed ;
  int closingSpeed ;
};

class Config {
public:
  Config()
    : manualSpeed(255),
      ssid("RFBP"),
      password("CE1736A5"),
      daylightSavingOffset(-4),
      latitude(45.508888),
      longitude(-73.561668) 
      {}

  bool operator==(const Config& other) const {
    return ((this->rideau[0] == other.rideau[0]) && 
            (this->rideau[1] == other.rideau[1]) &&
            (this->manualSpeed == other.manualSpeed));
  };

  bool operator!=(const Config& other) const {
    return !(*this == other);
  };

  class ConfigRideau rideau[2];

  int manualSpeed;

  char ssid[48];      // network SSID (name)
  char password[48];  // network password

  // Geographical and timezone information for Montreal
  int daylightSavingOffset;
  double latitude;
  double longitude;
};

class Data {
public:
  Data()
    : openTimerId{0, 0}, // Timers not configured use 0
      closeTimerId{0, 0}, // Timers not configured use 0
      motorRideau({ Motor(0 /* orange */, 1 /* jaune */, 2 /* vert */ , 25000 /* Rideau tention threshold */), 
                    Motor(3 /* bleu */  , 4 /* mauve */, 5 /* blanc */, 25000 /* Rideau tention threshold */) }), 
      webStatus(WL_IDLE_STATUS){};

  unsigned short openTimerId[2];   // timer not configure use 0
  unsigned short closeTimerId[2];  // timer not configure use 0
  Motor motorRideau[2];


  // WEB Server
  int webStatus;  // connection status
};

#endif