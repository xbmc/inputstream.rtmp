cmake_minimum_required(VERSION 3.5)
project(inputstream.rtmp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(PkgConfig)
find_package(Kodi REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(RTMP REQUIRED)

set(RTMP_SOURCES src/RTMPStream.cpp
                 src/timer/Timer.cpp)

set(RTMP_HEADERS src/utils/Log.h
                 src/timer/Timer.h
                 src/timer/TimerHelper.h)

include_directories(${INCLUDES}
                    ${RTMP_INCLUDE_DIRS}
                    ${OPENSSL_INCLUDE_DIR}
                    ${ZLIB_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}/..) # Hack way with "/..", need bigger Kodi cmake rework to match right include ways (becomes done in future)

list(APPEND DEPLIBS ${RTMP_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES})

if(CORE_SYSTEM_NAME MATCHES "windows")
  list(APPEND DEPLIBS Winmm.lib WS2_32.lib)
endif()

build_addon(inputstream.rtmp RTMP DEPLIBS)

if(CORE_SYSTEM_NAME STREQUAL windowsstore)
  # fix linking
  set_target_properties(inputstream.rtmp PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/LTCG /defaultlib:vccorlib.lib /defaultlib:msvcrt.lib")
  set_target_properties(inputstream.rtmp PROPERTIES LINK_FLAGS_RELEASE "/LTCG /defaultlib:vccorlib.lib /defaultlib:msvcrt.lib")
  set_target_properties(inputstream.rtmp PROPERTIES LINK_FLAGS_DEBUG "/LTCG /defaultlib:vccorlibd.lib /defaultlib:msvcrtd.lib")
endif()

include(CPack)
