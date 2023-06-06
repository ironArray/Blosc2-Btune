# Blosc - Blocked Shuffling and Compression Library
#
# Copyright (C) 2021  The Blosc Developers <blosc@blosc.org>
# https://blosc.org
# License: BSD 3-Clause (see LICENSE.txt)
#
# See LICENSE.txt for details about copyright and rights to use.

VERSION = "1.0.0-rc.2"

import os
import platform
from pathlib import Path

def print_libpath():
    system = platform.system()
    if system == "Linux":
        libname = "libblosc2_btune.so"
    elif system == "Darwin":
        libname = "libblosc2_btune.dylib"
    elif system == "Windows":
        libname = "libblosc2_btune.dll"
    else:
        raise RuntimeError("Unsupported system: ", system)
    libpath = os.path.abspath(Path(__file__).parent / libname)
    print(libpath, end="")


if __name__ == "__main__":
    print_libpath()
