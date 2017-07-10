project(openssl)

cmake_minimum_required(VERSION 2.8)

include(CheckSymbolExists)
check_symbol_exists(_X86_ "Windows.h" _X86_)
check_symbol_exists(_AMD64_ "Windows.h" _X64_)

if(_X86_)
  set(OPENSSL_ARCHIVE_NAME openssl-1.0.2g-win32-vc140-v2)
  set(OPENSSL_INSTALL_DIR project/BuildDependencies)
elseif(_X64_)
  set(OPENSSL_ARCHIVE_NAME openssl-1.0.2k-x64-vc140)
  set(OPENSSL_INSTALL_DIR project/BuildDependencies/x64)
else()
  message(FATAL_ERROR "Unsupported architecture")
endif()

message(STATUS "Downloading http://mirrors.kodi.tv/build-deps/win32/${OPENSSL_ARCHIVE_NAME}.7z")
file(DOWNLOAD http://mirrors.kodi.tv/build-deps/win32/${OPENSSL_ARCHIVE_NAME}.7z ${CMAKE_CURRENT_BINARY_DIR}/${OPENSSL_ARCHIVE_NAME}.7z SHOW_PROGRESS)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzvf ${OPENSSL_ARCHIVE_NAME}.7z WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${OPENSSL_ARCHIVE_NAME}/${OPENSSL_INSTALL_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX})
