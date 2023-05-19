# Blosc2 BTune

For using BTune you will first have to install its wheel.

## Install the wheel

```shell
pip install blosc2_btune
```

## Using BTune from Python

To make Blosc2 use BTune from Python, you will only need to set the `BTUNE_BALANCE`
environment variable to any value between 0 (optimize only speed) and 1 (optimize only cratio).

```shell
BTUNE_BALANCE=0.5 python python-blosc2/examples/schunk.py
```

You can use `BTUNE_TRACE=1` to see what BTune is doing.

```shell
PYTHONPATH=. BTUNE_BALANCE=0.5 BTUNE_TRACE=1  python examples/schunk_roundtrip.py 
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
BTune version: 1.0.0.
Perfomance Mode: COMP, Compression balance: 0.500000, Bandwidth: 20 GB/s.
Behaviour: Waits - 0, Softs - 5, Hards - 11, Repeat Mode - STOP.
|    Codec   | Filter | Split | C.Level | Blocksize | Shufflesize | C.Threads | D.Threads |   Score   |  C.Ratio   |   BTune State   | Readapt | Winner
|       zstd |      0 |     1 |       3 |         0 |           4 |        16 |        16 |   0.00156 |      7.55x |    CODEC_FILTER |    HARD | W
|       zstd |      0 |     0 |       3 |         0 |           4 |        16 |        16 |   0.00057 |      7.85x |    CODEC_FILTER |    HARD | W
|       zstd |      1 |     1 |       3 |         0 |           4 |        16 |        16 |  0.000115 |       209x |    CODEC_FILTER |    HARD | W
|       zstd |      1 |     0 |       3 |         0 |           4 |        16 |        16 |  0.000167 |       218x |    CODEC_FILTER |    HARD | W
|       zstd |      2 |     1 |       3 |         0 |           4 |        16 |        16 |  0.000134 |       290x |    CODEC_FILTER |    HARD | W
|       zstd |      2 |     0 |       3 |         0 |           4 |        16 |        16 |  0.000239 |       199x |    CODEC_FILTER |    HARD | -
|       zlib |      0 |     1 |       3 |         0 |           4 |        16 |        16 |   0.00107 |      5.27x |    CODEC_FILTER |    HARD | -
|       zlib |      0 |     0 |       3 |         0 |           4 |        16 |        16 |   0.00133 |       5.3x |    CODEC_FILTER |    HARD | -
|       zlib |      1 |     1 |       3 |         0 |           4 |        16 |        16 |   0.00114 |       111x |    CODEC_FILTER |    HARD | -
|       zlib |      1 |     0 |       3 |         0 |           4 |        16 |        16 |  0.000902 |       101x |    CODEC_FILTER |    HARD | -
|       zlib |      2 |     1 |       3 |         0 |           4 |        16 |        16 |   0.00108 |       191x |    CODEC_FILTER |    HARD | -
|       zlib |      2 |     0 |       3 |         0 |           4 |        16 |        16 |  0.000967 |       186x |    CODEC_FILTER |    HARD | -
|       zstd |      2 |     1 |       3 |         0 |           4 |        16 |        16 |  0.000307 |       285x |    THREADS_COMP |    HARD | -
|       zstd |      2 |     1 |       3 |         0 |           4 |        15 |        16 |  0.000902 |       198x |    THREADS_COMP |    HARD | -
|       zstd |      2 |     1 |       6 |         0 |           4 |        16 |        16 |  0.000637 |       323x |          CLEVEL |    HARD | W
|       zstd |      2 |     1 |       5 |         0 |           4 |        16 |        16 |   0.00037 |       362x |          CLEVEL |    HARD | W
|       zstd |      2 |     1 |       3 |         0 |           4 |        16 |        16 |  0.000197 |       290x |          CLEVEL |    HARD | -
|       zstd |      2 |     1 |       2 |         0 |           4 |        16 |        16 |  0.000236 |       197x |          CLEVEL |    SOFT | -
|       zstd |      2 |     1 |       3 |         0 |           4 |        16 |        16 |  0.000261 |       287x |          CLEVEL |    SOFT | -
|       zstd |      2 |     1 |       4 |         0 |           4 |        16 |        16 |  0.000267 |       286x |          CLEVEL |    SOFT | -
```

Furthermore, the Blosc Development Team provides a service in which BTune can use
a neural network model trained for your data so that it asserts better the combination
of codecs and filters.
To use it, once the Blosc Development Team has trained the model, you have to set the 
`BTUNE_DATA_DIR` to the path were the model files are and BTune will use it right
away!

