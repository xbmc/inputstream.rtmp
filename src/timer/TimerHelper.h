/*
 *  Copyright (C) 2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <chrono>
#include <condition_variable>
#include <ctime>
#include <functional>
#include <kodi/General.h>
#include <mutex>
#include <thread>

namespace rtmpstream
{

inline std::string NowToString()
{
  std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
  time_t t = std::chrono::system_clock::to_time_t(p);
  return std::ctime(&t);
}

class ATTRIBUTE_HIDDEN CTimeout
{
public:
  CTimeout(void) : m_iTarget(0)
  {
  }

  CTimeout(uint32_t iTimeout)
  {
    Init(iTimeout);
  }

  bool IsSet(void) const
  {
    return m_iTarget > 0;
  }

  void Init(uint32_t iTimeout)
  {
    m_iTarget = static_cast<int64_t>(std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() * 1000.0) + iTimeout;
  }

  uint32_t TimeLeft(void) const
  {
    uint64_t iNow = static_cast<int64_t>(std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() * 1000.0);
    return (iNow > m_iTarget) ? 0 : (uint32_t)(m_iTarget - iNow);
  }

private:
  uint64_t m_iTarget;
};

typedef bool (*PredicateCallback) (void *param);

template <typename _Predicate>
class ATTRIBUTE_HIDDEN CCondition
{
private:
  static bool _PredicateCallbackDefault ( void *param )
  {
    _Predicate *p = (_Predicate*)param;
    return (*p);
  }
public:
  inline CCondition(void) {}
  inline ~CCondition(void)
  {
    m_condition.notify_all();
  }

  inline void Broadcast(void)
  {
    m_condition.notify_all();
  }

  inline void Signal(void)
  {
    m_condition.notify_one();
  }

  inline bool Wait(std::recursive_mutex& mutex, uint32_t iTimeout)
  {
    std::unique_lock<std::recursive_mutex> lck(mutex);
    return m_condition.wait_for(lck, std::chrono::milliseconds(iTimeout)) != std::cv_status::timeout;
  }

  inline bool Wait(std::recursive_mutex &mutex, PredicateCallback callback, void* param, uint32_t iTimeout)
  {
    bool bReturn(false);
    CTimeout timeout(iTimeout);

    while (!bReturn)
    {
      if ((bReturn = callback(param)) == true)
        break;
      uint32_t iMsLeft = timeout.TimeLeft();
      if ((iTimeout != 0) && (iMsLeft == 0))
        break;
      std::unique_lock<std::recursive_mutex> lck(mutex);
      m_condition.wait_for(lck, std::chrono::milliseconds(iMsLeft));
    }

    return bReturn;
  }

  inline bool Wait(std::recursive_mutex &mutex, _Predicate &predicate, uint32_t iTimeout = 0)
  {
    return Wait(mutex, _PredicateCallbackDefault, (void*)&predicate, iTimeout);
  }

private:
  std::condition_variable_any m_condition;
};

class ATTRIBUTE_HIDDEN CEvent
{
public:
  CEvent(bool bAutoReset = true) :
    m_bSignaled(false),
    m_bBroadcast(false),
    m_iWaitingThreads(0),
    m_bAutoReset(bAutoReset) {}
  virtual ~CEvent(void) {}

  void Broadcast(void)
  {
    Set(true);
    m_condition.Broadcast();
  }

  void Signal(void)
  {
    Set(false);
    m_condition.Signal();
  }

  bool Wait(void)
  {
    std::unique_lock<std::recursive_mutex> lck(m_mutex);
    ++m_iWaitingThreads;

    bool bReturn = m_condition.Wait(m_mutex, m_bSignaled);
    return ResetAndReturn() && bReturn;
  }

  bool Wait(uint32_t iTimeout)
  {
    if (iTimeout == 0)
      return Wait();

    std::unique_lock<std::recursive_mutex> lck(m_mutex);
    ++m_iWaitingThreads;
    bool bReturn = m_condition.Wait(m_mutex, m_bSignaled, iTimeout);
    return ResetAndReturn() && bReturn;
  }

  static void Sleep(uint32_t iTimeout)
  {
    CEvent event;
    event.Wait(iTimeout);
  }

  void Reset(void)
  {
    m_bSignaled = false;
  }

private:
  void Set(bool bBroadcast = false)
  {
    m_bSignaled  = true;
    m_bBroadcast = bBroadcast;
  }

  bool ResetAndReturn(void)
  {
    std::unique_lock<std::recursive_mutex> lck(m_mutex);
    bool bReturn(m_bSignaled);
    if (bReturn && (--m_iWaitingThreads == 0 || !m_bBroadcast) && m_bAutoReset)
      m_bSignaled = false;
    return bReturn;
  }

  volatile bool m_bSignaled;
  CCondition<volatile bool> m_condition;
  std::recursive_mutex m_mutex;
  volatile bool m_bBroadcast;
  unsigned int m_iWaitingThreads;
  bool m_bAutoReset;
};

} /* namespace rtmpstream */
