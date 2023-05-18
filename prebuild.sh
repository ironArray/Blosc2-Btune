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
  git pull
fi

# Checkout C-Blosc2 sources
BLOSC2_VERSION="main"
if [ ! -d "c-blosc2" ]
then
  git clone --depth=1 -b $BLOSC2_VERSION https://github.com/Blosc/c-blosc2.git c-blosc2
  mkdir c-blosc2/build
else
  echo "C-Blosc2 ($BLOSC2_VERSION) already cloned"
  git pull
fi

# Compile static version of C-Blosc2
cd c-blosc2/build
cmake ..
cmake --build . --target blosc2_static -j
cd -

# Still need to decide how to test the wheels
# Checkout Python-Blosc2 sources (just for testing the BTune wheel)
#PYTHON_BLOSC2_VERSION="main"
#if [ ! -d "python-blosc2" ]
#then
#  git clone --recursive --depth=1 -b $PYTHON_BLOSC2_VERSION https://github.com/Blosc/python-blosc2.git python-blosc2
#else
#  echo "Python-Blosc2 ($PYTHON_BLOSC2_VERSION) already cloned"
#  git pull
#fi
#
## Compile Python-Blosc2 extension
#cd python-blosc2
#python setup.py build_ext -i -j4
#cd -
