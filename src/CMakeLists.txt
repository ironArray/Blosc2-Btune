# Blosc - Blocked Shuffling and Compression Library
#
# Copyright (C) 2021  The Blosc Developers <blosc@blosc.org>
# https://blosc.org
# License: BSD 3-Clause (see LICENSE.txt)
#
# See LICENSE.txt for details about copyright and rights to use.

include_directories(
    ${BLOSC_SRC_DIR}/blosc
    ${BLOSC_SRC_DIR}/include
    /usr/local/include/
    ${TENSORFLOW_SRC_DIR}
)

add_library(btune SHARED btune.c btune_model.cpp json.c
            blosc2_entropy_prober.c)
target_link_directories(btune
    PUBLIC ${BLOSC_SRC_DIR}/build/blosc
    PUBLIC ${TENSORFLOW_SRC_DIR}/bazel-out/darwin-opt/bin/tensorflow/lite
)
target_link_libraries(btune blosc2 tensorflowlite)

link_directories(${Python_LIB})
include_directories(${Python_INCLUDE})

add_executable(btune_example btune_example.c)
target_link_libraries(btune_example btune)
