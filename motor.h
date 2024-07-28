
#ifndef MOTOR_H
#define MOTOR_H
#include <Arduino.h>
#include <ezButton.h>

extern int ropeTentionCur;
extern int ropeTentionPrev;

class Motor
{
public:
  enum MotorState
  {
    FOWARD,
    REVERSE,
    STANDBY,
    BREAK
  };

  enum MotorAction
  {
    A_STOP,
    A_FOWARD,
    A_REVERSE
  };

public:
  Motor(uint16_t enA, uint16_t pin1, uint16_t pin2, float rideauTentionThreshold) : m_action(A_STOP),
                                                                                    m_enA(enA),
                                                                                    m_pin1(pin1),
                                                                                    m_pin2(pin2),
                                                                                    m_rideauTentionThreshold(rideauTentionThreshold) {}

  void setup();

  void action(enum MotorState state, byte power = 255);

  void run();

  void setTentionThreshold(int tention) { m_rideauTentionThreshold = tention; }

public:
  enum MotorAction m_action;
  uint32_t m_stopAt;

private:
  uint16_t m_enA;
  uint16_t m_pin1;
  uint16_t m_pin2;
  float m_rideauTentionThreshold;
};

#endif