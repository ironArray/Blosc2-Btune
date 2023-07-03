# Blosc2 Btune

For using Btune you will first have to create and install its wheel.

**Note:** Remove flatbuffers package in your system (if installed).
This could be incompatible wit the required version in tensorflow.
Also, tensorflow comes with its own version, so this is not needed.

## Create the wheel

For Linux:

```shell
CIBW_BEFORE_BUILD="bash prebuild.sh" python -m cibuildwheel --only 'cp311-manylinux_x86_64'
```

Please note that the prebuild.sh will be executed from inside the docker
(handled by CIBuild).

For Mac:

```shell
CIBW_BEFORE_BUILD="bash prebuild.sh" python -m cibuildwheel --only 'cp311-macosx_x86_64'
```

## Install the wheel

```shell
pip install wheelhouse/blosc2_btune-*.whl --force-reinstall
```

## Compile and run example

For Linux and Mac:

```shell
cd examples  # avoid staying in the main package directory
gcc -o btune_example btune_example.c -lblosc2 -lm
./btune_example .../pressure.b2nd pressure-btune.b2nd
Compression ratio: 3456.0 MB -> 662.0 MB (5.2x)
Compression time: 17.3 s, 199.8 MB/s
```

You can use `BTUNE_TRACE=1`, `BTUNE_TRADEOFF=X` and `BTUNE_MODELS_DIR` to see how Btune is doing.

```shell
BTUNE_TRADEOFF=0.1 BTUNE_TRACE=1 BTUNE_MODELS_DIR=./models ./btune_example .../pressure.b2nd pressure-btune.b2nd
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Btune version: 1.0.0.
Perfomance Mode: BALANCED, Compression Mode: BALANCED, Bandwidth: 20 GB/s.
Behaviour: Waits - 0, Softs - 5, Hards - 1, Repeat Mode - STOP.
|    Codec   | Filter | Split | C.Level | Blocksize | Shufflesize | C.Threads | D.Threads |   Score   |  C.Ratio   |   Btune State   | Readapt | Winner
|        lz4 |      0 |     1 |       9 |         8 |           8 |         1 |         1 |    0.0066 |      2.19x |    CODEC_FILTER |    HARD | W
|        lz4 |      0 |     0 |       9 |         8 |           8 |         1 |         1 |   0.00594 |      2.61x |    CODEC_FILTER |    HARD | W
|        lz4 |      1 |     1 |       9 |         8 |           8 |         1 |         1 |   0.00387 |      3.65x |    CODEC_FILTER |    HARD | W
|        lz4 |      1 |     0 |       9 |         8 |           8 |         1 |         1 |   0.00466 |      3.81x |    CODEC_FILTER |    HARD | -
|        lz4 |      2 |     1 |       9 |         8 |           8 |         1 |         1 |   0.00293 |      4.85x |    CODEC_FILTER |    HARD | W
|        lz4 |      2 |     0 |       9 |         8 |           8 |         1 |         1 |   0.00259 |      5.16x |    CODEC_FILTER |    HARD | W
|    blosclz |      0 |     1 |       9 |         8 |           8 |         1 |         1 |    0.0196 |      2.68x |    CODEC_FILTER |    HARD | -
|    blosclz |      0 |     0 |       9 |         8 |           8 |         1 |         1 |    0.0112 |      3.13x |    CODEC_FILTER |    HARD | -
|    blosclz |      1 |     1 |       9 |         8 |           8 |         1 |         1 |    0.0106 |       3.3x |    CODEC_FILTER |    HARD | -
|    blosclz |      1 |     0 |       9 |         8 |           8 |         1 |         1 |    0.0119 |      3.49x |    CODEC_FILTER |    HARD | -
|    blosclz |      2 |     1 |       9 |         8 |           8 |         1 |         1 |   0.00948 |      4.33x |    CODEC_FILTER |    HARD | -
|    blosclz |      2 |     0 |       9 |         8 |           8 |         1 |         1 |   0.00225 |      18.2x |    CODEC_FILTER |    HARD | W
|    blosclz |      2 |     0 |       7 |         8 |           8 |         1 |         1 |   0.00612 |      4.44x |          CLEVEL |    HARD | -
|    blosclz |      2 |     0 |       8 |         8 |           8 |         1 |         1 |   0.00628 |      4.42x |          CLEVEL |    SOFT | -
|    blosclz |      2 |     0 |       8 |         8 |           8 |         1 |         1 |   0.00579 |      4.28x |          CLEVEL |    SOFT | -
|    blosclz |      2 |     0 |       8 |         8 |           8 |         1 |         1 |   0.00579 |      4.13x |          CLEVEL |    SOFT | -
|    blosclz |      2 |     0 |       8 |         8 |           8 |         1 |         1 |   0.00619 |         4x |          CLEVEL |    SOFT | -
|    blosclz |      2 |     0 |       8 |         8 |           8 |         1 |         1 |   0.00701 |      4.08x |          CLEVEL |    SOFT | -
|        lz4 |      0 |     1 |       9 |         8 |           8 |         1 |         1 |   0.00787 |      1.45x |    CODEC_FILTER |    HARD | -
|        lz4 |      0 |     0 |       9 |         8 |           8 |         1 |         1 |   0.00657 |      1.51x |    CODEC_FILTER |    HARD | -
|        lz4 |      1 |     1 |       9 |         8 |           8 |         1 |         1 |   0.00283 |      2.61x |    CODEC_FILTER |    HARD | -
|        lz4 |      1 |     0 |       9 |         8 |           8 |         1 |         1 |   0.00338 |      2.76x |    CODEC_FILTER |    HARD | -
|        lz4 |      2 |     1 |       9 |         8 |           8 |         1 |         1 |    0.0024 |      4.14x |    CODEC_FILTER |    HARD | -
|        lz4 |      2 |     0 |       9 |         8 |           8 |         1 |         1 |   0.00101 |      19.9x |    CODEC_FILTER |    HARD | W
|    blosclz |      0 |     1 |       9 |         8 |           8 |         1 |         1 |    0.0223 |      1.57x |    CODEC_FILTER |    HARD | -
|    blosclz |      0 |     0 |       9 |         8 |           8 |         1 |         1 |    0.0138 |      1.95x |    CODEC_FILTER |    HARD | -
|    blosclz |      1 |     1 |       9 |         8 |           8 |         1 |         1 |   0.00919 |      2.54x |    CODEC_FILTER |    HARD | -
|    blosclz |      1 |     0 |       9 |         8 |           8 |         1 |         1 |    0.0111 |       2.5x |    CODEC_FILTER |    HARD | -
|    blosclz |      2 |     1 |       9 |         8 |           8 |         1 |         1 |    0.0087 |      3.97x |    CODEC_FILTER |    HARD | -
|    blosclz |      2 |     0 |       9 |         8 |           8 |         1 |         1 |   0.00546 |      5.22x |    CODEC_FILTER |    HARD | -
|        lz4 |      2 |     0 |       9 |         8 |           8 |         1 |         1 |   0.00224 |      4.98x |          CLEVEL |    HARD | -
|        lz4 |      2 |     0 |       8 |         8 |           8 |         1 |         1 |    0.0025 |      3.83x |          CLEVEL |    HARD | -
Compression ratio: 3456.0 MB -> 662.0 MB (5.2x)
Compression time: 17.3 s, 199.8 MB/s
```

That's all folks!
