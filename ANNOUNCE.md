# Announcing Btune 1.1.2

## What is new?

This is a maintenance release where slight variations in the way to
decide the winner in the genetic algorithm have been introduced.
This is to avoid that the same winner is always selected
when there is no improvement in the cratio (it can be speed too).
Also, express S.Speed in GB/s.  This is more intuitive than previous figure.

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
