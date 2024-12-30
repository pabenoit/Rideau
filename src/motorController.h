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
  Motor(int pin1, int pin2, int speed, std::function<int()> getThresholdFunc, std::function<int()> readCurrentFunc)
      : m_pin1(pin1), m_pin2(pin2), m_speed(speed), m_getThresholdFunc(getThresholdFunc), m_readCurrentFunc(readCurrentFunc)
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
  std::function<int()> m_getThresholdFunc;
  std::function<int()> m_readCurrentFunc;
};

#endif  // MOTORCONTROLLER_H