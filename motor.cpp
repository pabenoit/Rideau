#include "Motor.h"
#include "HX711.h"
extern HX711 scale;

void Motor::setup()
{

  // Specified pin to behave either as an input or an output
  pinMode(m_enA, OUTPUT);
  pinMode(m_pin1, OUTPUT);
  pinMode(m_pin2, OUTPUT);

  action(STANDBY);
}

void Motor::action(enum MotorState state, byte power)
{
  // Send PWM signal to L298N

  switch (state)
  {
  case FOWARD:
    Serial.print("FOWARD: ");
    Serial.println(power);

    analogWrite(m_enA, power);
    digitalWrite(m_pin2, HIGH);
    digitalWrite(m_pin1, LOW);
    m_action = A_FOWARD;
    m_stopAt = millis() + (5 * 60 * 100);
    break;

  case REVERSE:
    Serial.print("REVERSE: ");
    Serial.println(power);

    analogWrite(m_enA, power);
    digitalWrite(m_pin2, LOW);
    digitalWrite(m_pin1, HIGH);
    m_action = A_REVERSE;
    m_stopAt = millis() + (5 * 60 * 100);
    break;

  case STANDBY:
    Serial.println("STANDBY: ");

    analogWrite(m_enA, power);
    digitalWrite(m_pin1, LOW);
    digitalWrite(m_pin2, LOW);
    m_action = A_STOP;
    break;

  case BREAK:
    Serial.println("BREAK: ");

    analogWrite(m_enA, power);
    digitalWrite(m_pin1, HIGH);
    digitalWrite(m_pin2, HIGH);
    m_action = A_STOP;
    break;
  }
}

void Motor::run()
{

  // Stop after x time or if tention increase and exceed the threshold
  // Tention should increase.
  if ((m_action == A_FOWARD) && ((millis() > m_stopAt) ||
                                 ((ropeTentionCur > ropeTentionPrev) && (ropeTentionCur > m_rideauTentionThreshold))))
    action(BREAK, 0);

  if ((m_action == A_REVERSE) && ((millis() > m_stopAt) ||
                                  ((ropeTentionCur > ropeTentionPrev) && (ropeTentionCur > m_rideauTentionThreshold))))
    action(BREAK, 0);

  //  if ((m_action == RUNNING) && (millis() > m_stopAt))
  //    action(BREAK, 0);
}
