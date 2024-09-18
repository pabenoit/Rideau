#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

extern int ropeTensionCur;

class Motor {
public:
  enum MotorOperation {
    O_NORMAL,
    O_REMOVE_TENTION,
    O_NONE
  };

  enum MotorState {
    FORWARD,
    REVERSE,
    STANDBY,
    BRAKE
  };

  enum MotorAction {
    A_STOP,
    A_FORWARD,
    A_REVERSE
  };

public:
  Motor(uint16_t enA, uint16_t pin1, uint16_t pin2, float curtainTensionThreshold)
    : m_action(A_STOP),
      m_operation(O_NONE),
      m_enA(enA),
      m_pin1(pin1),
      m_pin2(pin2),
      m_curtainTensionThreshold(curtainTensionThreshold) {}

  void setup();

  void action(enum MotorOperation operation, enum MotorState state, byte power = 255);

  void run();

  void setTensionThreshold(int tension) {
    m_curtainTensionThreshold = tension;
  }

private:
  enum MotorAction m_action;
  enum MotorOperation m_operation;
  uint32_t m_stopAt;
  uint16_t m_enA;
  uint16_t m_pin1;
  uint16_t m_pin2;
  float m_curtainTensionThreshold;
};

#endif