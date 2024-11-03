#include "Motor.h"
#include "HX711.h"

extern HX711 scale;

/**
 * @brief Initializes the motor pins and sets the initial state to STANDBY.
 * 
 * This function configures the specified pins to behave as outputs and sets the motor to the STANDBY state.
 */
void Motor::setup() {
  // Specified pin to behave either as an input or an output
  pinMode(m_enA, OUTPUT);
  pinMode(m_pin1, OUTPUT);
  pinMode(m_pin2, OUTPUT);

  action(O_NONE, STANDBY);
}

/**
 * @brief Controls the motor action based on the specified state and power.
 * 
 * This function sends a PWM signal to the L298N motor driver to control the motor's direction and speed.
 * It sets the motor to move forward, reverse, standby, or brake based on the provided state.
 * 
 * @param state The desired motor state (FORWARD, REVERSE, STANDBY, BRAKE).
 * @param power The power level (PWM value) to be applied to the motor.
 */
void Motor::action(enum MotorOperation operation, enum MotorState state, byte power) {
  m_operation = operation;

  // Send PWM signal to L298N
  switch (state) {
    case FORWARD:
      analogWrite(m_enA, power);
      digitalWrite(m_pin2, HIGH);
      digitalWrite(m_pin1, LOW);
      m_action = A_FORWARD;
      if (m_operation == O_NORMAL)
        m_stopAt = millis() + (20 * 1000);
      if (m_operation == O_REMOVE_TENTION)
        m_stopAt = millis() + (500);
      break;

    case REVERSE:
      analogWrite(m_enA, power);
      digitalWrite(m_pin2, LOW);
      digitalWrite(m_pin1, HIGH);
      m_action = A_REVERSE;
      if (m_operation == O_NORMAL)
        m_stopAt = millis() + (20 * 1000);
      if (m_operation == O_REMOVE_TENTION)
        m_stopAt = millis() + (500);
      break;

    case STANDBY:
      analogWrite(m_enA, power);
      digitalWrite(m_pin1, LOW);
      digitalWrite(m_pin2, LOW);
      m_action = A_STOP;
      break;

    case BRAKE:
      analogWrite(m_enA, power);
      digitalWrite(m_pin1, HIGH);
      digitalWrite(m_pin2, HIGH);
      m_action = A_STOP;
      break;
  }
}

/**
 * @brief Runs the motor control logic.
 * 
 * This function checks the current motor action and stops the motor if the specified time has elapsed
 * or if the tension exceeds the threshold. It ensures the motor stops if the conditions are met.
 */
#if 0
void Motor::run() {
  // Stop after a specified time or if the tension increases and exceeds the threshold.
  // The tension should increase.
  if (m_operation == O_NORMAL)
  {
    if ((m_action == A_FORWARD) && ((millis() > m_stopAt) || (ropeTensionCur > m_curtainTensionThreshold)))
    {
      action(O_REMOVE_TENTION, REVERSE, 255);
    }
    else if ((m_action == A_REVERSE) && ((millis() > m_stopAt) || (ropeTensionCur > m_curtainTensionThreshold)))
    {
      action(O_REMOVE_TENTION, FORWARD, 255);
    }

  }
  else if (m_operation == O_REMOVE_TENTION)
  {
    if ((m_action == A_FORWARD) && (millis() > m_stopAt))
    {
      action(O_NONE, STANDBY, 0);
    }
    else if ((m_action == A_REVERSE) && (millis() > m_stopAt))
    {
      action(O_NONE, STANDBY, 0);
    }
  }
}
#else
void Motor::run() {
  // Stop after a specified time or if the tension increases and exceeds the threshold.
  // The tension should increase.
  if (m_operation == O_NORMAL)
  {
    if ((m_action == A_FORWARD) && ((millis() > m_stopAt) || (ropeTensionCur > m_curtainTensionThreshold)))
    {
      action(O_NONE, STANDBY, 0);
    }
    else if ((m_action == A_REVERSE) && ((millis() > m_stopAt) || (ropeTensionCur > m_curtainTensionThreshold)))
    {
      action(O_NONE, STANDBY, 0);
    }

  }
}
#endif