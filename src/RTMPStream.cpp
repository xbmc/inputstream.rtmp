/*
 *      Copyright (C) 2016 Arne Morten Kvarving
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  <http://www.gnu.org/licenses/>.
 *
 */

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
  : public kodi::addon::CInstanceInputStream
{
public:
  CInputStreamRTMP(KODI_HANDLE instance);

  bool Open(INPUTSTREAM& props) override;
  void Close() override;
  void GetCapabilities(INPUTSTREAM_CAPABILITIES& caps) override;
  INPUTSTREAM_IDS GetStreamIds() override;
  INPUTSTREAM_INFO GetStream(int streamid) override;
  void EnableStream(int streamid, bool enable) override;
  bool OpenStream(int streamid) override;
  int ReadStream(uint8_t* buffer, unsigned int bufferSize) override;
  void PauseStream(double time) override;
  bool PosTime(int ms) override;
  int GetTotalTime() override { return 20; }
  int GetTime() override { return 0; }
  bool CanPauseStream() override { return true; }
  bool CanSeekStream() override { return true; }

private:
  RTMP* m_session = nullptr;
  bool m_paused = false;
};

CInputStreamRTMP::CInputStreamRTMP(KODI_HANDLE instance)
  : CInstanceInputStream(instance)
{
}

bool CInputStreamRTMP::Open(INPUTSTREAM& props)
{
  kodi::Log(ADDON_LOG_DEBUG, "InputStream.rtmp: OpenStream()");

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
  if (m_session)
  {
    RTMP_Close(m_session);
    RTMP_Free(m_session);
  }
  m_session = nullptr;
  m_paused = false;
}

void CInputStreamRTMP::GetCapabilities(INPUTSTREAM_CAPABILITIES &caps)
{
  caps.m_mask |= INPUTSTREAM_CAPABILITIES::SUPPORTS_IPOSTIME;
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
  return RTMP_Read(m_session, reinterpret_cast<char*>(buf), size);
}

void CInputStreamRTMP::PauseStream(double time)
{
  m_paused = !m_paused;
  RTMP_Pause(m_session, m_paused);
}

bool CInputStreamRTMP::PosTime(int ms)
{
  return RTMP_SendSeek(m_session, ms);
}

/*****************************************************************************************************/

class ATTRIBUTE_HIDDEN CMyAddon
  : public kodi::addon::CAddonBase
{
public:
  CMyAddon() = default;
  ADDON_STATUS CreateInstance(int instanceType, std::string instanceID, KODI_HANDLE instance, KODI_HANDLE& addonInstance) override
  {
    if (instanceType == ADDON_INSTANCE_INPUTSTREAM)
    {
      addonInstance = new CInputStreamRTMP(instance);
      return ADDON_STATUS_OK;
    }
    return ADDON_STATUS_NOT_IMPLEMENTED;
  }
};

ADDONCREATOR(CMyAddon)
