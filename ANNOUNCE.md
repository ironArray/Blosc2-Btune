# Announcing Btune 1.2.0

## What is new?

New support for tuning compression parameters taking into account not only
a tradeoff of compression ratio and speed, but also the quality.
This mode is only supported for integer datasets (mainly RGB and gray-scale
images).

To get an idea of how this works, see:
https://github.com/Blosc/Blosc2-Btune?tab=readme-ov-file#btune-quality-mode.

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
