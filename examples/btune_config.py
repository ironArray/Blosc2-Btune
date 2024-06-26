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


import numpy as np
import blosc2
import blosc2_btune
import os


# Create the SChunk
rng = np.random.default_rng()
a = rng.integers(low=0, high=10000, size=int(1e5), dtype=np.int64)

# Set Btune parameters
base_dir = os.path.dirname(__file__)
kwargs = {
    "tradeoff": 0.3,
    "perf_mode": blosc2_btune.PerformanceMode.DECOMP,
    "models_dir": f"{base_dir}/models/"}
blosc2_btune.set_params_defaults(**kwargs)

# Tell blosc2 to use btune by setting the corresponding tuner in the cparams
urlpath = "btune_config.b2nd"
_ = blosc2.asarray(a, urlpath=urlpath, mode="w", chunks=(5e3,),
                   cparams={"tuner": blosc2.Tuner.BTUNE})

print(f"NDArray succesfully created in {urlpath}")
