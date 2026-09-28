cmake_minimum_required(VERSION 3.15)
project(librtmp C CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../common/librtmp/FetchSource.cmake)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

add_library(librtmp STATIC
  ${rtmpdump_SOURCE_DIR}/librtmp/amf.c
  ${rtmpdump_SOURCE_DIR}/librtmp/hashswf.c
  ${rtmpdump_SOURCE_DIR}/librtmp/log.c
  ${rtmpdump_SOURCE_DIR}/librtmp/parseurl.c
  ${rtmpdump_SOURCE_DIR}/librtmp/rtmp.c)
target_compile_definitions(librtmp PRIVATE
  USE_OPENSSL OPENSSL_API_COMPAT=0x10100000L
  _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS
  _WINSOCK_DEPRECATED_NO_WARNINGS RTMPDUMP_VERSION="v2.6")
# _DEBUG enables upstream test handshakes and tool-only packet dump streams.
target_compile_options(librtmp PRIVATE /U_DEBUG)
target_link_libraries(librtmp PRIVATE OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB Ws2_32 winmm)

if(MSVC AND CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
  target_sources(librtmp PRIVATE ${rtmpdump_SOURCE_DIR}/librtmp/uwpapi.cpp)
  set_source_files_properties(${rtmpdump_SOURCE_DIR}/librtmp/uwpapi.cpp PROPERTIES COMPILE_FLAGS "/TP /ZW")
endif()

install(TARGETS librtmp ARCHIVE DESTINATION lib)
install(FILES ${rtmpdump_SOURCE_DIR}/librtmp/amf.h ${rtmpdump_SOURCE_DIR}/librtmp/http.h ${rtmpdump_SOURCE_DIR}/librtmp/log.h ${rtmpdump_SOURCE_DIR}/librtmp/rtmp.h
        DESTINATION include/librtmp)
