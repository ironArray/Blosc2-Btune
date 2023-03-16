# Blosc - Blocked Shuffling and Compression Library
#
# Copyright (C) 2021  The Blosc Developers <blosc@blosc.org>
# https://blosc.org
# License: BSD 3-Clause (see LICENSE.txt)
#
# See LICENSE.txt for details about copyright and rights to use.

cmake_minimum_required(VERSION 3.20)

project(blosc2_btune_plugin LANGUAGES C CXX)

message("Building Blosc2 plugin example...")

# Find blosc2.h
find_package(Python COMPONENTS Interpreter)
if(Python_Interpreter_FOUND)
    message(STATUS "Found Python stdlib: ${Python_STDLIB}")
    cmake_path(SET Python_INCLUDE NORMALIZE "${Python_STDLIB}/../../include")
    message(STATUS "Found Python include: ${Python_INCLUDE}")
    cmake_path(SET Python_Blosc2_INCLUDE NORMALIZE "${Python_INCLUDE}/blosc2.h")
    cmake_path(HAS_FILENAME Python_Blosc2_INCLUDE HAS_BLOSC2)
    if(HAS_BLOSC2)
        set(BLOSC2_INCLUDE_DIR ${Python_INCLUDE})
        message(STATUS "Found Blosc2 include: ${Python_Blosc2_INCLUDE}")
    else()
        message(FATAL_ERROR "No Blosc2 includes found.  Aborting.")
    endif()



else()
    message(FATAL_ERROR "No Python found.  Aborting.")
endif()

add_library(blosc2_btune_plugin MODULE src/urfilters.c)
target_include_directories(blosc2_btune_plugin PUBLIC "${BLOSC2_INCLUDE_DIR}")
install(TARGETS blosc2_btune_plugin LIBRARY DESTINATION blosc2_btune_plugin)
