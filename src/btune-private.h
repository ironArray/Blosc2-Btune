/**********************************************************************
  Optimize Blosc2 parameters using deep/machine learning.

  Copyright (c) 2023 The Blosc Developers <blosc@blosc.org>
  License: GNU Affero General Public License v3.0

  See COPYING.txt for details about copyright and rights to use.
***********************************************************************/

/** @file  btune-private.h
 * @brief Btune private header file.
 *
 * This file contains the private methods and structures needed to use Btune.
 */

#ifndef BTUNE_PRIVATE_H
#define BTUNE_PRIVATE_H

#include <stdbool.h>
#include "context.h"


// Internal Btune compression parameters
typedef struct {
    int compcode;
    // The compressor code
    uint8_t filter;
    // The precompression filter
    int32_t splitmode;
    // Whether the blocks should be split or not
    int clevel;
    // The compression level
    int32_t blocksize;
    // The block size
    int32_t shufflesize;
    // The shuffle size
    int nthreads_comp;
    // The number of threads used for compressing
    int nthreads_decomp;
    // The number of threads used for decompressing
    bool increasing_clevel;
    // Control parameter for clevel
    bool increasing_block;
    // Control parameter for blocksize
    bool increasing_shuffle;
    // Control parameter for shufflesize
    bool increasing_nthreads;
    // Control parameter for nthreads
    double score;
    // The score obtained with this cparams
    double cratio;
    // The cratio obtained with this cparams
    double ctime;
    // The compression time obtained with this cparams
    double dtime;
    // The decompression time obtained with this cparams
} cparams_btune;

// Btune struct
typedef struct {
  btune_config config;
  // The Btune config
  int codecs[BTUNE_MAX_CODECS];
  // The codec list used by Btune
  uint8_t ncodecs;
  // Number of codecs used by Btune
  uint8_t filters[BTUNE_MAX_FILTERS];
  // The filter list used by Btune
  uint8_t nfilters;
  // Number of filters used by Btune
  int32_t splitmode;
  // Splitmode
  uint8_t clevels[BTUNE_MAX_CLEVELS];
  // The clevels list used by Btune
  uint8_t nclevels;
  // Number of clevels used by Btune
  cparams_btune * best;
  // The best cparams optained with Btune
  cparams_btune * aux_cparams;
  // The aux cparams for updating the best
  double * current_scores;
  // The aux array of scores to calculate the mean
  double * current_cratios;
  // The aux array of cratios to calculate the mean
  int rep_index;
  // The aux index for the repetitions
  int aux_index;
  // The auxiliar index for state management
  int clevel_index;
  // The index for the clevel array
  int steps_count;
  // The count of steps made by Btune
  btune_state state;
  // The state of Btune
  int step_size;
  // The step size within clevels and blocksizes
  int nwaitings;
  // The total count of nwaitings states
  int nsofts;
  // The total count of soft readapts
  int nhards;
  // The total count of hard readapts
  bool is_repeating;
  // If Btune has completed the initial readapts
  readapt_type readapt_from;
  // If Btune is making a hard or soft readapt, or is WAITING
  int max_threads;
  // The maximum number of threads used
  blosc2_context * dctx;
  // The decompression context
  int nthreads_decomp;
  // The number of threads for decompression (used if dctx is NULL)
  bool threads_for_comp;
  // Depending on this value the THREADS state will change the compression or decompression threads
  void * interpreter;
  // TF Lite model, used for inference
  void * metadata;
  // Metadata information used for model inference
  float zeros_speed;
  // Entropy speed for a zeros chunk.
  int inference_count;
  // Number of times to run inference
  bool inference_ended;
  // Whether all desired ninferences were already performed.
} btune_struct;
/// @endcond

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
void btune_init(void * config, blosc2_context* cctx, blosc2_context* dctx);

void btune_free(blosc2_context* context);

void btune_next_cparams(blosc2_context *context);

void btune_update(blosc2_context* context, double ctime);

void btune_next_blocksize(blosc2_context *context);

#endif  /* BTUNE_PRIVATE_H */
