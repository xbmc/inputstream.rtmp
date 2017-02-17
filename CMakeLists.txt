project(inputstream.mpd)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(PkgConfig)
find_package(Kodi REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(RTMP REQUIRED)

set(RTMP_SOURCES src/RTMPStream.cpp)

include_directories(${INCLUDES}
                    ${RTMP_INCLUDE_DIRS}
                    ${OPENSSL_INCLUDE_DIR}
                    ${ZLIB_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR})
                    
list(APPEND DEPLIBS ${RTMP_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES})

if(CORE_SYSTEM_NAME MATCHES "windows")
	list(APPEND DEPLIBS Winmm.lib WS2_32.lib)
endif()

build_addon(inputstream.rtmp RTMP DEPLIBS)

include(CPack)
