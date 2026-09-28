cmake_minimum_required(VERSION 3.15)
project(librtmp C)
include(${CMAKE_CURRENT_SOURCE_DIR}/FetchSource.cmake)

if(CORE_SYSTEM_NAME STREQUAL osx OR CORE_SYSTEM_NAME STREQUAL ios OR CORE_SYSTEM_NAME STREQUAL darwin_embedded)
  set(SYS darwin)
else()
  set(SYS posix)
endif()

# Build only the static library using upstream Makefile overrides. _DEBUG
# enables upstream protocol test data and references tool-only dump streams.
include(ExternalProject)
externalproject_add(librtmp
                    SOURCE_DIR "${rtmpdump_SOURCE_DIR}"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND $(MAKE) -C <SOURCE_DIR>/librtmp all librtmp.pc
                                  SHARED= SYS=${SYS} CRYPTO=OPENSSL
                                  "prefix=${OUTPUT_DIR}" "INC=-I${OUTPUT_DIR}/include"
                                  "XCFLAGS=${CMAKE_C_FLAGS} -fPIC" XDEF=-U_DEBUG
                                  "CC=${CMAKE_C_COMPILER}" "AR=${CMAKE_AR}"
                    INSTALL_COMMAND "")

install(FILES "${rtmpdump_SOURCE_DIR}/librtmp/librtmp.a" DESTINATION lib)
install(FILES "${rtmpdump_SOURCE_DIR}/librtmp/librtmp.pc" DESTINATION lib/pkgconfig)
install(FILES "${rtmpdump_SOURCE_DIR}/librtmp/amf.h" "${rtmpdump_SOURCE_DIR}/librtmp/http.h"
              "${rtmpdump_SOURCE_DIR}/librtmp/log.h" "${rtmpdump_SOURCE_DIR}/librtmp/rtmp.h"
        DESTINATION include/librtmp)
install(FILES "${rtmpdump_SOURCE_DIR}/librtmp/librtmp.3" DESTINATION share/man/man3)
