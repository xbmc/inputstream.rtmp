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
 {{ "SWFPlayer", AVC("swfUrl")  },
  { "PageURL",   AVC("pageUrl") },
  { "PlayPath",  AVC("playpath")},
  { "TcUrl",     AVC("tcUrl")   },
  { "IsLive",    AVC("live")    }};
}

class ATTRIBUTE_HIDDEN CInputStreamRTMP
  : public kodi::addon::CInstanceInputStream,
    public rtmpstream::ITimerCallback
{
public:
  CInputStreamRTMP(KODI_HANDLE instance, const std::string& instanceID);

  bool Open(INPUTSTREAM& props) override;
  void Close() override;
  void GetCapabilities(INPUTSTREAM_CAPABILITIES& caps) override;
  INPUTSTREAM_IDS GetStreamIds() override;
  INPUTSTREAM_INFO GetStream(int streamid) override;
  void EnableStream(int streamid, bool enable) override;
  bool OpenStream(int streamid) override;
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

CInputStreamRTMP::CInputStreamRTMP(KODI_HANDLE instance, const std::string& instanceID)
  : CInstanceInputStream(instance, instanceID),
    m_readPauseDetectTimer(this)
{
}

bool CInputStreamRTMP::Open(INPUTSTREAM& props)
{
  rtmpstream::Log(ADDON_LOG_DEBUG, "InputStream.rtmp: OpenStream()");

  m_session = RTMP_Alloc();
  RTMP_Init(m_session);

  RTMP_SetupURL(m_session, const_cast<char*>(props.m_strURL));
  for (auto& it : options)
  {
    for (size_t i = 0; i < props.m_nCountInfoValues; ++i)
    {
      if (it.first == props.m_ListItemProperties[i].m_strKey)
      {
        AVal av_tmp;
        SetAVal(av_tmp, props.m_ListItemProperties[i].m_strValue);
        RTMP_SetOpt(m_session, &it.second, &av_tmp);
      }
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

bool CInputStreamRTMP::OpenStream(int streamid)
{
  return false;
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

void CInputStreamRTMP::GetCapabilities(INPUTSTREAM_CAPABILITIES &caps)
{
  caps.m_mask |= INPUTSTREAM_CAPABILITIES::SUPPORTS_IPOSTIME;
  caps.m_mask |= INPUTSTREAM_CAPABILITIES::SUPPORTS_SEEK;
  caps.m_mask |= INPUTSTREAM_CAPABILITIES::SUPPORTS_PAUSE;
}

INPUTSTREAM_IDS CInputStreamRTMP::GetStreamIds()
{
  INPUTSTREAM_IDS ids = { 0 };
  return ids;
}

INPUTSTREAM_INFO CInputStreamRTMP::GetStream(int streamid)
{
  INPUTSTREAM_INFO info = { INPUTSTREAM_INFO::STREAM_TYPE::TYPE_NONE };
  return info;
}

void CInputStreamRTMP::EnableStream(int streamid, bool enable)
{
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
