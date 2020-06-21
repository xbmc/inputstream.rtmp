/*
 *  Copyright (C) 2016-2020 Arne Morten Kvarving
 *  Copyright (C) 2018-2020 Team Kodi
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "utils/Log.h"
#include "timer/Timer.h"

#include <iostream>
#include <map>
#include <string.h>
#include <sstream>
#include <librtmp/log.h>
#include <librtmp/rtmp.h>

#include <kodi/addon-instance/Inputstream.h>

/***********************************************************
* InputSteam Client AddOn specific public library functions
***********************************************************/

#define  SetAVal(av, cstr)  av.av_val = (char *)cstr; av.av_len = strlen(cstr)
#undef AVC
#define AVC(str)  {(char *)str,sizeof(str)-1}

/* librtmp option names are slightly different */
namespace
{
std::map<std::string, AVal> options =
 {{ "inputstream.rtmp.SWFPlayer", AVC("swfUrl")  },
  { "inputstream.rtmp.swfurl",    AVC("swfUrl")  },
  { "inputstream.rtmp.PageURL",   AVC("pageUrl") },
  { "inputstream.rtmp.PlayPath",  AVC("playpath")},
  { "inputstream.rtmp.TcUrl",     AVC("tcUrl")   },
  { "inputstream.rtmp.IsLive",    AVC("live")    },
  { "inputstream.rtmp.swfvfy",    AVC("swfVfy")  }};
}

class ATTRIBUTE_HIDDEN CInputStreamRTMP
  : public kodi::addon::CInstanceInputStream,
    public rtmpstream::ITimerCallback
{
public:
  CInputStreamRTMP(KODI_HANDLE instance, const std::string& kodiVersion);

  bool Open(const kodi::addon::InputstreamProperty& props) override;
  void Close() override;
  void GetCapabilities(kodi::addon::InputstreamCapabilities& caps) override;
  int ReadStream(uint8_t* buffer, unsigned int bufferSize) override;
  bool PosTime(int ms) override;
  int GetTotalTime() override { return 20; }
  int GetTime() override { return 0; }

private:
  void OnTimeout() override;

  RTMP* m_session = nullptr;
  bool m_readPauseDetected = false;
  mutable std::recursive_mutex m_critSection;
  rtmpstream::CTimer m_readPauseDetectTimer;
};

CInputStreamRTMP::CInputStreamRTMP(KODI_HANDLE instance, const std::string& kodiVersion)
  : CInstanceInputStream(instance, kodiVersion),
    m_readPauseDetectTimer(this)
{
}

bool CInputStreamRTMP::Open(const kodi::addon::InputstreamProperty& props)
{
  const std::string url = props.GetURL();
  const std::map<std::string, std::string> infoValues = props.GetProperties();

  rtmpstream::Log(ADDON_LOG_DEBUG, "InputStream.rtmp: OpenStream() URL: '%s'", url.c_str());

  m_session = RTMP_Alloc();
  RTMP_Init(m_session);

  RTMP_SetupURL(m_session, const_cast<char*>(url.c_str()));

  for (auto& it : options)
  {
    auto info = infoValues.find(it.first);
    if (info != infoValues.end())
    {
      AVal av_tmp;
      SetAVal(av_tmp, info->second.c_str());
      RTMP_SetOpt(m_session, &it.second, &av_tmp);
    }
  }

  if (!RTMP_Connect(m_session, nullptr) || !RTMP_ConnectStream(m_session, 0))
  {
    RTMP_Close(m_session);
    m_session = nullptr;
    return false;
  }

  return true;
}

void CInputStreamRTMP::Close()
{
  m_readPauseDetectTimer.Stop();

  if (m_session)
  {
    std::unique_lock<std::recursive_mutex> lock(m_critSection);

    RTMP_Close(m_session);
    RTMP_Free(m_session);
  }

  m_session = nullptr;
  m_readPauseDetected = false;
}

void CInputStreamRTMP::GetCapabilities(kodi::addon::InputstreamCapabilities& caps)
{
  caps.SetMask(INPUTSTREAM_SUPPORTS_IPOSTIME |
               INPUTSTREAM_SUPPORTS_SEEK |
               INPUTSTREAM_SUPPORTS_PAUSE);
}

int CInputStreamRTMP::ReadStream(uint8_t* buf, unsigned int size)
{
  std::unique_lock<std::recursive_mutex> lock(m_critSection);
  if (m_readPauseDetected)
  {
    m_readPauseDetected = false;
    RTMP_Pause(m_session, false);
    rtmpstream::Log(ADDON_LOG_DEBUG, "InputStream.rtmp: Read resume detected");
  }

  if (m_readPauseDetectTimer.IsRunning())
    m_readPauseDetectTimer.RestartAsync(2 * 1000);
  else
    m_readPauseDetectTimer.Start(2 * 1000);

  return RTMP_Read(m_session, reinterpret_cast<char*>(buf), size);
}

void CInputStreamRTMP::OnTimeout()
{
  std::unique_lock<std::recursive_mutex> lock(m_critSection);
  m_readPauseDetected = true;

  rtmpstream::Log(ADDON_LOG_DEBUG, "InputStream.rtmp: Read pause detected");

  RTMP_Pause(m_session, true);
}

bool CInputStreamRTMP::PosTime(int ms)
{
  std::unique_lock<std::recursive_mutex> lock(m_critSection);

  return RTMP_SendSeek(m_session, ms);
}

/*****************************************************************************************************/

class ATTRIBUTE_HIDDEN CMyAddon
  : public kodi::addon::CAddonBase
{
public:
  CMyAddon() = default;
  ADDON_STATUS CreateInstance(int instanceType,
                              const std::string& instanceID,
                              KODI_HANDLE instance,
                              const std::string& version,
                              KODI_HANDLE& addonInstance) override

  {
    if (instanceType == ADDON_INSTANCE_INPUTSTREAM)
    {
      addonInstance = new CInputStreamRTMP(instance, version);
      return ADDON_STATUS_OK;
    }
    return ADDON_STATUS_NOT_IMPLEMENTED;
  }
};

ADDONCREATOR(CMyAddon)
