/*
 *  Copyright (C) 2005-2020 Team Kodi
 *  https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "TimerHelper.h"

#include <functional>

namespace rtmpstream
{

class ATTRIBUTE_HIDDEN ITimerCallback
{
public:
  virtual ~ITimerCallback() = default;

  virtual void OnTimeout() = 0;
};

class ATTRIBUTE_HIDDEN CTimer
{
public:
  explicit CTimer(ITimerCallback *callback);
  explicit CTimer(std::function<void()> const& callback);
  ~CTimer();

  bool Start(uint32_t timeout, bool interval = false);
  bool Stop();
  bool Restart();
  void RestartAsync(uint32_t timeout);

  bool IsRunning() { return m_running; }

  float GetElapsedSeconds();
  float GetElapsedMilliseconds();

private:
  void Process();
  unsigned int SystemClockMillis();

  std::function<void()> m_callback;
  uint32_t m_timeout;
  bool m_interval;
  uint32_t m_endTime;
  CEvent m_eventTimeout;
  CTimeout m_timer;
  std::thread m_thread;
  bool m_running = false;
};

} /* namespace rtmpstream */
