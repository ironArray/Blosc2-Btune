# Blosc2 Btune

Btune is a dynamic plugin for Blosc2 that assists in finding the optimal combination of compression parameters. It works by training a neural network on your most representative datasets.

By default, this software uses a genetic algorithm to test different combinations of compression parameters that meet your requirements for both compression ratio and speed for every chunk in the dataset. It assigns a score to each combination and, after a number of iterations, the software stops and uses the best score (minimal value) found for the rest of the dataset. For more info and a graphical visualization, visit https://btune.blosc.org.

The process of finding optimal compression parameters in Blosc2 can be slow because of the large number of combinations of compression parameters (codec, compression level, filter, split mode, number of threads, etc.). This can require a significant amount of trial and error to find the best combinations. However, you can significantly accelerate this process by training a neural network on your own datasets.

To begin the training process, provide your datasets to the Blosc Development Team. We will then perform the training and provide neural network models tailored to your needs, along with general tuning advice for Blosc2. In exchange, we request financial contributions to the project.

If you are interested, please contact the Blosc Development Team at contact@blosc.org.

## Install the Btune wheel

Btune uses a Python wheel for installation, but it can be used from any application that uses C-Blosc2, whether it is in C, Python, or any other language. Currently, only Linux and Mac installers are supported.

```shell
pip install blosc2-btune
```

Next, we will run an example for Python and then for C.
To do so, change your current directory to `examples` from this repository.

````shell
cd examples
````

## Using Btune from Python

To use Btune with Blosc2 in Python, you have two options.
The first one is to set the `BTUNE_TRADEOFF` environment variable to a floating-point number between 0 (to optimize speed) and 1 (to optimize compression ratio). Additionally, you can use `BTUNE_PERF_MODE` to optimize compression, decompression, or to achieve a balance between the two by setting it to `COMP`, `DECOMP`, or `BALANCED`, respectively.

```shell
BTUNE_TRADEOFF=0.5 BTUNE_PERF_MODE=COMP python create_ndarray.py
WARNING: Empty metadata, no inference performed
NDArray succesfully created!
```

This creates a NDArray on disk with some data. The warning message
`Empty metadata, no inference performed`
should be ignored as long as we are not using the trained models.

The second option is to set `cparams={"tuner": blosc2.Tuner.BTUNE}` when creating the array like in the
`btune_config.py`. We will see this example later in this section.

You can set `BTUNE_TRACE=1` to see what Btune is doing.

```shell
BTUNE_TRACE=1 BTUNE_TRADEOFF=0.5 BTUNE_PERF_MODE=COMP python create_ndarray.py
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Btune version: 1.0.1.dev
Performance Mode: COMP, Compression tradeoff: 0.500000, Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 11, Repeat Mode - STOP
TRACE: Environment variable BTUNE_MODELS_DIR is not defined
WARNING: Empty metadata, no inference performed
|    Codec   | Filter | Split | C.Level | Blocksize | Shufflesize | C.Threads | D.Threads |   Score   |  C.Ratio   |   Btune State   | Readapt | Winner
|        lz4 |      0 |     1 |       8 |         0 |           8 |         4 |         4 |  9.74e-05 |      1.97x |    CODEC_FILTER |    HARD | W
|        lz4 |      0 |     0 |       8 |         0 |           8 |         4 |         4 |  6.13e-05 |      2.07x |    CODEC_FILTER |    HARD | W
|        lz4 |      1 |     1 |       8 |         0 |           8 |         4 |         4 |  2.72e-05 |      3.97x |    CODEC_FILTER |    HARD | W
|        lz4 |      1 |     0 |       8 |         0 |           8 |         4 |         4 |   1.8e-05 |      3.91x |    CODEC_FILTER |    HARD | -
|        lz4 |      2 |     1 |       8 |         0 |           8 |         4 |         4 |     3e-05 |      4.52x |    CODEC_FILTER |    HARD | W
|        lz4 |      2 |     0 |       8 |         0 |           8 |         4 |         4 |  2.23e-05 |      4.46x |    CODEC_FILTER |    HARD | -
|    blosclz |      0 |     1 |       8 |         0 |           8 |         4 |         4 |  8.03e-05 |      1.99x |    CODEC_FILTER |    HARD | -
|    blosclz |      0 |     0 |       8 |         0 |           8 |         4 |         4 |  6.01e-05 |      2.09x |    CODEC_FILTER |    HARD | -
|    blosclz |      1 |     1 |       8 |         0 |           8 |         4 |         4 |  3.12e-05 |      3.97x |    CODEC_FILTER |    HARD | -
|    blosclz |      1 |     0 |       8 |         0 |           8 |         4 |         4 |   4.4e-05 |      3.82x |    CODEC_FILTER |    HARD | -
NDArray succesfully created!
```

