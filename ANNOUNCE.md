# Announcing Btune 1.0.3

A Blosc2 plugin to assist you on which Blosc2 compression parameters to use, based on data compression patterns.

## What is new?

This is a maintenance release where added the capability to pass parameters to Btune programmatically from Python (passing them from C was already possible); see new ``examples/btune_config.py``.  Also, Windows wheels are here (finally!).

For more info, please see the release notes in:

https://github.com/Blosc/blosc2_btune/blob/main/RELEASE_NOTES.md


## What is it?

Btune is a dynamic plugin for Blosc2 that assists in finding the optimal combination of compression parameters. It works by training a neural network on your most representative datasets.


## Download sources

The github repository is over here:

https://github.com/Blosc/blosc2_btune

Blosc is distributed using the GNU Affero General Public License,
see LICENSE.txt for details.

## Mailing list

There is an official Blosc mailing list at:

blosc@googlegroups.com
https://groups.google.com/g/blosc

## Tweeter feed

Please follow @Blosc2 to get informed about the latest developments.


Enjoy Data!
- The Blosc Development Team
