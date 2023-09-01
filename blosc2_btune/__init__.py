##############################################################################
# Btune for Blosc2 - Automatically choose the best codec/filter for your data
#
# Copyright (c) 2023 The Blosc Developers <blosc@blosc.org>
# https://btune.blosc.org
# License: GNU Affero General Public License v3.0
# See LICENSE.txt for details about copyright and rights to use.
##############################################################################

VERSION = "1.0.1"

import os
import platform
from pathlib import Path

def print_libpath():
    system = platform.system()
    if system == "Linux":
        libname = "libblosc2_btune.so"
    elif system == "Darwin":
        libname = "libblosc2_btune.so"
    elif system == "Windows":
        libname = "libblosc2_btune.dll"
    else:
        raise RuntimeError("Unsupported system: ", system)
    libpath = os.path.abspath(Path(__file__).parent / libname)
    print(libpath, end="")


if __name__ == "__main__":
    print_libpath()
