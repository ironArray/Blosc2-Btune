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

message("Building Blosc2 plugin btune ...")

# Find blosc2.h
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
message(STATUS "Executable: ${Python_EXECUTABLE}")

if (UNIX)
    cmake_path(SET Python_LIB NORMALIZE "${Python_STDLIB}/..")
else()
    cmake_path(SET Python_LIB NORMALIZE "${Python_STDLIB}")
endif()

cmake_path(SET Python_ROOT NORMALIZE "${Python_LIB}/..")
message(STATUS "Inferred Python root: ${Python_ROOT}")
cmake_path(SET Python_LIB64 NORMALIZE "${Python_ROOT}/lib64")

cmake_path(SET Python_INCLUDE NORMALIZE "${Python_ROOT}/include")
message(STATUS "Inferred Python include: ${Python_INCLUDE}")
message(STATUS "Inferred Python lib: ${Python_LIB}")
message(STATUS "Inferred Python lib64: ${Python_LIB64}")

add_subdirectory(src)
