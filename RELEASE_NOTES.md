Release notes for Btune
=======================

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
