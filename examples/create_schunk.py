#######################################################################
# Copyright (c) 2019-present, Blosc Development Team <blosc@blosc.org>
# All rights reserved.
#
# This source code is licensed under a BSD-style license (found in the
# LICENSE file in the root directory of this source tree)
#######################################################################

import numpy as np

import blosc2

nchunks = 10
chunknitems = 200 * 1000
# Set the compression and decompression parameters
cparams = { "typesize": 4}
dparams = {}
contiguous = True
urlpath = "linspace.b2frame"

storage = {"contiguous": contiguous, "urlpath": urlpath, "cparams": cparams, "dparams": dparams}
blosc2.remove_urlpath(urlpath)

# Create the SChunk
chunk = np.linspace(0, 10, chunknitems)
schunk = blosc2.SChunk(chunksize=chunknitems * 4, **storage)
for i in range(nchunks):
    schunk[i * chunknitems: (i + 1) * chunknitems] = chunk

print("SChunk succesfully created!")
# blosc2.remove_urlpath(urlpath)
