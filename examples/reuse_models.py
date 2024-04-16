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
from time import time
import blosc2_btune
import numpy as np

# Create the data
rng = np.random.default_rng()
a = rng.integers(low=0, high=10000, size=int(1e8), dtype=np.int64)
nchunks = 100
chunk_nitems = a.size // nchunks

# Set some parameters
cparams = {
        "tuner": blosc2.Tuner.BTUNE,
        }
kwargs = {
        "tradeoff": 0.2,
        "perf_mode": blosc2_btune.PerformanceMode.COMP,
        "models_dir": "./models/",
        }
blosc2_btune.set_params_defaults(**kwargs)

# Create a small array to evict counting the tflite init time
b = blosc2.asarray(a[:100], cparams=cparams)

print("Creating arrays reusing loaded models")
tr = 0
with blosc2_btune.ReuseModels():
    for nchunk in range(0, nchunks):
        tref = time()
        b = blosc2.asarray(a[nchunk * chunk_nitems:(nchunk + 1) * chunk_nitems], chunks=(chunk_nitems,), blocks=(chunk_nitems//10,), cparams=cparams)
        tr += time() - tref

print("Creating arrays reloading models each time")
tl = 0
for nchunk in range(0, nchunks):
    tref = time()
    b = blosc2.asarray(a[nchunk * chunk_nitems:(nchunk + 1) * chunk_nitems], chunks=(chunk_nitems,), blocks=(chunk_nitems//10,),cparams=cparams)
    tl += time() - tref


size = a.size * 8 / 10**9
print(f"Reusing time: {tr:.3f}s ({size / tr:.3f} GB/s)")
print(f"Reloading time: {tl:.3f}s ({size / tl:.3f} GB/s)")
