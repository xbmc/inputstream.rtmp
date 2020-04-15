/*
 *  Copyright (C) 2005-2020 Team Kodi
 *  https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Timer.h"

#include <algorithm>

namespace rtmpstream
{

CTimer::CTimer(std::function<void()> const& callback)
  : m_callback(callback),
    m_timeout(0),
    m_interval(false),
    m_endTime(0)
{ }

CTimer::CTimer(ITimerCallback *callback)
  : CTimer(std::bind(&ITimerCallback::OnTimeout, callback))
{ }

CTimer::~CTimer()
{
  Stop();
}

bool CTimer::Start(uint32_t timeout, bool interval /* = false */)
{
  if (m_callback == nullptr || timeout == 0 || IsRunning())
    return false;    

  m_timeout = timeout;
  m_interval = interval;

  m_eventTimeout.Reset();

  m_running = true;
  m_thread = std::thread{&CTimer::Process, this};

  m_thread.detach();

  return true;
}

bool CTimer::Stop()
{
  if (!m_running)
    return false;

  m_running = false;
  m_eventTimeout.Signal();

  return true;
}

void CTimer::RestartAsync(uint32_t timeout)
{
  m_timeout = timeout;
  m_endTime = SystemClockMillis() + timeout;
  m_eventTimeout.Signal();
}

bool CTimer::Restart()
{
  if (!IsRunning())
    return false;

  Stop();
  return Start(m_timeout, m_interval);
}

float CTimer::GetElapsedSeconds()
{
  return GetElapsedMilliseconds() / 1000.0f;
}

float CTimer::GetElapsedMilliseconds()
{
  if (!IsRunning())
    return 0.0f;

  return static_cast<float>(m_timer.TimeLeft());
}

void CTimer::Process()
{
  while (m_running)
  {
    uint32_t currentTime = SystemClockMillis();
    m_endTime = currentTime + m_timeout;
    m_timer.Init(m_timeout);

    // wait the necessary time
    if (!m_eventTimeout.Wait(m_timeout))
    {
      currentTime = SystemClockMillis();
      if (m_running && m_endTime <= currentTime)
      {
        // execute OnTimeout() callback
        m_callback();

        // continue if this is an interval timer, or if it was restarted during callback
        if (!m_interval && m_endTime <= currentTime)
          break;
      }
    }
  }

  m_running = false;
}

unsigned int CTimer::SystemClockMillis()
{
  uint64_t now_time;
  static uint64_t start_time = 0;
  static bool start_time_set = false;

  now_time = static_cast<int64_t>(std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() * 1000.0);

  if (!start_time_set)
  {
    start_time = now_time;
    start_time_set = true;
  }
  return (unsigned int)(now_time - start_time);
}

} /* namespace rtmpstream */
