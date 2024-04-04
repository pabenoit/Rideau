
#ifndef MOTOR_H
#define MOTOR_H
#include <Arduino.h>

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
      STOP,
      RUNNING
    };

    public:
        Motor(uint16_t pin1, uint16_t pin2): m_action(STOP), m_pin1(pin1), m_pin2(pin2) {}

        void setup();

        void action(enum MotorState state, byte power=255);

        void run();


    public:
    enum MotorAction m_action;
    uint32_t m_stopAt;


    private:
      uint16_t m_pin1;
      uint16_t m_pin2;
      
};

#endif