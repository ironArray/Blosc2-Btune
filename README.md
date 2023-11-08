# Blosc2 Btune

Btune is a dynamic plugin for Blosc2 that assists in finding the optimal combination of compression parameters. It works by training a neural network on your most representative datasets.

By default, this software uses a genetic algorithm to test different combinations of compression parameters that meet your requirements for both compression ratio and speed for every chunk in the dataset. It assigns a score to each combination and, after a number of iterations, the software stops and uses the best score (minimal value) found for the rest of the dataset. For more info and a graphical visualization, visit https://btune.blosc.org.

The process of finding optimal compression parameters in Blosc2 can be slow because of the large number of combinations of compression parameters (codec, compression level, filter, split mode, number of threads, etc.). This can require a significant amount of trial and error to find the best combinations. However, you can significantly accelerate this process by training a neural network on your own datasets.

To begin the training process, provide your datasets to the Blosc Development Team. We will then perform the training and provide neural network models tailored to your needs, along with general tuning advice for Blosc2. In exchange, we request financial contributions to the project.

If interested, please contact us at contact@blosc.org.

## Install the Btune wheel

Btune uses a Python wheel for installation, but it can be used from any application that uses C-Blosc2, whether it is in C, Python, or any other language. Currently, only Linux and Mac installers are supported.

```shell
pip install blosc2-btune
```

Next, we will run an example for Python and then for C. To do so, change your current directory to `examples` from this repository.

````shell
cd examples
````

## Using Btune from Python

To use Btune with Blosc2 in Python, you can do it either via environment variables or programmatically.

### Via environment variables

* Set the `BTUNE_TRADEOFF` environment variable to a floating-point number between 0 (to optimize just for speed) and 1 (to optimize just for compression ratio). 
* Additionally, you can use `BTUNE_PERF_MODE` to optimize for compression, decompression, or to achieve a balance between the two by setting it to `COMP`, `DECOMP`, or `BALANCED`, respectively.

```shell
BTUNE_TRADEOFF=0.5 BTUNE_PERF_MODE=COMP python create_ndarray.py
WARNING: Empty metadata, no inference performed
NDArray succesfully created!
```

This creates a NDArray on disk with some data. The warning message `Empty metadata, no inference performed` can be ignored, as we are not using trained models yet.

### Programmatically

Set `cparams={"tuner": blosc2.Tuner.BTUNE}` when creating the array like in the `btune_config.py` script. We will visit this example later in this section.

### Enabling tracing

You can set `BTUNE_TRACE=1` to see what Btune is doing:

