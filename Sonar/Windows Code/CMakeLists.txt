cmake_minimum_required(VERSION 3.8)
project(RecordData)

set(CMAKE_CXX_STANDARD 11)

# Project version number
set(RECORDDATA_VERSION_MAJOR 1)
set(RECORDDATA_VERSION_MINOR 0)
set(RECORDDATA_VERSION_PATCH 0)
set(RECORDDATA_VERSION_NAME "\"Boggy Blitz\"")

# configure a header file to pass some of the CMake settings to the source code
message(STATUS "Generating RecordDataConfig.h")
configure_file(
  "${PROJECT_SOURCE_DIR}/RecordDataConfig.h.in"
  "${PROJECT_BINARY_DIR}/RecordDataConfig.h"
  )

# add the executable
add_executable(RecordData RecordData.cpp pch.cpp)
# This command isn't needed because we are't adding our own additional target_link_libraries
#  to the final executable
#target_link_libraries(RecordData ${PROJECT_LIBS})

# add the binary tree to the search path for include files
# so that we will find RecordDataConfig.h
target_include_directories(RecordData PUBLIC
                           "${PROJECT_BINARY_DIR}"
)
