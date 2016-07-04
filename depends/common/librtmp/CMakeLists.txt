project(librtmp)

cmake_minimum_required(VERSION 2.8)
set(SYS posix)
if(CORE_SYSTEM_NAME MATCHES "osx" OR
   CORE_SYSTEM_NAMES MATCHES "ios")
  set(SYS darwin)
endif()

include(ExternalProject)
externalproject_add(librtmp
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND cd <SOURCE_DIR> && make -C librtmp SHARED= prefix=${OUTPUT_DIR} SYS=${SYS} XCFLAGS=-fpic
                    INSTALL_COMMAND "")

install(CODE "execute_process(COMMAND make -C librtmp install
                                      prefix=${OUTPUT_DIR} SYS=${SYS} XCFLAGS=-fpic SHARED=
                              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
