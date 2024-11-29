#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include <HX711.h>

extern int ropeTensionCur;

class CurtainMotor
{
public:
    enum CurtainMotorAction
    {
        FORWARD,
        REVERSE,
        STANDBY,
        BRAKE
    };

    CurtainMotor(HX711 *scale, uint16_t pin1, uint16_t pin2)
        : m_scale(scale), m_pin1(pin1), m_pin2(pin2) {}

    void setup();
    void run(CurtainMotorAction action);

private:
    void doAction(CurtainMotorAction action);

    HX711 *m_scale;
    uint16_t m_pin1;
    uint16_t m_pin2;
};

#endif // MOTOR_H