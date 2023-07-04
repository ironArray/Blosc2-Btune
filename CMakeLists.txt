# Blosc - Blocked Shuffling and Compression Library
#
# Copyright (C) 2021  Blosc Developement Team <blosc@blosc.org>
# https://blosc.org
# License: BSD 3-Clause (see LICENSE.txt)
#
# See LICENSE.txt for details about copyright and rights to use.

cmake_minimum_required(VERSION 3.20)
project(blosc2_btune LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD 20)

# Only linking tensorflow statically is officially supported at this time
option(BUILD_STATIC_TFLITE "Link tflite statically" ON)

cmake_path(SET TENSORFLOW_SRC_DIR NORMALIZE "${CMAKE_SOURCE_DIR}/tensorflow_src")
cmake_path(ABSOLUTE_PATH TENSORFLOW_SRC_DIR NORMALIZE)
if(NOT EXISTS ${TENSORFLOW_SRC_DIR})
    message( SEND_ERROR "Call prebuild.sh first" )
endif()
message("TENSORFLOW dir: " ${TENSORFLOW_SRC_DIR})

cmake_path(SET BLOSC2_SRC_DIR NORMALIZE "${CMAKE_SOURCE_DIR}/c-blosc2")
cmake_path(ABSOLUTE_PATH BLOSC2_SRC_DIR NORMALIZE)
if(NOT EXISTS ${BLOSC2_SRC_DIR})
    message( SEND_ERROR "Call prebuild.sh first" )
endif()
message("BLOSC2 dir: " ${BLOSC2_SRC_DIR})

message("Building Blosc2 plugin btune ...")

cmake_path(SET BLOSC2_INCLUDE_DIR NORMALIZE "${BLOSC2_SRC_DIR}/include")
# Find blosc2.h (sanity check)
cmake_path(SET BLOSC2_INCLUDE NORMALIZE "${BLOSC2_INCLUDE_DIR}/blosc2.h")
cmake_path(HAS_FILENAME BLOSC2_INCLUDE HAS_BLOSC2)
if(HAS_BLOSC2)
    message(STATUS "Found Blosc2 include: ${BLOSC2_INCLUDE}")
else()
    message(FATAL_ERROR "No Blosc2 includes found.  Aborting.")
endif()

add_subdirectory(src)