You can see in the column `Winner` if the combination is a winner (`W`), it does not improve
the previous winner (`-`) or it is a special value chunk meaning that it is really easy to 
compress no matter the compression parameters (`S`), so Btune cannot determine whether
this is a winner or not in this last case.

## Btune Models

The Blosc Development Team offers **Btune Models**, a service in which we provide neural network models trained specifically for your data to determine the optimal combination of codecs and filters. To use these models, set `BTUNE_MODELS_DIR` to the directory containing the models files after the Blosc Development Team has completed training. Btune will then automatically use the trained model; keep reading for how this works.

To determine the number of chunks for performing inference, use `BTUNE_USE_INFERENCE`. If set to -1, it performs inference on all chunks. If set to a number greater than 0, it performs inference on this number of chunks and then tweaks parameters for the rest of the chunks. If set to 0, it does not perform inference at all. The default is -1.

```shell
BTUNE_TRADEOFF=0.5 BTUNE_PERF_MODE=COMP BTUNE_TRACE=1  BTUNE_MODELS_DIR=./models/ BTUNE_USE_INFERENCE=3 python create_ndarray.py
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Btune version: 1.0.1.dev
Performance Mode: COMP, Compression tradeoff: 0.500000, Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 11, Repeat Mode - STOP
INFO: Model files found in the './models/' directory
TRACE: time load model: 0.000071
TRACE: Inference category=7 codec=1 filter=35 clevel=5 splitmode=2 time entropy=0.000218 inference=0.000012
|    Codec   | Filter | Split | C.Level | Blocksize | Shufflesize | C.Threads | D.Threads |   Score   |  C.Ratio   |   Btune State   | Readapt | Winner
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |   5.4e-05 |      3.97x |    CODEC_FILTER |    HARD | W
TRACE: Inference category=7 codec=1 filter=35 clevel=5 splitmode=2 time entropy=0.000120 inference=0.000003
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  2.01e-05 |      3.97x |    CODEC_FILTER |    HARD | -
TRACE: Inference category=7 codec=1 filter=35 clevel=5 splitmode=2 time entropy=0.000104 inference=0.000002
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  1.92e-05 |      3.97x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |   2.1e-05 |      3.97x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     0 |       5 |         0 |           8 |         4 |         4 |  1.96e-05 |      3.91x |    CODEC_FILTER |    HARD | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  1.95e-05 |      3.97x |    THREADS_COMP |    HARD | W
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  1.96e-05 |      3.97x |          CLEVEL |    HARD | -
|        lz4 |     35 |     1 |       6 |         0 |           8 |         4 |         4 |  2.25e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       5 |         0 |           8 |         4 |         4 |  1.99e-05 |      3.97x |          CLEVEL |    SOFT | -
|        lz4 |     35 |     1 |       4 |         0 |           8 |         4 |         4 |  1.91e-05 |      3.97x |          CLEVEL |    SOFT | -
NDArray succesfully created!
```

Using Btune Models usually leads to significantly better performance scores, as demonstrated by the table above. Moreover, the process of finding the best combination is much faster with trained models.  See https://btune.blosc.org for more info.

### Configuring Btune from Python

If you want to use different configurations for different Blosc2 data containers in the same script, you can do it
configuring Btune from Python instead of using the environment variables.
To do so, you will have to set the desired configuration by passing it as 
keyword arguments to the `set_params_defaults` function.
```
kwargs = {"tradeoff": 0.3, "perf_mode": blosc2_btune.PerformanceMode.DECOMP}
blosc2_btune.set_params_defaults(**kwargs)
```
And then, tell Blosc2 you want to use Btune with `cparams={"tuner": blosc2.Tuner.BTUNE}`.
```
ba = blosc2.asarray(a, urlpath=urlpath, mode="w", chunks=(5e3,), cparams={"tuner": blosc2.Tuner.BTUNE})
```
You can see that the parameters have been applied activating the `BTUNE_TRACE`.

