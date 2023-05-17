#!/usr/bin/env bash

# This should be executed in the root directory of the project!

git clone --depth=1 -b v2.11.0 https://github.com/tensorflow/tensorflow.git tensorflow_src
# v2.12.0 does not seem to work on Linux (and static compiling)
# git clone --depth=1 -b v2.12.0 https://github.com/tensorflow/tensorflow.git tensorflow_src
git clone --depth=1 https://github.com/Blosc/c-blosc2.git c-blosc2
mkdir c-blosc2/build

