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

For Mac x86_64:

```shell
CMAKE_OSX_ARCHITECTURES=x86_64 CIBW_BEFORE_BUILD="bash prebuild.sh" python -m cibuildwheel --only 'cp311-macosx_x86_64'
```

For Mac arm64:

```shell
CMAKE_OSX_ARCHITECTURES=arm64 CIBW_BEFORE_BUILD="bash prebuild.sh" python -m cibuildwheel --only 'cp311-macosx_arm64'
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
./btune_example ./rand_int.b2nd out.b2nd
INFO: Model files found in the './models/' directory
INFO: Created TensorFlow Lite XNNPACK delegate for CPU.
Compression ratio: 0.8 MB -> 0.2 MB (4.0x)
Compression time: 0.008 s, 95.4 MB/s
```

You can use `BTUNE_TRACE=1`, `BTUNE_TRADEOFF=X` and `BTUNE_MODELS_DIR` to see how Btune is doing.

```shell
BTUNE_TRADEOFF=0.1 BTUNE_TRACE=1 BTUNE_MODELS_DIR=./models ./btune_example ./rand_int.b2nd out.b2nd
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Btune version: 1.0.3.dev
Performance Mode: COMP, Compression tradeoff: 0.100000, Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 10, Repeat Mode - REPEAT_ALL
INFO: Model files found in the './models' directory
INFO: Created TensorFlow Lite XNNPACK delegate for CPU.
TRACE: time load model: 0.000292
TRACE: Inference category=7 codec=1 filter=35 clevel=5 splitmode=2 time entropy=0.000242 inference=0.000022
|    Codec   | Filter | Split | C.Level | Blocksize | Shufflesize | C.Threads | D.Threads |   Score   |  C.Ratio   |   Btune State   | Readapt | Winner
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |  0.000181 |      3.97x |    CODEC_FILTER |    HARD | W
TRACE: Inference category=7 codec=1 filter=35 clevel=5 splitmode=2 time entropy=0.000078 inference=0.000004
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |  2.35e-05 |      3.97x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |  2.32e-05 |      3.97x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     0 |       5 |         0 |           8 |        16 |        16 |   2.2e-05 |      3.91x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |   2.3e-05 |      3.97x |    THREADS_COMP |    HARD | W
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |  2.28e-05 |      3.97x |          CLEVEL |    HARD | -
|        lz4 |     35 |     1 |       6 |         0 |           8 |        16 |        16 |  2.25e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |  2.34e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       4 |         0 |           8 |        16 |        16 |   2.4e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |  2.32e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       6 |         0 |           8 |        16 |        16 |  2.31e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |  2.52e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       4 |         0 |           8 |        16 |        16 |  2.23e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |  2.24e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       6 |         0 |           8 |        16 |        16 |  2.23e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |  2.25e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |  2.36e-05 |      3.97x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     0 |       5 |         0 |           8 |        16 |        16 |   2.1e-05 |      3.91x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |  2.12e-05 |      3.97x |    THREADS_COMP |    HARD | W
|        lz4 |     35 |     1 |       5 |         0 |           8 |        16 |        16 |  2.12e-05 |      3.97x |          CLEVEL |    HARD | -
Compression ratio: 0.8 MB -> 0.2 MB (4.0x)
Compression time: 0.0108 s, 70.7 MB/s
```

That's all folks!