```shell
BTUNE_TRACE=1 python btune_config.py 
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Btune version: 1.0.3.dev
Performance Mode: DECOMP, Compression tradeoff: 0.300000, Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 10, Repeat Mode - STOP
TRACE: Environment variable BTUNE_MODELS_DIR is not defined
WARNING: Empty metadata, no inference performed
|    Codec   | Filter | Split | C.Level | Blocksize | Shufflesize | C.Threads | D.Threads |   Score   |  C.Ratio   |   Btune State   | Readapt | Winner
|        lz4 |      0 |     1 |       8 |         0 |           8 |         4 |         4 |  0.000237 |      1.95x |    CODEC_FILTER |    HARD | W
|        lz4 |      0 |     0 |       8 |         0 |           8 |         4 |         4 |  3.56e-05 |      2.07x |    CODEC_FILTER |    HARD | W
|        lz4 |      1 |     1 |       8 |         0 |           8 |         4 |         4 |  2.96e-05 |      3.97x |    CODEC_FILTER |    HARD | W
|        lz4 |      1 |     0 |       8 |         0 |           8 |         4 |         4 |  3.73e-05 |      3.91x |    CODEC_FILTER |    HARD | -
|        lz4 |      2 |     1 |       8 |         0 |           8 |         4 |         4 |  4.72e-05 |      4.52x |    CODEC_FILTER |    HARD | W
|        lz4 |      2 |     0 |       8 |         0 |           8 |         4 |         4 |  4.73e-05 |      4.46x |    CODEC_FILTER |    HARD | -
|      lz4hc |      0 |     1 |       8 |         0 |           8 |         4 |         4 |  4.22e-05 |      2.02x |    CODEC_FILTER |    HARD | -
|      lz4hc |      0 |     0 |       8 |         0 |           8 |         4 |         4 |  3.66e-05 |      2.39x |    CODEC_FILTER |    HARD | -
|      lz4hc |      1 |     1 |       8 |         0 |           8 |         4 |         4 |  3.16e-05 |      3.97x |    CODEC_FILTER |    HARD | -
|      lz4hc |      1 |     0 |       8 |         0 |           8 |         4 |         4 |  3.83e-05 |      3.92x |    CODEC_FILTER |    HARD | -
|      lz4hc |      2 |     1 |       8 |         0 |           8 |         4 |         4 |  4.51e-05 |      4.52x |    CODEC_FILTER |    HARD | W
|      lz4hc |      2 |     0 |       8 |         0 |           8 |         4 |         4 |  5.19e-05 |      4.47x |    CODEC_FILTER |    HARD | -
|      lz4hc |      2 |     1 |       8 |         0 |           8 |         4 |         4 |   5.6e-05 |      4.52x |  THREADS_DECOMP |    HARD | -
|      lz4hc |      2 |     1 |       8 |         0 |           8 |         4 |         3 |  4.97e-05 |      4.52x |  THREADS_DECOMP |    HARD | -
|      lz4hc |      2 |     1 |       7 |         0 |           8 |         4 |         4 |  4.78e-05 |      4.52x |          CLEVEL |    HARD | W
|      lz4hc |      2 |     1 |       5 |         0 |           8 |         4 |         4 |  4.87e-05 |      4.52x |          CLEVEL |    HARD | -
|      lz4hc |      2 |     1 |       4 |         0 |           8 |         4 |         4 |  4.07e-05 |      4.52x |          CLEVEL |    SOFT | -
|      lz4hc |      2 |     1 |       5 |         0 |           8 |         4 |         4 |   4.3e-05 |      4.52x |          CLEVEL |    SOFT | -
|      lz4hc |      2 |     1 |       6 |         0 |           8 |         4 |         4 |  4.07e-05 |      4.52x |          CLEVEL |    SOFT | -
|      lz4hc |      2 |     1 |       5 |         0 |           8 |         4 |         4 |  4.38e-05 |      4.52x |          CLEVEL |    SOFT | -
NDArray succesfully created!

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

## Platform support

Right now, we support Btune on Intel/ARM64 Linux and Intel/ARM64 Mac, and we are providing binary wheels for these.
