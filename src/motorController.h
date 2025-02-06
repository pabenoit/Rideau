#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <functional>

extern int getThreshold();
extern int readCurrent();

void clearTensionLog();
void logTension(int value);

int readTension();

class Motor
{
public:
  enum MotorAction
  {
    FORWARD,
    REVERSE,
    STANDBY,
    BRAKE
  };

  //    Motor(int pin1, int pin2) : m_pin1(pin1), m_pin2(pin2) {}
  Motor(int pin1, int pin2, int speed,
        std::function<int()> getMotorThresholdMaxFunc,
        std::function<int()> getMotorThresholdFunc,
        std::function<int()> getMotorBlindTimeFunc,
        std::function<int()> getMotorRunTimeLimitFunc,
        std::function<int()> getMotorSpeedFunc,
        std::function<int()> readCurrentFunc)
      : m_pin1(pin1), m_pin2(pin2), m_speed(speed),

        m_getMotorThresholdMaxFunc(getMotorThresholdMaxFunc),
        m_getMotorThresholdFunc(getMotorThresholdFunc),
        m_getMotorBlindTimeFunc(getMotorBlindTimeFunc),
        m_getMotorRunTimeLimitFunc(getMotorRunTimeLimitFunc),
        m_getMotorSpeedFunc(getMotorSpeedFunc),
        m_readCurrentFunc(readCurrentFunc)
  {
  }

  void setup();
  void performAction(MotorAction action);
  void run(MotorAction action);
  int readCurrent(uint32_t logTime);

private:
  int m_pin1;
  int m_pin2;
  int m_speed;

  std::function<int()> m_getMotorThresholdMaxFunc;
  std::function<int()> m_getMotorThresholdFunc;
  std::function<int()> m_getMotorBlindTimeFunc;
  std::function<int()> m_getMotorRunTimeLimitFunc;
  std::function<int()> m_getMotorSpeedFunc;
  std::function<int()> m_readCurrentFunc;
};

#endif // MOTORCONTROLLER_H