##############################################################################
# Btune for Blosc2 - Automatically choose the best codec/filter for your data
#
# Copyright (c) 2023 The Blosc Developers <blosc@blosc.org>
# https://btune.blosc.org
# License: GNU Affero General Public License v3.0
# See LICENSE.txt for details about copyright and rights to use.
##############################################################################

import os
import platform
from pathlib import Path
from enum import Enum
import ctypes

VERSION = "1.0.4.dev"


class RepeatMode(Enum):
    """
    Available repeat modes.
    """

    STOP = 0
    SOFT = 1
    ALL = 2


class PerformanceMode(Enum):
    """
    Available performance modes.
    """

    COMP = 0
    DECOMP = 1
    BALANCED = 2
    AUTO = 3


def get_libpath():
    system = platform.system()
    if system == "Linux":
        libname = "libblosc2_btune.so"
    elif system == "Darwin":
        libname = "libblosc2_btune.so"
    elif system == "Windows":
        libname = "libblosc2_btune.dll"
    else:
        raise RuntimeError("Unsupported system: ", system)
    return os.path.abspath(Path(__file__).parent / libname)


def print_libpath():
    print(get_libpath(), end="")


params_defaults = {
    'bandwidth': 2 * 10 * 1024 ** 2,
    'perf_mode': PerformanceMode.AUTO,
    'tradeoff': 0.5,
    'cparams_hint': False,
    'use_inference': -1,
    'models_dir': None,
    # behaviour
    'nwaits': 0,
    'nsofts': 5,
    'nhards': 10,
    # behaviour.repeat_mode
    'repeat_mode': RepeatMode.STOP
}


def set_params_defaults(**kwargs):
    # Check arguments
    not_supported = [k for k in kwargs.keys() if k not in params_defaults]
    if not_supported:
        raise ValueError(f"The next params are not supported: {not_supported}")
    # Prepare arguments
    params = params_defaults.copy()
    params.update(kwargs)
    args = params.values()
    args = list(args)
    args[2] = ctypes.c_float(args[2])
    args[5] = args[5].encode('utf-8')
    # Get value of enums
    args[1] = args[1].value
    args[-1] = args[-1].value

    libpath = get_libpath()
    lib = ctypes.cdll.LoadLibrary(libpath)
    lib.set_params_defaults(*args)


class ReuseModels:
    def __enter__(self):
        libpath = get_libpath()
        lib = ctypes.cdll.LoadLibrary(libpath)
        lib.set_reuse_models(True)


    def __exit__(self, exc_type, exc_value, exc_tb):
        libpath = get_libpath()
        lib = ctypes.cdll.LoadLibrary(libpath)
        lib.btune_g_models_free()


if __name__ == "__main__":
    print_libpath()
