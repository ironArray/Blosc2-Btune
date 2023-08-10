#######################################################################
# Copyright (c) 2019-present, Blosc Development Team <blosc@blosc.org>
# All rights reserved.
#
# This source code is licensed under a BSD-style license (found in the
# LICENSE file in the root directory of this source tree)
#######################################################################

import numpy as np
import blosc2


urlpath = "rand_int.b2nd"

# Create the SChunk
rng = np.random.default_rng()
# a = rng.integers(low=0, high=10000, size=int(5e7), dtype=np.int64) # For generating the models
a = rng.integers(low=0, high=10000, size=int(5e4), dtype=np.int64)
ba = blosc2.asarray(a, urlpath=urlpath, mode="w", chunks=(5e3,))
print(ba.info)
print("NDArray succesfully created!")
# blosc2.remove_urlpath(urlpath)
