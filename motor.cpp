#include "Motor.h"

void Motor::setup() {
  // Specified pin to behave either as an input or an output
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);

  action(STANDBY);
  action(STANDBY);
}

void Motor::action(enum MotorState state, byte power) {
  // Send PWM signal to L298N

  switch (state) {
    case FOWARD:
      Serial.print("FOWARD: ");
      Serial.println(power);

      analogWrite(m_pin1, 0);
      analogWrite(m_pin2, power);
      m_action = RUNNING;
      m_stopAt = millis() +  (((255-power)*1000 / 54)+1000);
      break;
    case REVERSE:
      Serial.print("REVERSE: ");
      Serial.println(power);

      analogWrite(m_pin1, power);
      analogWrite(m_pin2, 0);
      m_action = RUNNING;
      m_stopAt = millis() + (((255-power)*1000 / 54)+1000);
      break;
    case STANDBY:
      Serial.println("STANDBY: ");

      analogWrite(m_pin1, 0);
      analogWrite(m_pin2, 0);
      m_action = STOP;
      break;

    case BREAK:
      Serial.println("BREAK: ");

      analogWrite(m_pin1, 1);
      analogWrite(m_pin2, 1);
      m_action = STOP;
      break;
  }
}

void Motor::run() {
  if ((m_action == RUNNING) && (millis() > m_stopAt))
    action(BREAK, 0);
}
