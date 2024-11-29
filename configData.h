#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include <EEPROM.h>
#include "CurtainMotor.h"
#include "configurationFile.h"

class CurtainCfg {
public:
  CurtainCfg()
      : isEnabled(false),
        isOpenAtSunrise(true),
        isOpenAtTime(false),
        openAtTime(DEFAULT_OPEN_TIME),
        isCloseAtSunset(true),
        isCloseAtTime(false),
        closeAtTime(DEFAULT_CLOSE_TIME),
        manualAction(NONE) {}

  bool operator==(const CurtainCfg &other) const {
    return (isEnabled == other.isEnabled &&
            isOpenAtSunrise == other.isOpenAtSunrise &&
            isCloseAtSunset == other.isCloseAtSunset &&
            isOpenAtTime == other.isOpenAtTime &&
            isCloseAtTime == other.isCloseAtTime &&
            openAtTime == other.openAtTime &&
            closeAtTime == other.closeAtTime);
  }

  bool operator!=(const CurtainCfg &other) const {
    return !(*this == other);
  }

  char *debugDisplayConfig() {
    static char text[512];
    sprintf(text, "Feature status  (%s)\n\n"
                  "Open At Sunrise (%s)\n"
                  "Open At Time    (%s)\n"
                  "open time       : %2d:%02d\n\n"
                  "Close At Sunset (%s)\n"
                  "Close At Time   (%s)\n"
                  "Close time      : %2d:%02d\n\n",
            isEnabled ? "enable" : "disable",
            isOpenAtSunrise ? "enable" : "disable",
            isOpenAtTime ? "enable" : "disable",
            openAtTime / 60, openAtTime % 60,
            isCloseAtSunset ? "enable" : "disable",
            isCloseAtTime ? "enable" : "disable",
            closeAtTime / 60, closeAtTime % 60);
    return text;
  }

  bool isEnabled;
  bool isOpenAtSunrise;
  bool isOpenAtTime;
  int openAtTime;
  bool isCloseAtSunset;
  bool isCloseAtTime;
  int closeAtTime;

  enum ManualAction { NONE, OPEN, CLOSE } manualAction;
};

class Config {
public:
  Config()
      : m_daylightSavingOffset(DAYLIGHT_SAVING_OFFSET),
        m_latitude(LATITUDE),
        m_longitude(LONGITUDE),
        m_threshold(MAX_TENSION_THRESHOLD) {}

  bool operator==(const Config &other) const {
    return (curtainCfg[0] == other.curtainCfg[0] &&
            curtainCfg[1] == other.curtainCfg[1]);
  }

  bool operator!=(const Config &other) const {
    return !(*this == other);
  }

  void writeCfg2Epprom() {
    int address = 0;
    int version = VERSION;
    updateEeprom(address, version);
    address += sizeof(version);

    updateEeprom(address, m_threshold);
    address += sizeof(m_threshold);

    for (int idx = 0; idx < 2; ++idx) {
      updateEeprom(address, curtainCfg[idx].isEnabled);
      address += sizeof(curtainCfg[idx].isEnabled);

      updateEeprom(address, curtainCfg[idx].isOpenAtSunrise);
      address += sizeof(curtainCfg[idx].isOpenAtSunrise);

      updateEeprom(address, curtainCfg[idx].isOpenAtTime);
      address += sizeof(curtainCfg[idx].isOpenAtTime);

      updateEeprom(address, curtainCfg[idx].openAtTime);
      address += sizeof(curtainCfg[idx].openAtTime);

      updateEeprom(address, curtainCfg[idx].isCloseAtSunset);
      address += sizeof(curtainCfg[idx].isCloseAtSunset);

      updateEeprom(address, curtainCfg[idx].isCloseAtTime);
      address += sizeof(curtainCfg[idx].isCloseAtTime);

      updateEeprom(address, curtainCfg[idx].closeAtTime);
      address += sizeof(curtainCfg[idx].closeAtTime);
    }
  }

  void readCfg2Epprom() {
    int address = 0;
    int version = 0;
    EEPROM.get(address, version);
    address += sizeof(version);

    if (version != VERSION) {
      Serial.println("WARNING: Data in EEPROM do not match the version.");
      return;
    }

    EEPROM.get(address, m_threshold);
    address += sizeof(m_threshold);

    for (int idx = 0; idx < 2; ++idx) {
      EEPROM.get(address, curtainCfg[idx].isEnabled);
      address += sizeof(curtainCfg[idx].isEnabled);

      EEPROM.get(address, curtainCfg[idx].isOpenAtSunrise);
      address += sizeof(curtainCfg[idx].isOpenAtSunrise);

      EEPROM.get(address, curtainCfg[idx].isOpenAtTime);
      address += sizeof(curtainCfg[idx].isOpenAtTime);

      EEPROM.get(address, curtainCfg[idx].openAtTime);
      address += sizeof(curtainCfg[idx].openAtTime);

      EEPROM.get(address, curtainCfg[idx].isCloseAtSunset);
      address += sizeof(curtainCfg[idx].isCloseAtSunset);

      EEPROM.get(address, curtainCfg[idx].isCloseAtTime);
      address += sizeof(curtainCfg[idx].isCloseAtTime);

      EEPROM.get(address, curtainCfg[idx].closeAtTime);
      address += sizeof(curtainCfg[idx].closeAtTime);
    }
  }

  CurtainCfg *getCurtainCfg(int idx) {
    return &curtainCfg[idx];
  }

private:
  template <typename T>
  void updateEeprom(int address, const T &sData) {
    T rData;
    EEPROM.get(address, rData);
    if (rData != sData) {
      EEPROM.put(address, sData);
    }
  }


public:
  CurtainCfg curtainCfg[2];
  int m_daylightSavingOffset;
  double m_latitude;
  double m_longitude;
  int m_threshold;
};

#endif