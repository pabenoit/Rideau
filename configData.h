#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include <EEPROM.h>

#include "Motor.h"
#include "configurationFile.h"

class ConfigCurtain {
public:
  ConfigCurtain()
    : isEnabled(false),
      isOpenAtSunrise(true),
      isOpenAtTime(false),
      openAtTime(DEFAULT_OPEN_TIME),
      isCloseAtSunset(true),
      isCloseAtTime(false),
      closeAtTime(DEFAULT_CLOSE_TIME),
      openingSpeed(OPENING_SPEED),
      closingSpeed(CLOSING_SPEED) {
  }

  bool operator==(const ConfigCurtain &other) const {
    return ((this->isEnabled == other.isEnabled) && (this->isOpenAtSunrise == other.isOpenAtSunrise) && (this->isCloseAtSunset == other.isCloseAtSunset) && (this->isOpenAtTime == other.isOpenAtTime) && (this->isCloseAtTime == other.isCloseAtTime) && (this->openAtTime == other.openAtTime) && (this->closeAtTime == other.closeAtTime) && (this->openingSpeed == other.openingSpeed) && (this->closingSpeed == other.closingSpeed));
  };

  bool operator!=(const ConfigCurtain &c) const {
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

class Config {
public:
  Config()
    : m_manualSpeed(MANUAL_SPEED),
      m_daylightSavingOffset(DAYLIGHT_SAVING_OFFSET),
      m_latitude(LATITUDE),
      m_longitude(LONGITUDE) {
  }

  bool operator==(const Config &other) const {
    return ((this->curtain[0] == other.curtain[0]) && (this->curtain[1] == other.curtain[1]) && (this->m_manualSpeed == other.m_manualSpeed));
  };

  bool operator!=(const Config &other) const {
    return !(*this == other);
  };

  // Optimisation to save the EEPROM as it only allowed 1000000 write. 
  // Only write if valued as changes.
  template< typename T > void updateEeprom(int address, const T &sData) {
    T rData;
    EEPROM.get(address, rData);

    if (rData != sData)
      EEPROM.put(address, sData);
  };

  void writeCfg2Epprom(void) {
    int address = 0;
    int idx;

    int version = VERSION;
    updateEeprom(address, version);
    address += sizeof(version);

    updateEeprom(address, m_manualSpeed);
    address += sizeof(m_manualSpeed);

    Serial.print("write EEPRON m_manualSpeed:");
    Serial.println(m_manualSpeed);


    for (idx = 0; idx < 2; idx++) {
      updateEeprom(address, curtain[idx].isEnabled);
      address += sizeof(curtain[idx].isEnabled);

      updateEeprom(address, curtain[idx].isOpenAtSunrise);
      address += sizeof(curtain[idx].isOpenAtSunrise);

      updateEeprom(address, curtain[idx].isOpenAtTime);
      address += sizeof(curtain[idx].isOpenAtTime);

      updateEeprom(address, curtain[idx].openAtTime);
      address += sizeof(curtain[idx].openAtTime);

      updateEeprom(address, curtain[idx].isCloseAtSunset);
      address += sizeof(curtain[idx].isCloseAtSunset);

      updateEeprom(address, curtain[idx].isCloseAtTime);
      address += sizeof(curtain[idx].isCloseAtTime);

      updateEeprom(address, curtain[idx].closeAtTime);
      address += sizeof(curtain[idx].closeAtTime);

      updateEeprom(address, curtain[idx].openingSpeed);
      address += sizeof(curtain[idx].openingSpeed);

      updateEeprom(address, curtain[idx].closingSpeed);
      address += sizeof(curtain[idx].closingSpeed);
    }
  };

  void readCfg2Epprom(void) {
    int address = 0;
    int idx;

    int version = 0;
    EEPROM.get(address, version);
    if (version != VERSION) {
      Serial.println("WARNING: Data in EEPROM do not match the version.");
      // Data can not be read as format is unknow for this version
      return;
    }

    address += sizeof(version);

    EEPROM.get(address, m_manualSpeed);
    address += sizeof(m_manualSpeed);

    Serial.print("read EEPRON m_manualSpeed:");
    Serial.println(m_manualSpeed);

    for (idx = 0; idx < 2; idx++) {
      EEPROM.get(address, curtain[idx].isEnabled);
      address += sizeof(curtain[idx].isEnabled);

      EEPROM.get(address, curtain[idx].isOpenAtSunrise);
      address += sizeof(curtain[idx].isOpenAtSunrise);

      EEPROM.get(address, curtain[idx].isOpenAtTime);
      address += sizeof(curtain[idx].isOpenAtTime);

      EEPROM.get(address, curtain[idx].openAtTime);
      address += sizeof(curtain[idx].openAtTime);

      EEPROM.get(address, curtain[idx].isCloseAtSunset);
      address += sizeof(curtain[idx].isCloseAtSunset);

      EEPROM.get(address, curtain[idx].isCloseAtTime);
      address += sizeof(curtain[idx].isCloseAtTime);

      EEPROM.get(address, curtain[idx].closeAtTime);
      address += sizeof(curtain[idx].closeAtTime);

      EEPROM.get(address, curtain[idx].openingSpeed);
      address += sizeof(curtain[idx].openingSpeed);

      EEPROM.get(address, curtain[idx].closingSpeed);
      address += sizeof(curtain[idx].closingSpeed);
    }
  };

public:
  class ConfigCurtain curtain[2];

  int m_manualSpeed;

  // Geographical and timezone information
  int m_daylightSavingOffset;
  double m_latitude;
  double m_longitude;
};

#endif