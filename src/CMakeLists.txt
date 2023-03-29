# Blosc - Blocked Shuffling and Compression Library
#
# Copyright (C) 2021  The Blosc Developers <blosc@blosc.org>
# https://blosc.org
# License: BSD 3-Clause (see LICENSE.txt)
#
# See LICENSE.txt for details about copyright and rights to use.

include_directories(
    ${BLOSC2_INCLUDE_DIR}
    /usr/local/include/
    ${TENSORFLOW_SRC_DIR}
)

add_library(btune SHARED btune.c btune_model.cpp json.c
            entropy_probe.c)

target_link_directories(btune
    PUBLIC ${Python_LIB}
    PUBLIC ${BLOSC2_INCLUDE_DIR}
    PUBLIC ${TENSORFLOW_SRC_DIR}/bazel-out/darwin-opt/bin/tensorflow/lite
)
target_link_libraries(btune blosc2 tensorflowlite)

link_directories(${Python_LIB})
include_directories(${Python_INCLUDE})

add_executable("btune_example" "btune_example.c")
target_link_libraries("btune_example" btune blosc2)

install(TARGETS btune LIBRARY DESTINATION btune)
