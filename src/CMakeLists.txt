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

add_library(blosc2_btune MODULE btune.c btune_model.cpp json.c entropy_probe.c)

target_link_directories(blosc2_btune
    PUBLIC ${BLOSC2_SRC_DIR}/build/blosc
)
target_link_directories(blosc2_btune PUBLIC ${BLOSC2_SRC_DIR}/build/blosc)

if (BUILD_STATIC_TFLITE)
    # This only works in Linux and Mac (at least for v2.11.0)
    add_subdirectory(
        "${TENSORFLOW_SRC_DIR}/tensorflow/lite"
        "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite"
        EXCLUDE_FROM_ALL
    )
    target_link_libraries(blosc2_btune blosc2 tensorflow-lite)
else()
    # This is meant for using bazel from outside
    if(APPLE)
        target_link_directories(blosc2_btune
                PUBLIC ${TENSORFLOW_SRC_DIR}/bazel-out/darwin-opt/bin/tensorflow/lite
                )
    else()
        target_link_directories(blosc2_btune
                PUBLIC ${TENSORFLOW_SRC_DIR}/bazel-out/k8-opt/bin/tensorflow/lite
                )
    endif()

    target_link_libraries(blosc2_btune blosc2 tensorflowlite)
endif()

# Add btune.h to wheel
install(FILES btune.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT DEV)


install(TARGETS blosc2_btune LIBRARY DESTINATION blosc2_btune)