```shell
BTUNE_TRACE=1 BTUNE_TRADEOFF=0.5 BTUNE_PERF_MODE=COMP python create_ndarray.py
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Btune version: 1.0.4.dev
Performance Mode: COMP, Compression tradeoff: 0.500000, Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 10, Repeat Mode - STOP
TRACE: Environment variable BTUNE_MODELS_DIR is not defined
WARNING: Empty metadata, no inference performed
|    Codec   | Filter | Split | C.Level | Blocksize | Shufflesize | C.Threads | D.Threads |   Score   |  C.Ratio   |   Btune State   | Readapt | Winner
|        lz4 |      0 |     1 |       8 |         0 |           8 |         4 |         4 |  9.34e-05 |      1.97x |    CODEC_FILTER |    HARD | W
|        lz4 |      0 |     0 |       8 |         0 |           8 |         4 |         4 |  6.22e-05 |      2.08x |    CODEC_FILTER |    HARD | W
|        lz4 |      1 |     1 |       8 |         0 |           8 |         4 |         4 |  2.46e-05 |      3.97x |    CODEC_FILTER |    HARD | W
|        lz4 |      1 |     0 |       8 |         0 |           8 |         4 |         4 |  2.35e-05 |      3.91x |    CODEC_FILTER |    HARD | -
|        lz4 |      2 |     1 |       8 |         0 |           8 |         4 |         4 |  3.24e-05 |      4.52x |    CODEC_FILTER |    HARD | W
|        lz4 |      2 |     0 |       8 |         0 |           8 |         4 |         4 |  2.32e-05 |      4.46x |    CODEC_FILTER |    HARD | -
|    blosclz |      0 |     1 |       8 |         0 |           8 |         4 |         4 |  8.48e-05 |      1.98x |    CODEC_FILTER |    HARD | -
|    blosclz |      0 |     0 |       8 |         0 |           8 |         4 |         4 |   5.7e-05 |      2.08x |    CODEC_FILTER |    HARD | -
|    blosclz |      1 |     1 |       8 |         0 |           8 |         4 |         4 |  3.13e-05 |      3.97x |    CODEC_FILTER |    HARD | -
|    blosclz |      1 |     0 |       8 |         0 |           8 |         4 |         4 |  4.43e-05 |      3.82x |    CODEC_FILTER |    HARD | -
|    blosclz |      2 |     1 |       8 |         0 |           8 |         4 |         4 |  3.99e-05 |      4.47x |    CODEC_FILTER |    HARD | -
|    blosclz |      2 |     0 |       8 |         0 |           8 |         4 |         4 |  4.44e-05 |      4.35x |    CODEC_FILTER |    HARD | -
|        lz4 |      2 |     1 |       8 |         0 |           8 |         4 |         4 |  2.27e-05 |      4.52x |    THREADS_COMP |    HARD | W
|        lz4 |      2 |     1 |       7 |         0 |           8 |         4 |         4 |  2.18e-05 |      4.52x |          CLEVEL |    HARD | W
|        lz4 |      2 |     1 |       5 |         0 |           8 |         4 |         4 |  2.57e-05 |      4.52x |          CLEVEL |    HARD | -
|        lz4 |      2 |     1 |       4 |         0 |           8 |         4 |         4 |  2.15e-05 |      4.52x |          CLEVEL |    SOFT | -
|        lz4 |      2 |     1 |       5 |         0 |           8 |         4 |         4 |  2.23e-05 |      4.52x |          CLEVEL |    SOFT | -
|        lz4 |      2 |     1 |       6 |         0 |           8 |         4 |         4 |  2.17e-05 |      4.52x |          CLEVEL |    SOFT | -
|        lz4 |      2 |     1 |       5 |         0 |           8 |         4 |         4 |  2.17e-05 |      4.52x |          CLEVEL |    SOFT | -
|        lz4 |      2 |     1 |       4 |         0 |           8 |         4 |         4 |  2.17e-05 |      4.52x |          CLEVEL |    SOFT | W
NDArray succesfully created!
```

You can see in the column `Winner` if the combination is a winner (`W`), it does not improve the previous winner (`-`). When Btune finds a special value chunk (i.e. the chunk is made of repeated values that are encoded in a special way), it outputs `S`, meaning that Btune cannot determine whether this is a winner or not (it is not compressed in the regular way).

## Btune Models

The Blosc Development Team offers **Btune Models**, a service in which we provide neural network models trained specifically for your data to determine the optimal combination of codecs and filters. To use these models, set `BTUNE_MODELS_DIR` to the directory containing the models files after the Blosc Development Team has completed training. Btune will then automatically use the trained model; keep reading for how this works.

To determine the number of chunks for performing inference, use `BTUNE_USE_INFERENCE`. If set to -1, it performs inference on all chunks. If set to a number greater than 0, it performs inference on this number of chunks and then tweaks parameters for the rest of the chunks. If set to 0, it does not perform inference at all. The default is -1.

