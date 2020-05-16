/*
 *  Copyright (C) 2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <kodi/General.h>

namespace rtmpstream
{

inline const char* kodiTranslateLogLevel(const AddonLog logLevel)
{
  switch (logLevel)
  {
    case ADDON_LOG_DEBUG:
      return "LOG_DEBUG:   ";
    case ADDON_LOG_INFO:
      return "LOG_INFO:    ";
    case ADDON_LOG_WARNING:
      return "LOG_WARNING: ";
    case ADDON_LOG_ERROR:
      return "LOG_ERROR:   ";
    case ADDON_LOG_FATAL:
      return "LOG_FATAL:   ";
    default:
      break;
  }
  return "LOG_UNKNOWN: ";
}

inline void Log(const AddonLog logLevel, const char* format, ...)
{
  char buffer[16384];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  kodi::Log(logLevel, buffer);
#ifdef DEBUG
  fprintf(stderr, "%s%s\n", kodiTranslateLogLevel(logLevel), buffer);
#endif
}

} /* namespace rtmpstream */
