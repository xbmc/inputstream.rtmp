# RTMPDump 2.6: the official release is distributed as a Git tag.
# Fetch the tag explicitly: this server rejects Kodi's generic shallow clone.
find_package(Git REQUIRED)
include(FetchContent)
set(RTMPDUMP_REVISION 138fdb258d9fc26f1843fd1b891180416c9dc575)
set(RTMPDUMP_PATCHES
    "${CMAKE_CURRENT_LIST_DIR}/patches/0001-openssl-dh-key-generation.patch")
if(WIN32)
  list(APPEND RTMPDUMP_PATCHES
       "${CMAKE_CURRENT_SOURCE_DIR}/patches/0002-msvc-stdio.patch"
       "${CMAKE_CURRENT_SOURCE_DIR}/patches/0003-uwp-cache-and-clock.patch")
endif()
FetchContent_Declare(rtmpdump
  DOWNLOAD_COMMAND "${GIT_EXECUTABLE}" clone --branch v2.6 --single-branch
                   --config core.autocrlf=false
                   https://git.ffmpeg.org/rtmpdump.git <SOURCE_DIR>
  UPDATE_COMMAND ""
  PATCH_COMMAND "${GIT_EXECUTABLE}" -C <SOURCE_DIR> checkout --detach ${RTMPDUMP_REVISION}
        COMMAND "${GIT_EXECUTABLE}" -C <SOURCE_DIR> apply ${RTMPDUMP_PATCHES})
FetchContent_MakeAvailable(rtmpdump)
execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${rtmpdump_SOURCE_DIR}" rev-parse HEAD
                OUTPUT_VARIABLE RTMPDUMP_HEAD OUTPUT_STRIP_TRAILING_WHITESPACE
                RESULT_VARIABLE RTMPDUMP_GIT_RESULT)
if(NOT RTMPDUMP_GIT_RESULT EQUAL 0 OR NOT RTMPDUMP_HEAD STREQUAL RTMPDUMP_REVISION)
  message(FATAL_ERROR "RTMPDump source does not match the verified 2.6 release")
endif()

# Hash the canonical unpatched source archive as well as pinning its Git commit.
set(RTMPDUMP_SOURCE_SHA256 0cba5d49d41b5e35a34ea8230124ed853eeeac19091d17a4188a47c31a17d352)
set(RTMPDUMP_SOURCE_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/rtmpdump-source.tar")
execute_process(COMMAND "${GIT_EXECUTABLE}" -c tar.umask=0002 -C "${rtmpdump_SOURCE_DIR}"
                        archive --format=tar ${RTMPDUMP_REVISION}
                OUTPUT_FILE "${RTMPDUMP_SOURCE_ARCHIVE}"
                RESULT_VARIABLE RTMPDUMP_ARCHIVE_RESULT)
if(NOT RTMPDUMP_ARCHIVE_RESULT EQUAL 0)
  message(FATAL_ERROR "Cannot verify RTMPDump source archive")
endif()
file(SHA256 "${RTMPDUMP_SOURCE_ARCHIVE}" RTMPDUMP_ACTUAL_SHA256)
file(REMOVE "${RTMPDUMP_SOURCE_ARCHIVE}")
if(NOT RTMPDUMP_ACTUAL_SHA256 STREQUAL RTMPDUMP_SOURCE_SHA256)
  message(FATAL_ERROR "RTMPDump source archive checksum mismatch")
endif()
