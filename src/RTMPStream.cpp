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

#include "xbmc_addon_types.h"
#include "libXBMC_addon.h"
#include "kodi_inputstream_types.h"

ADDON::CHelper_libXBMC_addon *xbmc = nullptr;

/***************************  Interface *********************************/

#include "kodi_inputstream_dll.h"
#include "libKODI_inputstream.h"

extern "C" {
  
  ADDON_STATUS curAddonStatus = ADDON_STATUS_UNKNOWN;
  RTMP* session = nullptr;
  bool paused = false;

  /***********************************************************
  * Standart AddOn related public library functions
  ***********************************************************/

  ADDON_STATUS ADDON_Create(void* hdl, void* props)
  {
    if (!hdl)
      return ADDON_STATUS_UNKNOWN;

    xbmc = new ADDON::CHelper_libXBMC_addon;
    if (!xbmc->RegisterMe(hdl))
    {
      delete xbmc, xbmc = nullptr;
      return ADDON_STATUS_PERMANENT_FAILURE;
    }

    xbmc->Log(ADDON::LOG_DEBUG, "InputStream.rtmp: ADDON_Create()");

    curAddonStatus = ADDON_STATUS_OK;
    return curAddonStatus;
  }

  ADDON_STATUS ADDON_GetStatus()
  {
    return curAddonStatus;
  }

  void ADDON_Destroy()
  {
    if (xbmc)
      xbmc->Log(ADDON::LOG_DEBUG, "InputStream.rtmp: ADDON_Destroy()");

    delete xbmc, xbmc = nullptr;
  }

  bool ADDON_HasSettings()
  {
    return false;
  }

  unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
  {
    return 0;
  }

  ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
  {
    return ADDON_STATUS_OK;
  }

  void ADDON_Stop()
  {
  }

  void ADDON_FreeSettings()
  {
  }

  void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
  {
  }

  /***********************************************************
  * InputSteam Client AddOn specific public library functions
  ***********************************************************/

#define  SetAVal(av, cstr)  av.av_val = (char *)cstr; av.av_len = strlen(cstr)
#undef AVC
#define AVC(str)  {(char *)str,sizeof(str)-1}
      
/* librtmp option names are slightly different */
std::map<std::string, AVal> options = 
 {{ "SWFPlayer", AVC("swfUrl")  },
  { "PageURL",   AVC("pageUrl") },
  { "PlayPath",  AVC("playpath")},
  { "TcUrl",     AVC("tcUrl")   },
  { "IsLive",    AVC("live")    }};

  bool Open(INPUTSTREAM& props)
  {
    xbmc->Log(ADDON::LOG_DEBUG, "InputStream.rtmp: OpenStream()");

    session = RTMP_Alloc();
    RTMP_Init(session);

    RTMP_SetupURL(session, (char*)props.m_strURL);

    for (auto& it : options)
    {
      for (size_t i = 0; i < props.m_nCountInfoValues; ++i)
      {
        if (it.first == props.m_ListItemProperties[i].m_strKey)
        {
          AVal av_tmp;
          SetAVal(av_tmp, props.m_ListItemProperties[i].m_strValue);
          RTMP_SetOpt(session, &it.second, &av_tmp);
        }
      }
    }

    if (!RTMP_Connect(session, nullptr) || !RTMP_ConnectStream(session, 0))
    {
      RTMP_Close(session);
      return false;
    }

    return true;
  }

  void Close(void)
  {
    if (session)
      RTMP_Close(session);
    session = nullptr;
    paused = false;
  }

  struct INPUTSTREAM_CAPABILITIES GetCapabilities()
  {
    INPUTSTREAM_CAPABILITIES caps;
    caps.m_supportsIDemux = false;
    caps.m_supportsIPosTime = true;
    caps.m_supportsIDisplayTime = false;
    return caps;
  }

  int ReadStream(unsigned char* buf, unsigned int size)
  {
    return RTMP_Read(session, (char*)buf, size);
  }

  int64_t SeekStream(int64_t offset, int whence)
  {
    return -1;
  }

  int64_t PositionStream(void)
  {
    return -1;
  }

  int64_t LengthStream(void)
  {
    return -1;
  }

  int GetTotalTime()
  {
    return 20;
  }

  int GetTime()
  {
    return 0;
  }

  bool CanPauseStream(void)
  {
    return true;
  }

  void PauseStream(double dTime)
  {
    paused = !paused;
    RTMP_Pause(session, paused);
  }

  bool CanSeekStream(void)
  {
    return true;
  }

  bool PosTime(int ms)
  {
    return RTMP_SendSeek(session, ms);
  }

  void SetSpeed(int)
  {
  }

  bool IsRealTimeStream(void)
  {
    return false;
  }

  void EnableStream(int streamid, bool enable)
  {
  }

  void EnableStreamAtPTS(int streamid, uint64_t pts)
  {
  }

  INPUTSTREAM_IDS GetStreamIds()
  {
    return INPUTSTREAM_IDS();
  }

  void DemuxSetSpeed(int speed)
  {
  }

  bool DemuxSeekTime(double time, bool backwards, double *startpts)
  {
    return false;
  }

  DemuxPacket* DemuxRead(void)
  {
    return nullptr;
  }

  void DemuxFlush(void)
  {
  }

  void DemuxAbort(void)
  {
  }

  void DemuxReset(void)
  {
  }

  const char* GetPathList(void)
  {
    return "wtf";
  }

  void SetVideoResolution(int width, int height)
  {
  }

  struct INPUTSTREAM_INFO GetStream(int streamid)
  {
    return INPUTSTREAM_INFO(); 
  }

}//extern "C"
