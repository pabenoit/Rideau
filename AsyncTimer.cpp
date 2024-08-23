#include "AsyncTimer.h"

unsigned short AsyncTimer::m_generateId() {
  unsigned short id = rand() + 1;

  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].id == id)
      return m_generateId();
  }

  return id;
}

unsigned short AsyncTimer::m_newTimerInfo(Callback &callback, time_t sec) {
  if (m_availableIndicesLength == 0 || m_arrayLength == m_maxArrayLength)
    return 0;

  unsigned short id = m_generateId();
  m_availableIndicesLength--;
  unsigned short availableIndex = m_availableIndices[m_availableIndicesLength];
  m_callsArray[availableIndex].id = id;
  m_callsArray[availableIndex].callback = callback;
  m_callsArray[availableIndex].timestamp = sec;
  m_callsArray[availableIndex].active = true;
  m_arrayLength++;

  return id;
}

void AsyncTimer::m_cancelEntry(unsigned short index) {
  m_callsArray[index].active = false;
  m_arrayLength--;
  m_availableIndices[m_availableIndicesLength] = index;
  m_availableIndicesLength++;
}

unsigned short AsyncTimer::_setTimeout(Callback &callback, time_t sec) {
  return m_newTimerInfo(callback, sec);
}

unsigned short AsyncTimer::_setInterval(Callback &callback, time_t sec) {
  return m_newTimerInfo(callback, sec);
}

void AsyncTimer::cancel(unsigned short id) {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].id == id && m_callsArray[i].active)
      m_cancelEntry(i);
  }
}

void AsyncTimer::cancelAll() {
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    m_cancelEntry(i);
  }
}

void AsyncTimer::handle(time_t sec) {
  if (m_arrayLength == 0)
    return;

#if DEBUG_TRACE
  static time_t oldSec = 0;
#endif
  for (unsigned short i = 0; i < m_maxArrayLength; i++) {
    if (m_callsArray[i].active == 1) {
#if DEBUG_TRACE
      if (oldSec != sec) {
        Serial.print("  ");
        Serial.print(i);
        Serial.print(" : ");
        Serial.print(m_callsArray[i].active);
        Serial.print("  -  ");
        Serial.println(m_callsArray[i].timestamp - sec);
      }
#endif
    }

    if (!m_callsArray[i].active || m_callsArray[i].timestamp > sec)
      continue;
    m_cancelEntry(i);
    m_callsArray[i].callback();
  }
#if DEBUG_TRACE
  oldSec = sec;
#endif
}
