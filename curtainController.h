#ifndef CURTAIN_CONTROLLER_H
#define CURTAIN_CONTROLLER_H

#include "Motor.h"
#include "configurationFile.h"

class CurtainController {
public:
  CurtainController(const char *wifiSsid, const char *wifiPassword)
    : m_openTimerId{ 0, 0 },   // Timers not configured use 0
      m_closeTimerId{ 0, 0 },  // Timers not configured use 0
      m_motorCurtain({ Motor(MOTOR1_PIN_ENA,
                            MOTOR1_PIN1,
                            MOTOR1_PIN2,
                            MAX_TENSION_THRESHOLD),
                      Motor(MOTOR2_PIN_ENA,
                            MOTOR2_PIN1,
                            MOTOR2_PIN2,
                            MAX_TENSION_THRESHOLD) }),
      m_webStatus(WL_IDLE_STATUS) {
    strncpy(m_wifiSsid, wifiSsid, sizeof(m_wifiSsid));
    strncpy(m_wifiPassword, wifiPassword, sizeof(m_wifiPassword));
  };

  void setup(void) {
    m_motorCurtain[0].setup();
    m_motorCurtain[1].setup();
  };

  void run(void) {
    m_motorCurtain[0].run();
    m_motorCurtain[1].run();
  }

  int getWebStatus() {
    return m_webStatus;
  }
  void setWebStatus(int webStatus) {
    m_webStatus = webStatus;
  }

  char *getWifiSsid() {
    return m_wifiSsid;
  };
  char *getWifiPassword() {
    return m_wifiPassword;
  };

public:
  unsigned short m_openTimerId[2];   // timer not configure use 0
  unsigned short m_closeTimerId[2];  // timer not configure use 0
  Motor m_motorCurtain[2];

private:

  // WEB Server
  int m_webStatus;  // connection status

  char m_wifiSsid[48];      // network SSID (name)
  char m_wifiPassword[48];  // network password
};

#endif