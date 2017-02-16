# - Try to find rtmp
# Once done this will define
#
# RTMP_FOUND - system has librtmp
# RTMP_INCLUDE_DIRS - the librtmp include directory
# RTMP_LIBRARIES - The librtmp libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules (PC_RTMP librtmp QUIET)
endif()

find_path(RTMP_INCLUDE_DIR librtmp/rtmp.h PATHS ${PC_RTMP_INCLUDEDIR})
find_library(RTMP_LIBRARY rtmp librtmp PATHS ${PC_RTMP_LIBDIR})

if(PC_RTMP_FOUND)
  if(RTMP_LIBRARY)
    set(RTMP_LIBRARIES ${PC_RTMP_LIBRARIES})
  else()
    unset(RTMP_LIBRARIES)
  endif()
  if(RTMP_INCLUDE_DIR)
    set(RTMP_INCLUDE_DIRS ${RTMP_INCLUDE_DIR} ${PC_RTMP_INCLUDE_DIRS})
  endif()
else()
  set(RTMP_LIBRARIES ${RTMP_LIBRARY})
  set(RTMP_INCLUDE_DIRS ${RTMP_INCLUDE_DIR})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RTMP DEFAULT_MSG RTMP_INCLUDE_DIRS RTMP_LIBRARIES)

mark_as_advanced(RTMP_INCLUDE_DIRS RTMP_LIBRARIES)
