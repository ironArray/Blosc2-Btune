# Blosc2 BTune

BTune is a dynamic plugin for Blosc2 that helps you find the optimal combination of compression parameters by training a neural network on your most representative datasets.

By default, this software uses a brute force approach (a genetic algorithm) to test different combinations of compression parameters that meet your requirements for both compression ratio and speed for every chunk in the dataset. It assigns a score to each combination. After a number of iterations, the software stops and uses the best score (minimal value) found for the rest of the dataset. For a more graphical explanation, visit https://btune.blosc.org.

The process of finding optimal compression parameters in Blosc2 can be slow. Due to the large number of combinations of compression parameters (codec, compression level, filter, split mode, number of threads, etc.), a significant amount of trial and error may be required to find the best combinations. However, the good news is that this process can be significantly accelerated by training a neural network on your own datasets.

To start the training process, provide your datasets (several tens or hundreds of gigabytes, depending on your requirements) to the Blosc Development Team. We will then perform the training and provide neural network models tailored to your needs, along with general tuning advice for Blosc2. In exchange, we request monetary contributions to the project.

If interested, please contact the Blosc Development Team at contact@blosc.org.

## Install the BTune wheel

BTune uses a Python wheel for installation, but it can be used from any application that uses C-Blosc2, whether it is in C, Python, or any other language. Currently, only Linux and Mac installers are supported.

```shell
pip install blosc2_btune
```

## Using BTune from Python

To use BTune with Blosc2 in Python, set the BTUNE_BALANCE environment variable to a floating-point number between 0 (for optimizing speed) and 1 (for optimizing compression ratio).

```shell
BTUNE_BALANCE=0.5 python python-blosc2/examples/schunk.py
```

You can use `BTUNE_TRACE=1` to see what BTune is doing.

```shell
PYTHONPATH=. BTUNE_BALANCE=0.5 BTUNE_TRACE=1  python examples/schunk_roundtrip.py 
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
BTune version: 1.0.0
Perfomance Mode: COMP, Compression balance: 0.500000, Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 11, Repeat Mode - STOP
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

Additionally, the Blosc Development Team offers a service in which BTune can utilize a neural network model trained specifically for your data to better determine the optimal combination of codecs and filters. To use this service, once the Blosc Development Team has trained the model, you need to set the `BTUNE_MODELS_DIR` to the directory containing the model files. BTune will then use the trained model automatically.

```shell
PYTHONPATH=. BTUNE_BALANCE=0.5 BTUNE_TRACE=1  BTUNE_MODELS_DIR=./models_sample/ python examples/schunk_roundtrip.py
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
BTune version: 1.0.0
Perfomance Mode: COMP, Compression balance: 0.500000, Bandwidth: 20 GB/s
Behaviour: Waits - 0, Softs - 5, Hards - 11, Repeat Mode - STOP
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

Using trained models leads to much better performance scores, as demonstrated here by a balance between compression speed and compression ratio. Furthermore, the process of finding the best combination is significantly faster with trained models.

## Using BTune from C

You can also use BTune from a C program. Similar to Python, you can activate it by using `BTUNE_BALANCE`, or alternatively, you can set the `tuner_id` in the compression parameters (aka `cparams`) to `BLOSC_BTUNE`. This will use the default BTune configuration, but the advantage of running BTune from C is that you can tune more parameters depending on what you are interested in.

```
    // compression params
    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    cparams.nthreads = 16; // Btune may lower this
    cparams.tuner_id = BLOSC_BTUNE;
    
    // BTune config parameters
    btune_config btune_config = BTUNE_CONFIG_DEFAULTS;
    btune_config.perf_mode = BTUNE_PERF_COMP; // You can choose BTUNE_PERF_COMP, BTUNE_PERF_DECOMP or BTUNE_PERF_BALANCED
    btune_config.comp_balance = .5; // Equivalent to BTUNE_BALANCE
    btune_config.use_inference = 2; // Number of times to perform inference, equivalent to BTUNE_USE_INFERENCE
    btune_config.models_dir = "../models_dir/"; // Equivalent to BTUNE_MODELS_DIR
    btune_config.behaviour.nwaits_before_readapt = 1;       // Number of waits before a readapt
    btune_config.behaviour.nsofts_before_hard = 3;          // Number of soft readapts before a hard readapt
    btune_config.behaviour.nhards_before_stop = 10;         // Number of hard readapts before stoping
    btune_config.behaviour.repeat_mode = BTUNE_REPEAT_ALL;  // Repeat all the initial readaptions (BTUNE_REPEAT_ALL), 
                                                            // only soft readaptions (BTUNE_REPEAT_SOFT)
                                                            // or stop improving (BTUNE_STOP)
    // Set the personalized BTune configuration
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

To see the full example see `src/btune_example.c`.
