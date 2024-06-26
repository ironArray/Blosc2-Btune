##############################################################################
# Btune for Blosc2 - Automatically choose the best codec/filter for your data
#
# Copyright (c) 2023-present  Blosc Development Team <blosc@blosc.org>
# https://btune.blosc.org
# Copyright (c) 2023-present  ironArray SLU <contact@ironarray.io>
# https://ironarray.io
# License: GNU Affero General Public License v3.0
# See LICENSE.txt for details about copyright and rights to use.
##############################################################################

import blosc2
import argparse
import numpy as np
from PIL import Image
import os
import blosc2_btune



if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Compress the given input image using Blosc2 and grok',
    )
    add_argument = parser.add_argument
    add_argument('inputfile')
    add_argument('-o', '--outputfile', type=str, help='File name from decompressed image')
    args = parser.parse_args()
    im = Image.open(args.inputfile)

    # Set Btune parameters
    base_dir = os.path.dirname(__file__)
    kwargs = {
        "tradeoff": (0.5, 0.3, 0.2),  # (cratio tradeoff, speed tradeoff, quality tradeoff)
        "perf_mode": blosc2_btune.PerformanceMode.COMP,
        "models_dir": f"{base_dir}/models/"}
    blosc2_btune.set_params_defaults(**kwargs)

    # Convert the image to a numpy array
    np_array = np.asarray(im)

    cparams = {'tuner': blosc2.Tuner.BTUNE}
    nchunks = 10

    # Create a ndarray made out of the same image `nchunks` times
    chunks = blocks = [1] + list(np_array.shape)
    bl_array = blosc2.uninit(
        shape=[nchunks] + list(np_array.shape),
        dtype=np_array.dtype,
        chunks=chunks,
        blocks=blocks,
        mode="w",
        cparams=cparams,
    )
    for i in range(nchunks):
        bl_array[i,...] = np_array
