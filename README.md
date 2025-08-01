# Btune for Blosc2

<img src="img/blosc2-btune-diagram-2.png">

Btune is a dynamic plugin for Blosc2 that assists in finding the optimal combination of compression parameters. It works by training a neural network on your most representative datasets.

By default, this software uses a genetic algorithm to test different combinations of compression parameters that meet your requirements for both compression ratio and speed for every chunk in the dataset. It assigns a score to each combination and, after a number of iterations, it stops and uses the best score (minimal value) found for the rest of the dataset. For more info and a graphical visualization, visit https://ironarray.io/btune.

The process of finding optimal compression parameters in Blosc2 can be slow because of the large number of combinations of compression parameters (codec, compression level, filter, split mode, number of threads, etc.). This can require a significant amount of trial and error to find the best combinations. However, you can significantly accelerate this process by training a neural network on your own datasets; [ironArray SLU](https://ironarray.io) provides professional services for doing that. If interested, [contact us](mailto:contact@ironarray.io).

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
Btune version: 1.1.2
Performance Mode: COMP, Compression tradeoff: 0.500000, Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 10, Repeat Mode - STOP
TRACE: Environment variable BTUNE_MODELS_DIR is not defined
WARNING: Empty metadata, no inference performed
|    Codec   | Filter | Split | C.Level | C.Threads | D.Threads |  S.Score  |  C.Ratio   |   Btune State   | Readapt | Winner
|        lz4 |      0 |     1 |       8 |        16 |        16 |     0.144 |      1.97x |    CODEC_FILTER |    HARD | W
|        lz4 |      0 |     0 |       8 |        16 |        16 |     0.933 |      2.07x |    CODEC_FILTER |    HARD | W
|        lz4 |      1 |     1 |       8 |        16 |        16 |      3.23 |      3.97x |    CODEC_FILTER |    HARD | W
|        lz4 |      1 |     0 |       8 |        16 |        16 |      7.66 |      3.91x |    CODEC_FILTER |    HARD | -
|        lz4 |      2 |     1 |       8 |        16 |        16 |      1.55 |      4.52x |    CODEC_FILTER |    HARD | -
|        lz4 |      2 |     0 |       8 |        16 |        16 |      2.84 |      4.46x |    CODEC_FILTER |    HARD | -
|    blosclz |      0 |     1 |       8 |        16 |        16 |     0.386 |      1.98x |    CODEC_FILTER |    HARD | -
|    blosclz |      0 |     0 |       8 |        16 |        16 |     0.492 |      2.09x |    CODEC_FILTER |    HARD | -
|    blosclz |      1 |     1 |       8 |        16 |        16 |      1.33 |      3.97x |    CODEC_FILTER |    HARD | -
|    blosclz |      1 |     0 |       8 |        16 |        16 |      1.72 |      3.82x |    CODEC_FILTER |    HARD | -
|    blosclz |      2 |     1 |       8 |        16 |        16 |      1.83 |      4.47x |    CODEC_FILTER |    HARD | -
|    blosclz |      2 |     0 |       8 |        16 |        16 |      1.65 |      4.35x |    CODEC_FILTER |    HARD | -
|        lz4 |      1 |     1 |       8 |        14 |        16 |     0.151 |      3.97x |    THREADS_COMP |    HARD | -
|        lz4 |      1 |     1 |       8 |        18 |        16 |     0.111 |      3.97x |    THREADS_COMP |    HARD | -
|        lz4 |      1 |     1 |       7 |        16 |        16 |     0.108 |      3.97x |          CLEVEL |    HARD | -
|        lz4 |      1 |     1 |       9 |        16 |        16 |      9.92 |      3.97x |          CLEVEL |    HARD | W
|        lz4 |      1 |     1 |       8 |        16 |        16 |      9.96 |      3.97x |          CLEVEL |    SOFT | W
|        lz4 |      1 |     1 |       7 |        16 |        16 |      9.74 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |      1 |     1 |       6 |        16 |        16 |      9.78 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |      1 |     1 |       7 |        16 |        16 |      10.1 |      3.97x |          CLEVEL |    SOFT | W
NDArray succesfully created!
```

You can see in the column `Winner` if the combination is a winner (`W`), it does not improve the previous winner (`-`). When Btune finds a special value chunk (i.e. the chunk is made of repeated values that are encoded in a special way), it outputs `S`, meaning that Btune cannot determine whether this is a winner or not (it is not compressed in the regular way).

## Btune Models

ironArray SLU offers **Btune Models**, a service in which we provide neural network models trained specifically for your data to determine the optimal combination of codecs and filters. To use these models, set `BTUNE_MODELS_DIR` to the directory containing the models files after the ironArray team has completed the training. Btune will then automatically use the trained model; keep reading for how this works.

To determine the number of chunks for performing inference, use `BTUNE_USE_INFERENCE`. If set to -1, it performs inference on all chunks. If set to a number greater than 0, it performs inference on this number of chunks and then tweaks parameters for the rest of the chunks. If set to 0, it does not perform inference at all. The default is -1.

```shell
BTUNE_TRADEOFF=0.5 BTUNE_PERF_MODE=COMP BTUNE_TRACE=1  BTUNE_MODELS_DIR=./models/ BTUNE_USE_INFERENCE=3 python create_ndarray.py
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Btune version: 1.1.2
Performance Mode: COMP, Compression tradeoff: 0.500000, Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 10, Repeat Mode - STOP
INFO: Created TensorFlow Lite XNNPACK delegate for CPU.
TRACE: time load model: 0.000558
TRACE: Inference category=39 codec=1 filter=35 clevel=5 splitmode=1 time entropy=0.000501 inference=0.000015
|    Codec   | Filter | Split | C.Level | C.Threads | D.Threads |  S.Score  |  C.Ratio   |   Btune State   | Readapt | Winner
|        lz4 |     35 |     1 |       5 |        16 |        16 |     0.162 |      3.97x |    CODEC_FILTER |    HARD | W
TRACE: Inference category=39 codec=1 filter=35 clevel=5 splitmode=1 time entropy=0.000201 inference=0.000002
|        lz4 |     35 |     1 |       5 |        16 |        16 |      4.14 |      3.97x |    CODEC_FILTER |    HARD | W
TRACE: Inference category=39 codec=1 filter=35 clevel=5 splitmode=1 time entropy=0.000175 inference=0.000003
|        lz4 |     35 |     1 |       5 |        16 |        16 |      2.85 |      3.97x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     1 |       5 |        16 |        16 |      3.79 |      3.97x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     1 |       5 |        14 |        16 |    0.0765 |      3.97x |    THREADS_COMP |    HARD | -
|        lz4 |     35 |     1 |       5 |        18 |        16 |    0.0996 |      3.97x |    THREADS_COMP |    HARD | -
|        lz4 |     35 |     1 |       5 |        16 |        16 |       0.1 |      3.97x |          CLEVEL |    HARD | -
|        lz4 |     35 |     1 |       6 |        16 |        16 |      3.87 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |        16 |        16 |       3.7 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       4 |        16 |        16 |      3.96 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |        16 |        16 |      3.94 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       6 |        16 |        16 |      3.96 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |        16 |        16 |      3.94 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       4 |        16 |        16 |      3.97 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |        16 |        16 |      3.97 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       6 |        16 |        16 |      3.94 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |        16 |        16 |      3.96 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |        16 |        16 |      3.98 |      3.97x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     1 |       5 |        14 |        16 |     0.116 |      3.97x |    THREADS_COMP |    HARD | -
|        lz4 |     35 |     1 |       5 |        18 |        16 |    0.0959 |      3.97x |    THREADS_COMP |    HARD | -
NDArray succesfully created!
```

Using Btune Models usually leads to significantly better performance scores, as demonstrated by the table above. Moreover, the process of finding the best combination is much faster with trained models.

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
Btune version: 1.1.2
Performance Mode: DECOMP, Compression tradeoff: 0.300000, Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 10, Repeat Mode - STOP
INFO: Created TensorFlow Lite XNNPACK delegate for CPU.
TRACE: time load model: 0.000503
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000292 inference=0.000009
|    Codec   | Filter | Split | C.Level | C.Threads | D.Threads |  S.Score  |  C.Ratio   |   Btune State   | Readapt | Winner
|        lz4 |      0 |     1 |       5 |        16 |        16 |      0.17 |      1.97x |    CODEC_FILTER |    HARD | W
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000137 inference=0.000002
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.285 |      1.96x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000040 inference=0.000001
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.282 |      1.96x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000035 inference=0.000001
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.272 |      1.97x |    CODEC_FILTER |    HARD | W
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000164 inference=0.000001
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.291 |      1.96x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000081 inference=0.000004
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.149 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000118 inference=0.000002
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.153 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000099 inference=0.000002
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.155 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000062 inference=0.000002
|        lz4 |      0 |     1 |       5 |        16 |        16 |      0.15 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000039 inference=0.000003
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.229 |      1.96x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000032 inference=0.000001
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.277 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000033 inference=0.000001
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.282 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000032 inference=0.000001
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.286 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000034 inference=0.000001
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.295 |      1.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000036 inference=0.000001
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.277 |      1.96x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=0 codec=0 filter=0 clevel=5 splitmode=2 time entropy=0.000035 inference=0.000001
|    blosclz |      0 |     0 |       5 |        16 |        16 |     0.263 |      2.08x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=0 codec=0 filter=0 clevel=5 splitmode=2 time entropy=0.000035 inference=0.000001
|    blosclz |      0 |     0 |       5 |        16 |        16 |      0.26 |      2.08x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=36 codec=1 filter=0 clevel=5 splitmode=1 time entropy=0.000032 inference=0.000001
|        lz4 |      0 |     1 |       5 |        16 |        16 |     0.294 |      1.96x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=0 codec=0 filter=0 clevel=5 splitmode=2 time entropy=0.000035 inference=0.000001
|    blosclz |      0 |     0 |       5 |        16 |        16 |     0.268 |      2.08x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=0 codec=0 filter=0 clevel=5 splitmode=2 time entropy=0.000035 inference=0.000001
|    blosclz |      0 |     0 |       5 |        16 |        16 |     0.266 |      2.08x |    CODEC_FILTER |    HARD | -
NDArray succesfully created in btune_config.b2nd
```

Here we set the tradeoff to 0.3 and the performance mode to `DECOMP`.

## Btune quality mode

Now Btune can work not only taking into account
the compression ratio and speed, but also the quality. This is intended
to work with datasets made out of images which datatypes are integers. Another 
requirement is that the `blosc2_grok` plugin will have to be installed in the
system.

To use it, you have to set the `BTUNE_TRADEOFF` to a tuple of 3 values 
`(cratio, speed, quality)`. These must sum up to 1. The bigger one value is,
the more important it will be. 

In the `examples` directory there is a Python example `lossy.py` which creates a NDArray
made out of 10 chunks (one chunk per image). You can run it with:

```shell
BTUNE_TRACE=1 python lossy.py lossy_example.tif
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Btune version: 1.1.2
Performance Mode: COMP, Compression tradeoff: (0.500000, 0.300000, 0.200000), Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 10, Repeat Mode - STOP
INFO: Created TensorFlow Lite XNNPACK delegate for CPU.
TRACE: time load model: 0.000487
|    Codec   | Filter | Split | C.Level | C.Threads | D.Threads |  S.Score  |  C.Ratio   |   Btune State   | Readapt | Winner
|       grok |      0 |     0 |       5 |        16 |        16 |  0.000468 |      8.15x |    CODEC_FILTER |    HARD | W
|       grok |      0 |     0 |       5 |        16 |        16 |    0.0236 |      8.15x |    CODEC_FILTER |    HARD | W
|       grok |      0 |     0 |       5 |        16 |        16 |    0.0297 |      8.15x |    CODEC_FILTER |    HARD | W
|       grok |      0 |     0 |       5 |        16 |        16 |    0.0362 |      8.15x |    CODEC_FILTER |    HARD | W
|       grok |      0 |     0 |       5 |        16 |        16 |    0.0328 |      8.15x |    CODEC_FILTER |    HARD | -
|       grok |      0 |     0 |       5 |        16 |        16 |     0.036 |      8.15x |    CODEC_FILTER |    HARD | -
|       grok |      0 |     0 |       5 |        16 |        16 |    0.0399 |      8.15x |    CODEC_FILTER |    HARD | W
|       grok |      0 |     0 |       5 |        16 |        16 |    0.0371 |      8.15x |    CODEC_FILTER |    HARD | -
|       grok |      0 |     0 |       5 |        16 |        16 |    0.0318 |      8.15x |    CODEC_FILTER |    HARD | -
|       grok |      0 |     0 |       5 |        16 |        16 |    0.0252 |      8.15x |    CODEC_FILTER |    HARD | -
```
The tradeoff used tells Btune that you care a lot about compression ratio but not as much
about speed or the quality loss. According to that, Btune predicts the `blosc2_grok` codec 
and manages to achieve a compression ratio of 8x in exchange for losing some quality.

Like in the traditional Btune, you can use the `BTUNE_TRADEOFF` environmnet
variable to change the tradeoff:
```shell
BTUNE_TRADEOFF="(0.3, 0.1, 0.6)" BTUNE_PERF_MODE=DECOMP BTUNE_TRACE=1 python lossy.py lossy_example.tif

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Btune version: 1.1.2
Performance Mode: DECOMP, Compression tradeoff: (0.300000, 0.100000, 0.600000), Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 10, Repeat Mode - STOP
INFO: Created TensorFlow Lite XNNPACK delegate for CPU.
TRACE: time load model: 0.000755
|    Codec   | Filter | Split | C.Level | C.Threads | D.Threads |  S.Score  |  C.Ratio   |   Btune State   | Readapt | Winner
|       zstd |     36 |     1 |       3 |        16 |        16 |     0.102 |      2.18x |    CODEC_FILTER |    HARD | W
|       zstd |     36 |     1 |       3 |        16 |        16 |     0.125 |      2.18x |    CODEC_FILTER |    HARD | W
|       zstd |     36 |     1 |       3 |        16 |        16 |    0.0815 |      2.18x |    CODEC_FILTER |    HARD | -
|       zstd |     36 |     1 |       3 |        16 |        16 |    0.0805 |      2.18x |    CODEC_FILTER |    HARD | -
|       zstd |     36 |     1 |       3 |        16 |        16 |    0.0801 |      2.18x |    CODEC_FILTER |    HARD | -
|       zstd |     36 |     1 |       3 |        16 |        16 |    0.0828 |      2.18x |    CODEC_FILTER |    HARD | -
|       zstd |     36 |     1 |       3 |        16 |        16 |    0.0834 |      2.18x |    CODEC_FILTER |    HARD | -
|       zstd |     36 |     1 |       3 |        16 |        16 |    0.0826 |      2.18x |    CODEC_FILTER |    HARD | -
|       zstd |     36 |     1 |       3 |        16 |        16 |     0.129 |      2.18x |    CODEC_FILTER |    HARD | W
|       zstd |     36 |     1 |       3 |        16 |        16 |     0.144 |      2.18x |    CODEC_FILTER |    HARD | W
```
In this case, because the quality is more important than before, Btune predicts
the `zstd` codec with the integer truncation filter (id 36), which achieves better quality than
the `blosc2_grok` codec with x8 compression ratio.

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
    btune_config.tradeoff[0] = .5;
    btune_config.tradeoff_nelems = 1;
    /* For lossy mode it would be
    btune_config.tradeoff[0] = .5;
    btune_config.tradeoff[1] = .2;
    btune_config.tradeoff[2] = .3;
    btune_config.tradeoff_nelems = 3;
    */
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
gcc -o btune_example btune_example.c -lblosc2 -lm -I $CONDA_PREFIX/include/ -I $CONDA_PREFIX/lib/python3.xx/site-packages/include/ -L $CONDA_PREFIX/lib/
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

We support Btune on Intel/ARM64 Linux and Mac, and Intel on Windows, and we are providing binary wheels for all of these.
