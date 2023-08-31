##############################################################################
# Btune for Blosc2 - Automatically choose the best codec/filter for your data
#
# Copyright (c) 2023 The Blosc Developers <blosc@blosc.org>
# https://btune.blosc.org
# License: GNU Affero General Public License v3.0
# See LICENSE.txt for details about copyright and rights to use.
##############################################################################


import numpy as np
import blosc2


urlpath = "rand_int.b2nd"

# Create the SChunk
rng = np.random.default_rng()
# a = rng.integers(low=0, high=10000, size=int(5e7), dtype=np.int64) # For generating the models
a = rng.integers(low=0, high=10000, size=int(1e5), dtype=np.int64)
ba = blosc2.asarray(a, urlpath=urlpath, mode="w", chunks=(5e3,))

print("NDArray succesfully created!")
# blosc2.remove_urlpath(urlpath)
