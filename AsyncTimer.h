/*
  Copyright (c) 2022 Aasim-A
  Repository: https://github.com/Aasim-A/AsyncTimer

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#ifndef ASYNC_TIMER_H
#define ASYNC_TIMER_H

#include <Arduino.h>

class AsyncTimer {
private:
  class Callback {
  public:
    using FuncPtr = void (*)();

    Callback() = default;
    Callback(const Callback&) = delete;

    Callback(FuncPtr funcPtr)
      : m_isLambda(false), m_funcPtr(funcPtr) {}

    template<typename T>
    Callback(T lambda)
      : m_isLambda(true) {
      m_lambda.data = new T(lambda);
      m_lambda.execFunc = [](void* data) {
        (*reinterpret_cast<T*>(data))();
      };
      m_lambda.deleteFunc = [](void* data) {
        delete reinterpret_cast<T*>(data);
      };
    }

    ~Callback() {
      if (m_isLambda && m_lambda.data) {
        m_lambda.deleteFunc(m_lambda.data);
      }
    }

    void operator()() {
      if (m_isLambda) {
        return m_lambda.execFunc(m_lambda.data);
      }
      return m_funcPtr();
    }

    Callback& operator=(Callback& other) {
      if (m_isLambda && m_lambda.data) {
        m_lambda.deleteFunc(m_lambda.data);
      }
      m_isLambda = other.m_isLambda;
      if (m_isLambda) {
        m_lambda = other.m_lambda;
        other.m_lambda.data = nullptr;
      } else {
        m_funcPtr = other.m_funcPtr;
      }
      return *this;
    }

  private:
    using LambdaExecFunc = void (*)(void*);
    using LambdaDeleteFunc = void (*)(void*);

    struct LambdaStore {
      void* data;
      LambdaExecFunc execFunc;
      LambdaDeleteFunc deleteFunc;
    };

    bool m_isLambda{ false };
    union {
      LambdaStore m_lambda;
      FuncPtr m_funcPtr;
    };
  };

  struct m_TimerInfo {
    unsigned short id;
    Callback callback;
    time_t timestamp;
    bool active;

    m_TimerInfo()
      : active(false) {}
  };

  unsigned short m_generateId();
  unsigned short m_newTimerInfo(Callback& callback, time_t sec);
  void m_cancelEntry(unsigned short index);
  unsigned short _setTimeout(Callback& callback, time_t sec);
  unsigned short _setInterval(Callback& callback, time_t sec);

  unsigned short m_maxArrayLength;
  unsigned short m_arrayLength = 0;
  m_TimerInfo* m_callsArray;
  unsigned short m_availableIndicesLength;
  unsigned short* m_availableIndices;

public:
  AsyncTimer(unsigned short arrayLength = 10) {
    m_maxArrayLength = arrayLength;
    m_callsArray = new m_TimerInfo[arrayLength];
    m_availableIndicesLength = arrayLength;
    m_availableIndices = new unsigned short[arrayLength];
    for (unsigned short i = 0; i < m_availableIndicesLength; i++)
      m_availableIndices[i] = i;
  }

  ~AsyncTimer() {
    delete[] m_callsArray;
    delete[] m_availableIndices;
  }


  unsigned short setTimeout(Callback::FuncPtr func_ptr, time_t sec) {
    Callback func_ptr_cb(func_ptr);
    return _setTimeout(func_ptr_cb, sec);
  };

  template<typename T>
  unsigned short setTimeout(T lambda, time_t sec) {
    Callback lambda_cb(lambda);
    return _setTimeout(lambda_cb, sec);
  };

  unsigned short setInterval(Callback::FuncPtr func_ptr, time_t sec) {
    Callback func_ptr_cb(func_ptr);
    return _setInterval(func_ptr_cb, sec);
  };

  template<typename T>
  unsigned short setInterval(T lambda, time_t sec) {
    Callback lambda_cb(lambda);
    return _setInterval(lambda_cb, sec);
  };

  unsigned long getRemaining(unsigned short id);
  void cancel(unsigned short id);
  void cancelAll();
  void handle(time_t sec);
};

#endif
