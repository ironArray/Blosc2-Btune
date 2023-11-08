//
// Created by Francesc Alted on 19/10/23.
//

#ifndef BLOSC2_BTUNE_BTUNE_INFO_PUBLIC_H
#define BLOSC2_BTUNE_BTUNE_INFO_PUBLIC_H

#if defined(_MSC_VER)
#define BLOSC2_BTUNE_EXPORT __declspec(dllexport)
#elif (defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define BLOSC2_BTUNE_EXPORT __attribute__((dllexport))
#else
#define BLOSC2_BTUNE_EXPORT __attribute__((visibility("default")))
#endif  /* defined(_WIN32) || defined(__CYGWIN__) */
#else
#error Cannot determine how to define BLOSC2_BTUNE_EXPORT for this compiler.
#endif

#include "btune.h"


BLOSC2_BTUNE_EXPORT int set_params_defaults(
    uint32_t bandwidth,
    uint32_t perf_mode,
    float tradeoff,
    bool cparams_hint,
    int use_inference,
    const char* models_dir,
    uint32_t nwaits,
    uint32_t nsofts,
    uint32_t nhards,
    uint32_t repeat_mode
);

BLOSC2_BTUNE_EXPORT void btune_free_all_models(void);

BLOSC2_BTUNE_EXPORT void btune_set_reuse_models(bool new_value);

/**
 * @brief Btune initializer.
 *
 * This method initializes Btune in the compression context and then it will be used automatically.
 * On each compression, Btune overwrites the compression parameters in the context and, depending
 * on the results obtained and its configuration, will adjust them.
 * Example of use:
 * @code{.c}
 * blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
 * blosc2_dparams params = BLOSC2_DPARAMS_DEFAULTS;
 * btune_config config = BTUNE_CONFIG_DEFAULTS;
 * blosc2_storage storage = {.cparams=&cparams, .dparams=&dparams};
 * blosc2_schunk * schunk = blosc2_schunk_new(&storage);
 * btune_init(&config, schunk->cctx, schunk->dctx);
 * @endcode
 * @param config The Btune configuration determines its behaviour and how will optimize.
 * @param cctx The compression context where Btune tunes the compression parameters. It <b>can not</b> be NULL.
 * @param dctx If not NULL, Btune will modify the number of threads for decompression inside this context.
*/
BLOSC2_BTUNE_EXPORT int btune_init(void * config, blosc2_context* cctx, blosc2_context* dctx);

BLOSC2_BTUNE_EXPORT int btune_free(blosc2_context* context);

BLOSC2_BTUNE_EXPORT int btune_next_cparams(blosc2_context *context);

BLOSC2_BTUNE_EXPORT int btune_update(blosc2_context* context, double ctime);

BLOSC2_BTUNE_EXPORT int btune_next_blocksize(blosc2_context *context);

// Blosc2 needs this in order to dynamically load the functions
BLOSC2_BTUNE_EXPORT tuner_info info = {
    .init=(char *)"btune_init",
    .next_blocksize=(char *)"btune_next_blocksize",
    .next_cparams=(char *)"btune_next_cparams",
    .update=(char *)"btune_update",
    .free=(char *)"btune_free"
};

#endif //BLOSC2_BTUNE_BTUNE_INFO_PUBLIC_H