```shell
BTUNE_TRADEOFF=0.5 BTUNE_PERF_MODE=COMP BTUNE_TRACE=1  BTUNE_MODELS_DIR=./models/ BTUNE_USE_INFERENCE=3 python create_ndarray.py
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Btune version: 1.0.4.dev
Performance Mode: COMP, Compression tradeoff: 0.500000, Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 10, Repeat Mode - STOP
INFO: Created TensorFlow Lite XNNPACK delegate for CPU.
TRACE: time load model: 0.001125
TRACE: Inference category=39 codec=1 filter=35 clevel=5 splitmode=1 time entropy=0.000296 inference=0.000057
|    Codec   | Filter | Split | C.Level | Blocksize | Shufflesize | C.Threads | D.Threads |   Score   |  C.Ratio   |   Btune State   | Readapt | Winner
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  5.53e-05 |      3.97x |    CODEC_FILTER |    HARD | W
TRACE: Inference category=39 codec=1 filter=35 clevel=5 splitmode=1 time entropy=0.000091 inference=0.000003
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  2.06e-05 |      3.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=39 codec=1 filter=35 clevel=5 splitmode=1 time entropy=0.000081 inference=0.000003
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  2.11e-05 |      3.97x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  2.03e-05 |      3.97x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  1.99e-05 |      3.97x |    THREADS_COMP |    HARD | W
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  1.99e-05 |      3.97x |          CLEVEL |    HARD | -
|        lz4 |     35 |     1 |       6 |         0 |           8 |         4 |         4 |  2.04e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  2.08e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       4 |         0 |           8 |         4 |         4 |  1.98e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  1.96e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       6 |         0 |           8 |         4 |         4 |  1.97e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  1.98e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       4 |         0 |           8 |         4 |         4 |  1.98e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  1.98e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       6 |         0 |           8 |         4 |         4 |  1.98e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  1.98e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  1.99e-05 |      3.97x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  1.97e-05 |      3.97x |    THREADS_COMP |    HARD | W
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  2.02e-05 |      3.97x |          CLEVEL |    HARD | -
|        lz4 |     35 |     1 |       6 |         0 |           8 |         4 |         4 |  1.97e-05 |      3.97x |          CLEVEL |    SOFT | -
NDArray succesfully created!
```

Using Btune Models usually leads to significantly better performance scores, as demonstrated by the table above. Moreover, the process of finding the best combination is much faster with trained models.  See https://btune.blosc.org for more info.

### Configuring Btune programmatically from Python

If you want to use different configurations for different Blosc2 data containers in the same script, you can do it configuring Btune from Python instead of using the environment variables. To do so, you will have to set the desired configuration by passing it as keyword arguments to the `set_params_defaults` function:

```
kwargs = {"tradeoff": 0.3, "perf_mode": blosc2_btune.PerformanceMode.DECOMP}
blosc2_btune.set_params_defaults(**kwargs)
```

And then, tell Blosc2 you want to use Btune with `cparams={"tuner": blosc2.Tuner.BTUNE}`:

```
ba = blosc2.asarray(a, urlpath=urlpath, mode="w", chunks=(1e6,), cparams={"tuner": blosc2.Tuner.BTUNE})
```

See an output example when activating the `BTUNE_TRACE` environment variable:

```shell
BTUNE_TRACE=1 python btune_config.py
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Btune version: 1.0.4.dev
Performance Mode: DECOMP, Compression tradeoff: 0.300000, Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 10, Repeat Mode - STOP
INFO: Created TensorFlow Lite XNNPACK delegate for CPU.
TRACE: time load model: 0.000260
TRACE: Inference category=4 codec=1 filter=0 clevel=5 splitmode=2 time entropy=0.000215 inference=0.000015
|    Codec   | Filter | Split | C.Level | Blocksize | Shufflesize | C.Threads | D.Threads |   Score   |  C.Ratio   |   Btune State   | Readapt | Winner
|        lz4 |      0 |     0 |       5 |         0 |           8 |         4 |         4 |  5.13e-05 |      2.07x |    CODEC_FILTER |    HARD | W
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000072 inference=0.000004
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  5.25e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000069 inference=0.000003
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  4.45e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000068 inference=0.000003
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  3.89e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000058 inference=0.000002
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  3.75e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000071 inference=0.000003
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  4.06e-05 |      1.96x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000076 inference=0.000003
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |   3.8e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000072 inference=0.000003
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  3.82e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000065 inference=0.000002
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |   3.7e-05 |      1.96x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000063 inference=0.000002
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  3.78e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000059 inference=0.000002
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  3.79e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000067 inference=0.000002
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  4.27e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000063 inference=0.000002
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  3.74e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000059 inference=0.000003
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |     4e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000073 inference=0.000003
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  3.84e-05 |      1.96x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000080 inference=0.000003
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  3.76e-05 |      1.96x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000088 inference=0.000002
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  4.37e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000083 inference=0.000003
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  3.78e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000073 inference=0.000002
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  4.09e-05 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000072 inference=0.000003
|        lz4 |      0 |     1 |       5 |         0 |           8 |         4 |         4 |  3.85e-05 |      1.96x |    CODEC_FILTER |    HARD | -
NDArray succesfully created in btune_config.b2nd
```

