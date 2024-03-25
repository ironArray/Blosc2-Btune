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
import numpy as np

VERSION = "1.2.0"


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
    'models_dir': "",
    # behaviour
    'nwaits': 0,
    'nsofts': 5,
    'nhards': 10,
    # behaviour.repeat_mode
    'repeat_mode': RepeatMode.STOP
}


libpath = get_libpath()
lib = ctypes.cdll.LoadLibrary(libpath)


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
    # Insert the number of tradeoff values
    if isinstance(args[2], (float, int)):
        args.insert(3, 1)
        args[2] = np.array(args[2], dtype=np.float32)
    else:
        args.insert(3, 3)
        args[2] = np.array(args[2], dtype=np.float32)
    # args[2] = ctypes.c_float(args[2])
    args[6] = args[6].encode('utf-8')
    # Get value of enums
    args[1] = args[1].value
    args[-1] = args[-1].value

    lib.set_params_defaults.argtypes = [ctypes.c_uint] * 2 + [np.ctypeslib.ndpointer(dtype=np.float32)] + \
                                        [ctypes.c_int, ctypes.c_bool, ctypes.c_int, ctypes.c_char_p] +  \
                                        [ctypes.c_uint] * 4

    lib.set_params_defaults(*args)


class ReuseModels:
    def __enter__(self):
        lib.btune_set_reuse_models(True)


    def __exit__(self, exc_type, exc_value, exc_tb):
        lib.btune_free_all_models()
        lib.btune_set_reuse_models(False)


if __name__ == "__main__":
    print_libpath()
