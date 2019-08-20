cmake_minimum_required(VERSION 3.5)
project(librtmp)

if(CORE_SYSTEM_NAME STREQUAL osx OR CORE_SYSTEM_NAME STREQUAL ios OR CORE_SYSTEM_NAME STREQUAL darwin_embedded)
  set(SYS darwin)
else()
  set(SYS posix)
endif()

set(cflags ${CMAKE_C_FLAGS} -I${OUTPUT_DIR}/include)

include(ExternalProject)
externalproject_add(librtmp
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND cd <SOURCE_DIR>
                                  && make -C librtmp SHARED= prefix=${OUTPUT_DIR} INC=-I${OUTPUT_DIR}/include SYS=${SYS}
                                     XCFLAGS=${cflags} XDEF=-U_DEBUG CC=${CMAKE_C_COMPILER} AR=${CMAKE_AR} XLDFLAGS=${OUTPUT_DIR}/lib
                    INSTALL_COMMAND "")

install(CODE "execute_process(COMMAND make -C librtmp install
                                      prefix=${OUTPUT_DIR} SYS=${SYS} XCFLAGS=-fpic SHARED=
                              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