Here we set the tradeoff to 0.3 and the performance mode to `DECOMP`.

## Using Btune from C

You can also use Btune from C. Similar to the Python examples above, you can activate it by setting the `BTUNE_TRADEOFF` environment variable. Alternatively, you can set the `tuner_id` in the compression parameters, also known as `cparams`, to the value of `BLOSC_BTUNE`. This will use the default Btune configuration. However, running Btune from C offers the advantage of being able to tune way more parameters, depending on your preferences:

```
    // compression params
    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    cparams.nthreads = 16; // Btune may lower this
    cparams.typesize = schunk_in->typesize;

    // btune
    btune_config btune_config = BTUNE_CONFIG_DEFAULTS;
    //btune_config.perf_mode = BTUNE_PERF_DECOMP;
    btune_config.tradeoff = .5;
    btune_config.behaviour.nhards_before_stop = 10;
    btune_config.behaviour.repeat_mode = BTUNE_REPEAT_ALL;
    btune_config.use_inference = 2;
    char *models_dir = "./models/";
    strcpy(btune_config.models_dir, models_dir);
    cparams.tuner_id = BLOSC_BTUNE;
    cparams.tuner_params = &btune_config;

    // Create super chunk
    blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
    dparams.nthreads = 1;
    blosc2_storage storage = {
        .cparams=&cparams,
        .dparams=&dparams,
        .contiguous=true,
        .urlpath=(char*)out_fname
    };
    blosc2_schunk* schunk_out = blosc2_schunk_new(&storage);
```

See the full example in `examples/btune_example.c`. You can compile and run this example as follows:

### Linux
```shell
gcc -o btune_example btune_example.c -lblosc2 -lm -I $CONDA_PREFIX/include/ -L $CONDA_PREFIX/lib64/
BTUNE_TRACE=1 LD_LIBRARY_PATH=$CONDA_PREFIX/lib64 ./btune_example rand_int.b2nd out.b2nd
```

### MacOS
```shell
gcc -o btune_example btune_example.c -lblosc2 -lm -I $CONDA_PREFIX/include/ -L $CONDA_PREFIX/lib/
BTUNE_TRACE=1 DYLD_LIBRARY_PATH=$CONDA_PREFIX/lib ./btune_example rand_int.b2nd out.b2nd
```

## Optimization tips

If you would like to use the same models for different arrays, you can save the loading time and reuse the first loaded
model with the Python context manager `ReuseModels`:

```
with blosc2_btune.ReuseModels():
    for nchunk in range(0, nchunks):
        b = blosc2.asarray(a[nchunk * chunk_nitems:(nchunk + 1) * chunk_nitems], chunks=(chunk_nitems,), blocks=(chunk_nitems//10,), cparams=cparams)
        tr += time() - tref
```
This enables reusing the models when they are the same inside the context and manages all the references and memory
needed to be deallocated at the end of it. Depending on your needs, this may accelerate your program around a 5%. You can see
a comparison of reusing the models and reloading them each time in the `reuse_models.py` example::

```
INFO: Created TensorFlow Lite XNNPACK delegate for CPU.
Creating arrays reusing loaded models
Creating arrays reloading models each time
Reusing time: 0.542s (1.476 GB/s)
Reloading time: 0.547s (1.463 GB/s)
```

## Platform support

Right now, we support Btune on Intel/ARM64 Linux and Intel/ARM64 Mac and Intel on Windows, and we are providing binary wheels for these.
