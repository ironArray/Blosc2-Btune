##############################################################################
# Btune for Blosc2 - Automatically choose the best codec/filter for your data
#
# Copyright (c) 2023 The Blosc Developers <blosc@blosc.org>
# https://btune.blosc.org
# License: GNU Affero General Public License v3.0
# See LICENSE.txt for details about copyright and rights to use.
##############################################################################

cmake_minimum_required(VERSION 3.20)
project(blosc2_btune LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD 17)

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
