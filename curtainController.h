#ifndef CURTAIN_CONTROLLER_H
#define CURTAIN_CONTROLLER_H

#include "CurtainMotor.h"
#include "configurationFile.h"

class ControllerCurtain
{
public:
  ControllerCurtain(const char *wifiSsid, const char *wifiPassword, HX711 *scale)
      : m_motorCurtain{CurtainMotor(scale, MOTOR1_PIN1, MOTOR1_PIN2),
                      CurtainMotor(scale, MOTOR2_PIN1, MOTOR2_PIN2)},
        m_webStatus(WL_IDLE_STATUS)
  {
    strncpy(m_wifiSsid, wifiSsid, sizeof(m_wifiSsid));
    strncpy(m_wifiPassword, wifiPassword, sizeof(m_wifiPassword));
  }

  void setup()
  {
    m_motorCurtain[0].setup();
    m_motorCurtain[1].setup();
  }

  CurtainMotor *getMotor(int idx)
  {
    return &(m_motorCurtain[idx]);
  }

  int getWebStatus() const
  {
    return m_webStatus;
  }
  void setWebStatus(int webStatus)
  {
    m_webStatus = webStatus;
  }

  const char *getWifiSsid() const
  {
    return m_wifiSsid;
  }
  const char *getWifiPassword() const
  {
    return m_wifiPassword;
  }

public:
  CurtainMotor m_motorCurtain[2];

private:
  int m_webStatus; // connection status

  char m_wifiSsid[48];     // network SSID (name)
  char m_wifiPassword[48]; // network password
};

#endif