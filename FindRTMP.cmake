# - Try to find rtmp
# Once done this will define
#
# RTMP_FOUND - system has librtmp
# RTMP_INCLUDE_DIRS - the librtmp include directory
# RTMP_LIBRARIES - The librtmp libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_RTMP librtmp QUIET)
endif()

find_path(RTMP_INCLUDE_DIR librtmp/rtmp.h PATHS ${PC_RTMP_INCLUDEDIR})
find_library(RTMP_LIBRARY rtmp librtmp PATHS ${PC_RTMP_LIBDIR})

set(RTMP_VERSION ${PC_RTMP_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RTMP
                                  REQUIRED_VARS RTMP_INCLUDE_DIR RTMP_LIBRARY
                                  VERSION_VAR RTMP_VERSION)

if(RTMP_FOUND)
  set(RTMP_LIBRARIES ${RTMP_LIBRARY})
  set(RTMP_INCLUDE_DIRS ${RTMP_INCLUDE_DIR})
endif()


mark_as_advanced(RTMP_INCLUDE_DIRS RTMP_LIBRARIES)
