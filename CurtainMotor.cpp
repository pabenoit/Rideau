#include "CurtainMotor.h"
#include "HX711.h"
#include <INA219_WE.h>
#include "configurationFile.h"

#ifdef ARDUINO_UNOR4_WIFI
#include "Arduino_LED_Matrix.h"
extern ArduinoLEDMatrix matrix;
#endif

extern HX711 scale;
extern INA219_WE ina219;

extern int getThreshold();

#define MAX_NB_LOG 1000
int logTensionValue[MAX_NB_LOG] = {};
float logCourantValue[MAX_NB_LOG] = {};
int logTensionIdx = 0;

void clearLogTension()
{
    memset(logTensionValue, 0, sizeof(logTensionValue));
    logTensionIdx = 0;
}

void logTension(int value, float current_mA)
{
    logCourantValue[logTensionIdx] = current_mA;  
    logTensionValue[logTensionIdx] = value;
    logTensionIdx = (logTensionIdx + 1) % MAX_NB_LOG;
}

char *tensionDebug(int idx)
{
    static char logDisplay[255];

    if (idx < MAX_NB_LOG && idx < logTensionIdx)
    {
        switch (logTensionValue[idx])
        {
        case CurtainMotor::FORWARD:
            sprintf(logDisplay, "\nFORWARD: ");
            break;
        case CurtainMotor::REVERSE:
            sprintf(logDisplay, "\nREVERSE: ");
            break;
        case CurtainMotor::STANDBY:
            sprintf(logDisplay, "\nSTANDBY: ");
            break;
        case CurtainMotor::BRAKE:
            sprintf(logDisplay, "\nBRAKE: ");
            break;
        default:
            sprintf(logDisplay, "%d, ", logTensionValue[idx]);
            break;
        }
        return logDisplay;
    }
    return NULL;
}

char *courantDebug(int idx)
{
  
    static char logDisplay[255];
    
    if (idx < MAX_NB_LOG && idx < logTensionIdx)
    {
        if (logCourantValue[idx] == CurtainMotor::FORWARD)
            sprintf(logDisplay, "\nFORWARD: ");
        else if (logCourantValue[idx] == CurtainMotor::REVERSE)
            sprintf(logDisplay, "\nREVERSE: ");
        else if (logCourantValue[idx] == CurtainMotor::STANDBY)
            sprintf(logDisplay, "\nSTANDBY: ");
        else if (logCourantValue[idx] == CurtainMotor::BRAKE)
            sprintf(logDisplay, "\nBRAKE: ");
        else
            sprintf(logDisplay, "%f, ", logCourantValue[idx]);

        return logDisplay;
    }
    return NULL;
}

inline int readTension()
{
    int tension = static_cast<int>(scale.read() / 500);
    float current_mA = ina219.getCurrent_mA();

    logTension(tension, current_mA);
    return tension;
}

void CurtainMotor::setup()
{
    pinMode(m_pin1, OUTPUT);
    pinMode(m_pin2, OUTPUT);
}

void CurtainMotor::doAction(CurtainMotorAction action)
{
    logTension(action, action);

    switch (action)
    {
    case FORWARD:
        digitalWrite(m_pin2, HIGH);
        digitalWrite(m_pin1, LOW);
#ifdef ARDUINO_UNOR4_WIFI
        matrix.stroke(0, 255, 0);
        matrix.loadFrame(LEDMATRIX_ARROW_RIGHT);
#endif
        break;

    case REVERSE:
        digitalWrite(m_pin2, LOW);
        digitalWrite(m_pin1, HIGH);
#ifdef ARDUINO_UNOR4_WIFI
        matrix.stroke(0, 255, 0);
        matrix.loadFrame(LEDMATRIX_ARROW_LEFT);
#endif
        break;

    case STANDBY:
        digitalWrite(m_pin1, LOW);
        digitalWrite(m_pin2, LOW);
#ifdef ARDUINO_UNOR4_WIFI
        matrix.stroke(0, 255, 0);
        matrix.loadFrame(LEDMATRIX_ARROW_STOP);
#endif
        break;

    case BRAKE:
        digitalWrite(m_pin1, HIGH);
        digitalWrite(m_pin2, HIGH);
#ifdef ARDUINO_UNOR4_WIFI
        matrix.loadFrame(LEDMATRIX_ARROW_STOP);
#endif
        break;
    }
}

void CurtainMotor::run(CurtainMotorAction action)
{
    bool cnt = true;
    int prevTension;
    int tension;
    uint32_t maxDuration = millis() + (20 * 1000);

    clearLogTension();

    prevTension = tension = readTension();

    doAction(action);

    char timeString[255];
    while (cnt)
    {
        prevTension = tension;
        tension = readTension();
        cnt = ((tension > getThreshold()) ? (prevTension >= tension) : true) && (millis() < maxDuration);
        sprintf(timeString, "tension(%d) prevTension(%d)  m_rideauTensionThreshold(%d) ", tension, prevTension, getThreshold());
        Serial.println(timeString);
    }

    switch (action)
    {
    case FORWARD:
        doAction(REVERSE);
        break;

    case REVERSE:
        doAction(FORWARD);
        break;

    default:
        delay(1);
    }

    readTension();
    readTension();

    doAction(BRAKE);

    doAction(STANDBY);
    readTension();
    readTension();
}
