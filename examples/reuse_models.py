#######################################################################
# Copyright (c) 2019-present, Blosc Development Team <blosc@blosc.org>
# All rights reserved.
#
# This source code is licensed under a BSD-style license (found in the
# LICENSE file in the root directory of this source tree)
#######################################################################
import blosc2
from time import time
import blosc2_btune
import numpy as np

# Create the data
rng = np.random.default_rng()
a = rng.integers(low=0, high=10000, size=int(1e5), dtype=np.int64)
nchunks = 10
chunk_nitems = a.size // nchunks

# Set some parameters
cparams = {
        "tuner": blosc2.Tuner.BTUNE,
        }
kwargs = {
        "perf_mode": blosc2_btune.PerformanceMode.DECOMP,
        "models_dir": "./models/",
        }
blosc2_btune.set_params_defaults(**kwargs)

print("Creating arrays reloading models each time")
tl = 0
for nchunk in range(0, nchunks):
    tref = time()
    b = blosc2.asarray(a[nchunk * chunk_nitems:(nchunk + 1) * chunk_nitems], cparams=cparams)
    tl += time() - tref

print("Creating arrays reusing loaded models")
tr = 0
with blosc2_btune.ReuseModels():
    for nchunk in range(0, nchunks):
        tref = time()
        b = blosc2.asarray(a[nchunk * chunk_nitems:(nchunk + 1) * chunk_nitems], cparams=cparams)
        tr += time() - tref

size = a.size * 8 / 10**6
print(f"Reloading time: {tl:.3f}s ({size / tl:.3f} MB/s)")
print(f"Reusing time: {tr:.3f}s ({size / tr:.3f} MB/s)")
