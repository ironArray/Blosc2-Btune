# Announcing Btune 1.2.1

## What is new?

This is a maintenance release for removing a stale
"WARNING: Empty metadata, no inference performed" message
that was printed when no inference was performed.  This is still
available when using `BTUNE_TRACE=1`, but it is not printed by
default anymore. Fixes #15.

For more info, please see the release notes in:

https://github.com/ironArray/Blosc2-Btune/blob/main/RELEASE_NOTES.md


## What is it?

Btune is a dynamic plugin for Blosc2 that assists in finding the optimal combination of compression parameters. It works by training a neural network on your most representative datasets.


## Download sources

The github repository is here:

https://github.com/ironArray/Blosc2-Btune

Btune is distributed using the GNU Affero General Public License,
see LICENSE.txt for details.


## Follow us

You can follow us on Mastodon: https://mastodon.social/@ironArray
or on Twitter: https://twitter.com/ironArray


## Contact

Please contact ironArray at: https://ironarray.io


We make compression better
- The ironArray Team