```shell
PYTHONPATH=. BTUNE_BALANCE=0.5 BTUNE_TRACE=1  BTUNE_DATA_DIR=./models_sample/ python examples/schunk_roundtrip.py
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
BTune version: 1.0.0.
Perfomance Mode: COMP, Compression balance: 0.500000, Bandwidth: 20 GB/s.
Behaviour: Waits - 0, Softs - 5, Hards - 11, Repeat Mode - STOP.
INFO: Model files found in the 'models_sample/' directory
INFO: Created TensorFlow Lite XNNPACK delegate for CPU.
TIME LOAD MO: 0.000584
TIME ENTROPY: 0.000402
TIME INFEREN: 0.000022
|    Codec   | Filter | Split | C.Level | Blocksize | Shufflesize | C.Threads | D.Threads |   Score   |  C.Ratio   |   BTune State   | Readapt | Winner
|        lz4 |     34 |     1 |       5 |         0 |           4 |        16 |        16 |  0.000565 |       133x |    CODEC_FILTER |    HARD | W
|        lz4 |     34 |     0 |       5 |         0 |           4 |        16 |        16 |  0.000203 |       110x |    CODEC_FILTER |    HARD | -
|        lz4 |     34 |     1 |       5 |         0 |           4 |        16 |        16 |  0.000148 |       122x |    THREADS_COMP |    HARD | W
|        lz4 |     34 |     1 |       5 |         0 |           4 |        16 |        16 |  9.73e-05 |       122x |          CLEVEL |    HARD | W
|        lz4 |     34 |     1 |       4 |         0 |           4 |        16 |        16 |  0.000165 |       123x |          CLEVEL |    SOFT | W
|        lz4 |     34 |     1 |       5 |         0 |           4 |        16 |        16 |  4.39e-05 |       123x |          CLEVEL |    SOFT | W
|        lz4 |     34 |     1 |       6 |         0 |           4 |        16 |        16 |  4.73e-05 |       124x |          CLEVEL |    SOFT | W
|        lz4 |     34 |     1 |       5 |         0 |           4 |        16 |        16 |  4.06e-05 |       124x |          CLEVEL |    SOFT | W
|        lz4 |     34 |     1 |       4 |         0 |           4 |        16 |        16 |  0.000187 |       125x |          CLEVEL |    SOFT | W
|        lz4 |     34 |     1 |       5 |         0 |           4 |        16 |        16 |   4.4e-05 |       125x |          CLEVEL |    SOFT | -
|        lz4 |     34 |     1 |       4 |         0 |           4 |        16 |        16 |  0.000179 |       125x |          CLEVEL |    SOFT | W
|        lz4 |     34 |     1 |       5 |         0 |           4 |        16 |        16 |  5.46e-05 |       124x |          CLEVEL |    SOFT | -
|        lz4 |     34 |     1 |       4 |         0 |           4 |        16 |        16 |  4.09e-05 |       124x |          CLEVEL |    SOFT | -
|        lz4 |     34 |     1 |       4 |         0 |           4 |        16 |        16 |  4.38e-05 |       124x |    CODEC_FILTER |    HARD | -
|        lz4 |     34 |     0 |       4 |         0 |           4 |        16 |        16 |  4.35e-05 |       111x |    CODEC_FILTER |    HARD | -
|        lz4 |     34 |     1 |       4 |         0 |           4 |        16 |        16 |  4.23e-05 |       123x |    THREADS_COMP |    HARD | W
|        lz4 |     34 |     1 |       6 |         0 |           4 |        16 |        16 |  4.52e-05 |       123x |          CLEVEL |    HARD | W
|        lz4 |     34 |     1 |       5 |         0 |           4 |        16 |        16 |  3.88e-05 |       122x |          CLEVEL |    SOFT | -
|        lz4 |     34 |     1 |       6 |         0 |           4 |        16 |        16 |  3.76e-05 |       122x |          CLEVEL |    SOFT | -
|        lz4 |     34 |     1 |       5 |         0 |           4 |        16 |        16 |  3.84e-05 |       121x |          CLEVEL |    SOFT | -
```
As you can see, you can get much better performance using the trained models.

## Using BTune from C

You can also use BTune from a C program. Like in Python, you can activate it only by using `BTUNE_BALANCE`. Or 
alternatively, you can set the `tune_id` from the compression parameters (aka `cparams`) to `BLOSC_BTUNE`. This will use the default
BTune configuration, but the advantage of running BTune from C is that you can tune more parameters depending on what
you are interested in (see `src/btune_example.c`) for more info.

