#include <stdio.h>
#include <string.h>
#include <vector>

#include <Arduino.h>
#include "motorController.h"

#define DEBUG 1  // Set to 0 to disable debug prints

#if DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

std::vector<std::pair<uint32_t, int>> tensionLog;

void clearTensionLog() { tensionLog.clear(); }

void logTension(uint32_t logTime, int value)
{
  static uint32_t lastLogTime = 0;
  if (logTime - lastLogTime > 150)
  {
    try
    {
      tensionLog.push_back(std::make_pair(logTime, value));
    }
    catch (const std::bad_alloc &)
    {
      // Handle memory allocation failure
      tensionLog.clear();
    }
    lastLogTime = logTime;
  }
}

int Motor::readCurrent(uint32_t logTime)
{
  int tension = m_readCurrentFunc();
  logTension(logTime, tension);
  return tension;
}

void Motor::setup()
{
  pinMode(m_pin1, OUTPUT);
  pinMode(m_pin2, OUTPUT);
}

void Motor::performAction(MotorAction action)
{
  switch (action)
  {
    case FORWARD:
      DEBUG_PRINTLN("Debug: FORWARD");
      digitalWrite(m_pin2, HIGH);
      digitalWrite(m_pin1, LOW);
      break;

    case REVERSE:
      DEBUG_PRINTLN("Debug: REVERSE");
      digitalWrite(m_pin2, LOW);
      digitalWrite(m_pin1, HIGH);
      break;

    case STANDBY:
      DEBUG_PRINTLN("Debug: STANDBY");
      digitalWrite(m_pin1, LOW);
      digitalWrite(m_pin2, LOW);
      break;

    case BRAKE:
      DEBUG_PRINTLN("Debug: BRAKE");
      digitalWrite(m_pin1, HIGH);
      digitalWrite(m_pin2, HIGH);
      break;
  }
}

void Motor::run(MotorAction action)
{
  bool continueRunning = true;
  int currentTension;
  uint32_t startTime = millis();

  clearTensionLog();

  performAction(action);

  while ((millis() < (startTime + 9000)) && (continueRunning))
  {
    currentTension = readCurrent(millis() - startTime);

    if (millis() >= (startTime + 350))
    {
      continueRunning = (currentTension < m_getThresholdFunc());
    }

    // Emengency stop
    if (currentTension > 1600)
    {
      performAction(BRAKE);
      continueRunning = false;
    }

    DEBUG_PRINTLN("currentTension(" + String(currentTension) + ") endTime(" + String(startTime + 9000) + ") millis()(" +
                  String(millis()) + ") threshold(" + String(getThreshold()) + ")");
  }

  switch (action)
  {
    case FORWARD:
      performAction(REVERSE);
      break;

    case REVERSE:
      performAction(FORWARD);
      break;

    default:
      delay(1);
  }

  uint32_t startTime2 = millis();
  while (millis() < (startTime2 + 150))
  {
    currentTension = readCurrent(millis() - startTime);

    DEBUG_PRINTLN("-- currentTension(" + String(currentTension) + ") startTime(" + String(startTime) + ") millis()(" +
                  String(millis()) + ") threshold(" + String(getThreshold()) + ")");
  }

  performAction(STANDBY);
}