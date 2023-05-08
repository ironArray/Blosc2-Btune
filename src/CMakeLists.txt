# Blosc - Blocked Shuffling and Compression Library
#
# Copyright (C) 2021  The Blosc Developers <blosc@blosc.org>
# https://blosc.org
# License: BSD 3-Clause (see LICENSE.txt)
#
# See LICENSE.txt for details about copyright and rights to use.

include_directories(
    ${BLOSC2_INCLUDE_DIR}
    ${TENSORFLOW_SRC_DIR}
)
if(APPLE)
    include_directories(
        /usr/local/include/ # Needed to find flatbuffers.h
    )
endif()

add_library(blosc2_btune SHARED btune.c btune_model.cpp json.c entropy_probe.c)

target_link_directories(blosc2_btune
    PUBLIC ${Python_LIB}
    PUBLIC ${Python_LIB64}
    PUBLIC ${BLOSC2_INCLUDE_DIR}
)
if(APPLE)
    target_link_directories(blosc2_btune
        PUBLIC ${TENSORFLOW_SRC_DIR}/bazel-out/darwin-opt/bin/tensorflow/lite
    )
else()
    target_link_directories(blosc2_btune
        PUBLIC ${TENSORFLOW_SRC_DIR}/bazel-out/k8-opt/bin/tensorflow/lite
    )
endif()

if (BUILD_STATIC_TFLITE)
    add_subdirectory(
        "${TENSORFLOW_SRC_DIR}/tensorflow/lite"
        "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite"
        EXCLUDE_FROM_ALL
    )
    target_link_libraries(blosc2_btune blosc2 tensorflow-lite)
else()
    target_link_libraries(blosc2_btune blosc2 tensorflowlite)
endif()

link_directories(${Python_LIB})
include_directories(${Python_INCLUDE})

add_executable("btune_example" "btune_example.c")
target_link_libraries("btune_example" blosc2_btune blosc2)

install(TARGETS blosc2_btune LIBRARY DESTINATION blosc2_btune)
