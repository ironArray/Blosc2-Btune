Release notes for Btune
=======================

Changes from 1.1.1 to 1.1.2
===========================

#XXX version-specific blurb XXX#


Changes from 1.1.0 to 1.1.1
===========================

* Disable printing of Blocksize and Shufflesize in `BTUNE_TRACE` header.
  This is because these values are not used in the tweaking process anymore.

* "Score" is now called "S.Score" (Speed Score) in the `BTUNE_TRACE` header.
  This is the inverse of the previous score. It provides a better
  interpretation (larger figures are better scores), and should read better.

* Provide some room for exploring a number of threads beyond usual limits.


Changes from 1.0.3 to 1.1.0
===========================

* Added both splitmode to inference, so no tweaking is
  needed when using the models.

* Fixed a bug in blosc2_btune.set_params_defaults when not 
  passing `models_dir` arg.

* Performance optimizations such as computing only once
  the zeros speed needed to perform the inference 
  and reusing the already loaded models so that time and memory
  are saved.


Changes from 1.0.2 to 1.0.3
===========================

* Support for Windows wheels is here.

* Wheels are not dependent on the Python version ABI anymore.
  This is because Btune is essentially a C++ library, with no
  calls to Python (or extension) ABI at all.

* There is support now for passing btune config through Python.
  See new example at ``examples/btune_config.py``.

* Embedded tensorflow lite bumped to 2.14.0.


Changes from 1.0.1 to 1.0.2
===========================

* Call blosc2_init() during Btune initialization to get
  access to user defined codecs and filters.

* Updated to use latest C-Blosc2 sources.


Changes from 1.0.0 to 1.0.1
===========================

* New binary wheels for Mac arm64

* Fixed the license (full AGPLv3 now)

* Fix `BTUNE_TRACE` header printing

* Do not change clevel when not tweaking & new models


Changes from 1.0.0-rc.2 to 1.0.0 (final)
========================================

* Rename `balance` to `tradeoff`.

* Perform inference for all chunks by default.

* Compile package as module instead of shared.

* Fix bug regarding float comparison when tradeoff > 1/3

* Use default tradeoff value if invalid value passed.

* Move all example data to the new examples directory and improve it.
