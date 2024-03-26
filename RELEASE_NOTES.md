Release notes for Btune
=======================

Changes from 1.2.0 to 1.2.1
===========================

#XXX version-specific blurb XXX#


Changes from 1.1.2 to 1.2.0
===========================

* New quality mode. This mode can be activated by using a tradeoff of 3 values:
  (compression ratio, speed, quality) instead of just one value for lossless
  compression (tradeoff). This mode is only supported for integer datasets
  (mainly RGB and gray-scale images).  For more info on how this works, take a look at
  https://github.com/Blosc/Blosc2-Btune?tab=readme-ov-file#btune-quality-mode.

* Removed shuffle size from the tweaking parameters since it is no longer
  supported in C-Blosc2.

* Removed block size from the tracing list since it is not changed by btune.


Changes from 1.1.1 to 1.1.2
===========================

* Slight variations in the way to decide the winner in the genetic
  algorithm.  This is to avoid that the same winner is always selected
  when there is no improvement in the cratio (it can be speed too).

*  Make exploration of threads a bit bolder.  Although due to the overhead
   in changing the threads, this is not very useful in practice.

* Express S.Speed in GB/s.  This is more intuitive than previous figure.


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
