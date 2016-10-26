project(openssl)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)

set(configure_command MACHINE=${PLATFORM} <SOURCE_DIR>/config no-shared zlib)

if(CORE_SYSTEM_NAME MATCHES "android")
  set(configure_command <SOURCE_DIR>/Configure shared zlib
                                               --openssldir=${OUTPUT_DIR}
                                               --with-zlib-include=${OUTPUT_DIR}/include
                                               --with-zlib-lib=${OUTPUT_DIR}/lib
                                               linux-generic32)
endif()

if(CORE_SYSTEM_NAME MATCHES "ios")
  set(configure_command CC=${CMAKE_COMPILER} AR=${CMAKE_AR} <SOURCE_DIR>/Configure iphoneos-cross zlib no-asm no-krb5 --openssldir=${OUTPUT_DIR})
  set(build_command && sed -ie "s|CFLAG= |CFLAG=${CMAKE_C_FLAGS} |" ${PROJECT_SOURCE_DIR}/Makefile
                    && sed -ie "s|-isysroot $.CROSS_TOP./SDKs/$.CROSS_SDK. ||" ${PROJECT_SOURCE_DIR}/Makefile
                    && sed -ie "s|static volatile sig_atomic_t intr_signal|static volatile intr_signal|" ${PROJECT_SOURCE_DIR}/crypto/ui/ui_openssl.c)
endif()

if(CORE_SYSTEM_NAME MATCHES "osx")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(configure_command <SOURCE_DIR>/Configure darwin64-x86_64-cc zlib no-asm no-krb5 shared --openssldir=${OUTPUT_DIR})
  else()
    set(configure_command <SOURCE_DIR>/Configure darwin-x86-cc zlib no-asm no-krb5 shared --openssldir=${OUTPUT_DIR})
  endif()
endif()

externalproject_add(openssl
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND ""
                    CONFIGURE_COMMAND ${configure_command}
                    BUILD_COMMAND echo Building openssl ${build_command}
                    COMMAND sed -ie "s|apps test||" ${PROJECT_SOURCE_DIR}/Makefile
                    COMMAND make depend
                    COMMAND make
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)

install(CODE "execute_process(COMMAND make install_sw WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
              execute_process(COMMAND rm -f ${OUTPUT_DIR}/lib/libcrypto.so* WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
              execute_process(COMMAND rm -f ${OUTPUT_DIR}/lib/libssl.so* WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
              execute_process(COMMAND rm -f ${OUTPUT_DIR}/lib/libcrypto*dylib* WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
              execute_process(COMMAND rm -f ${OUTPUT_DIR}/lib/libssl*dylib* WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})")
