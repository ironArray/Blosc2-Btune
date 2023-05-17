#!/usr/bin/env bash

# This should be executed in the root directory of the project!

# Checkout TensorFlow sources
# v2.11.0 works both on Linux and Mac
# v2.12.0 does not seem to work on neither Linux nor Mac (and static compiling)
# v2.13.0-rc0 does seems to work again on both platforms
TENSORFLOW_VERSION="v2.13.0-rc0"
if [ ! -d "tensorflow_src" ]
then
  git clone --depth=1 -b $TENSORFLOW_VERSION https://github.com/tensorflow/tensorflow.git tensorflow_src
else
  echo "TensorFlow ($TENSORFLOW_VERSION) already cloned"
fi

# Checkout C-Blosc2 sources
BLOSC2_VERSION="main"
if [ ! -d "c-blosc2" ]
then
  git clone --depth=1 -b $BLOSC2_VERSION https://github.com/Blosc/c-blosc2.git c-blosc2
fi
if [ -d "c-blosc2/build" ]
then
  echo "C-Blosc2 ($BLOSC2_VERSION) already cloned"
  rm -rf c-blosc2/build/*
else
  mkdir c-blosc2/build
fi
