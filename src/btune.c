/*********************************************************************
  Btune for Blosc2 - Automatically choose the best codec/filter for your data

  Copyright (c) 2023-present  Blosc Development Team <blosc@blosc.org>
  https://btune.blosc.org
  Copyright (c) 2023-present  ironArray SLU <contact@ironarray.io>
  https://ironarray.io
  License: GNU Affero General Public License v3.0
  See LICENSE.txt for details about copyright and rights to use.
**********************************************************************/

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <blosc2/filters-registry.h>
#include <blosc2/codecs-registry.h>
#include "btune_info_public.h"
#include "btune_model.h"
#include "entropy_probe.h"
#include "btune-private.h"


// Disable different states
#define BTUNE_ENABLE_MEMCPY       false
#define BTUNE_ENABLE_THREADS      true


// Internal btune control behaviour constants.
enum {
  BTUNE_KB = 1024,
  MIN_BLOCK = 16 * BTUNE_KB,  // TODO remove when included in blosc.h
  MAX_BLOCK = 2 * BTUNE_KB * BTUNE_KB,
  MIN_THREADS = 1,
  SOFT_STEP_SIZE = 1,
  HARD_STEP_SIZE = 2,
  MAX_STATE_THREADS = 50,  // 50 magic number big enough to not tune threads this number of times
};

static const cparams_btune cparams_btune_default = {
  .compcode = BLOSC_LZ4,
  .filter = BLOSC_SHUFFLE,
  .splitmode = BLOSC_ALWAYS_SPLIT,
  .clevel = 9,
  .blocksize = 0,
  .nthreads_comp = 0,
  .nthreads_decomp = 0,
  .increasing_clevel = false,
  .increasing_block = true,
  .increasing_nthreads = true,
  .score = 100,
  .cratio = 1.0,
  .ctime = 100,
  .dtime = 100
};

static void add_codec(btune_struct *btune_params, int compcode) {
  for (int i = 0; i < btune_params->ncodecs; i++) {
    if (btune_params->codecs[i] == compcode) {
      return;
    }
  }
  assert(btune_params->ncodecs < BTUNE_MAX_CODECS);
  btune_params->codecs[btune_params->ncodecs] = compcode;
  btune_params->ncodecs++;
}

static void add_filter(btune_struct *btune_params, uint8_t filter) {
  for (int i = 0; i < btune_params->nfilters; i++) {
    if (btune_params->filters[i] == filter) {
      return;
    }
  }
  assert(btune_params->nfilters < BTUNE_MAX_FILTERS);
  btune_params->filters[btune_params->nfilters] = filter;
  btune_params->nfilters++;
}

// Get the codecs list for btune
static void btune_init_codecs(btune_struct *btune_params) {
  const char * all_codecs = blosc2_list_compressors();
  // Already checked that 0. <= tradeoff <= 1.
  float tradeoff_1d = btune_params->config.tradeoff[0];
  if (btune_params->config.tradeoff_nelems == 3) {
    tradeoff_1d = btune_params->config.tradeoff[0] + btune_params->config.tradeoff[2] / 2;
  }
  if (0.666666 <= tradeoff_1d) {
    // In HCR mode only try with ZSTD and ZLIB
    if (strstr(all_codecs, "zstd") != NULL) {
      add_codec(btune_params, BLOSC_ZSTD);
    }
    if (strstr(all_codecs, "zlib") != NULL) {
      add_codec(btune_params, BLOSC_ZLIB);
    }
    // And disable LZ4HC as it compress typically less
    // add_codec(btune_params, BLOSC_LZ4HC);
  } else {
    // In all other modes, LZ4 is mandatory
    add_codec(btune_params, BLOSC_LZ4);
    if (0.333333 <= tradeoff_1d) {
      // In BALANCED mode give BLOSCLZ a chance
      add_codec(btune_params, BLOSC_BLOSCLZ);
    }
    if (btune_params->config.perf_mode == BTUNE_PERF_DECOMP) {
      add_codec(btune_params, BLOSC_LZ4HC);
    }
  }
}

static void btune_init_clevels(btune_struct *btune_params, int min, int max, int start) {
  assert(min >= 0 && max <= 9);
  assert(start >= min && start <= max);

  if (btune_params->best) {
    btune_params->best->clevel = start;
  }
  if (btune_params->aux_cparams) {
    btune_params->aux_cparams->clevel = start;
  }

  btune_params->nclevels = max - min + 1;
  for (int i = 0; i < btune_params->nclevels; i++) {
    btune_params->clevels[i] = min + i;
    if (min + i == start) {
        btune_params->clevel_index = i;
    }
  }
}

// Extract the cparams_btune inside blosc2_context
static void extract_btune_cparams(blosc2_context *context, cparams_btune *cparams){
  cparams->compcode = context->compcode;
  cparams->filter = context->filters[BLOSC2_MAX_FILTERS - 1];
  cparams->clevel = context->clevel;
  cparams->splitmode = context->splitmode;
  cparams->blocksize = context->blocksize;
  cparams->nthreads_comp = context->nthreads;
  btune_struct *btune_params = (btune_struct *) context->tuner_params;
  if (btune_params->dctx == NULL) {
    cparams->nthreads_decomp = btune_params->nthreads_decomp;
  } else {
    cparams->nthreads_decomp = btune_params->dctx->nthreads;
  }
}

// Check if btune can still modify the clevel or has to change the direction
static bool has_ended_clevel(btune_struct *btune_params) {
  int max_clevel = btune_params->nclevels;
  int clevel_index = btune_params->clevel_index;
  int step_size = btune_params->step_size;
  return (btune_params->best->increasing_clevel)
    ? (clevel_index + step_size) >= max_clevel
    : (clevel_index - step_size) < 0;
}

// Check if btune can still modify the nthreads or has to change the direction
static bool has_ended_threads(btune_struct *btune_params) {
  cparams_btune * best = btune_params->best;
  int nthreads;
  if (btune_params->threads_for_comp) {
    nthreads = best->nthreads_comp;
  } else {
    nthreads = best->nthreads_decomp;
  }
  return ((best->increasing_nthreads && (nthreads == btune_params->max_threads)) ||
          (!best->increasing_nthreads && (nthreads == MIN_THREADS)));
}

// Init a soft readapt
static void init_soft(btune_struct *btune_params) {
  if (has_ended_clevel(btune_params)) {
    btune_params->best->increasing_clevel = !btune_params->best->increasing_clevel;
  }
  btune_params->state = CLEVEL;
  btune_params->step_size = SOFT_STEP_SIZE;
  btune_params->readapt_from = SOFT;
}

// Init a hard readapt
static void init_hard(btune_struct *btune_params) {
  btune_params->state = CODEC_FILTER;
  btune_params->step_size = HARD_STEP_SIZE;
  btune_params->readapt_from = HARD;
  if (btune_params->config.perf_mode == BTUNE_PERF_DECOMP) {
    btune_params->threads_for_comp = false;
  } else {
    btune_params->threads_for_comp = true;
  }
}

// Init when the number of hard is 0
static void init_without_hards(blosc2_context *ctx) {
  btune_struct *btune_params = (btune_struct*) ctx->tuner_params;
  btune_behaviour behaviour = btune_params->config.behaviour;
  int minimum_hards = 0;
  if (!btune_params->config.cparams_hint) {
    minimum_hards++;
  }
  switch (behaviour.repeat_mode) {
    case BTUNE_REPEAT_ALL:
      if (behaviour.nhards_before_stop > (uint32_t)minimum_hards) {
        init_hard(btune_params);
        break;
      }
    case BTUNE_REPEAT_SOFT:
      if (behaviour.nsofts_before_hard > 0) {
        init_soft(btune_params);
        break;
      }
    case BTUNE_STOP:
      if ((minimum_hards == 0) && (behaviour.nsofts_before_hard > 0)) {
        init_soft(btune_params);
      } else {
        btune_params->state = STOP;
        btune_params->readapt_from = WAIT;
      }
      break;
    default:
      fprintf(stderr, "WARNING: stop mode unknown\n");
  }
  btune_params->is_repeating = true;
}

static const char* stcode_to_stname(btune_struct *btune_params) {
  switch (btune_params->state) {
    case CODEC_FILTER:
      return "CODEC_FILTER";
    case THREADS:
      if (btune_params->threads_for_comp) {
        return "THREADS_COMP";
      } else {
        return "THREADS_DECOMP";
      }
    case CLEVEL:
      return "CLEVEL";
    case MEMCPY:
      return "MEMCPY";
    case WAITING:
      return "WAITING";
    case STOP:
      return "STOP";
    default:
      return "UNKNOWN";
  }
}

static const char* readapt_to_str(readapt_type readapt) {
  switch (readapt) {
    case HARD:
      return "HARD";
    case SOFT:
      return "SOFT";
    case WAIT:
      return "WAIT";
    default:
      return "UNKNOWN";
  }
}

static const char* perf_mode_to_str(btune_performance_mode perf_mode) {
  switch (perf_mode) {
    case BTUNE_PERF_DECOMP:
      return "DECOMP";
    case BTUNE_PERF_BALANCED:
      return "BALANCED";
    case BTUNE_PERF_COMP:
      return "COMP";
    default:
      return "UNKNOWN";
  }
}


static void bandwidth_to_str(char * str, uint32_t bandwidth) {
  if (bandwidth < BTUNE_MBPS) {
    sprintf(str, "%d KB/s", bandwidth);
  } else if (bandwidth < BTUNE_GBPS) {
    sprintf(str, "%d MB/s", bandwidth / BTUNE_KB);
  } else if (bandwidth < BTUNE_TBPS) {
    sprintf(str, "%d GB/s", bandwidth / BTUNE_KB / BTUNE_KB);
  } else {
    sprintf(str, "%d TB/s", bandwidth / BTUNE_KB / BTUNE_KB / BTUNE_KB);
  }
}

static const char* repeat_mode_to_str(btune_repeat_mode repeat_mode) {
  switch (repeat_mode) {
    case BTUNE_REPEAT_ALL:
      return "REPEAT_ALL";
    case BTUNE_REPEAT_SOFT:
      return "REPEAT_SOFT";
    case BTUNE_STOP:
      return "STOP";
    default:
      return "UNKNOWN";
  }
}


// Init btune_struct inside blosc2_context
// TODO CHECK CONFIG ENUMS (bandwidth range...)
int btune_init(void *tuner_params, blosc2_context * cctx, blosc2_context * dctx) {
  btune_config *config = (btune_config *)tuner_params;

  blosc2_init();
  // Register entropy codec
  blosc2_codec codec;
  register_entropy_codec(&codec);

  // Allocate memory
  btune_struct *btune = calloc(sizeof(btune_struct), 1);

  // Configuration
  if (config == NULL) {
    memcpy(&btune->config, &BTUNE_CONFIG_DEFAULTS, sizeof(btune_config));
    config = &btune->config;
  } else {
    memcpy(&btune->config, config, sizeof(btune_config));
  }
  btune->inference_ended = false;

  const char* perf_mode = getenv("BTUNE_PERF_MODE");
  if (btune->config.perf_mode == BTUNE_PERF_AUTO || perf_mode != NULL) {
    if (perf_mode != NULL) {
      if (strcmp(perf_mode, "COMP") == 0) {
        btune->config.perf_mode = BTUNE_PERF_COMP;
      }
      else if (strcmp(perf_mode, "DECOMP") == 0) {
        btune->config.perf_mode = BTUNE_PERF_DECOMP;
      }
      else if (strcmp(perf_mode, "BALANCED") == 0) {
        btune->config.perf_mode = BTUNE_PERF_BALANCED;
      }
      else {
        BTUNE_TRACE("Unsupported %s compression mode, default to COMP", perf_mode);
        btune->config.perf_mode = BTUNE_PERF_COMP;
      }
    }
    else {
      btune->config.perf_mode = BTUNE_PERF_COMP;
    }
  }

  char* envvar = getenv("BTUNE_TRADEOFF");
  if (envvar != NULL) {
    if (strlen(envvar) <= 3) {
      btune->config.tradeoff_nelems = 1;
      btune->config.tradeoff[0] = (float)strtod(envvar, &envvar);
      btune->config.tradeoff[1] = btune->config.tradeoff[2] = 0.0;
    } else {
      // Invalid value or 3d case
      btune->config.tradeoff_nelems = 3;
      for (int i = 0; i < 3; ++i) {
        envvar++;
        btune->config.tradeoff[i] = (float)strtod(envvar, &envvar);
      }
      if (btune->config.tradeoff[2] == 1.0) {
        BTUNE_TRACE("Invalid value for quality tradeoff (found 1.0). Use tradeoff only between"
                    "cratio and speed instead. Aborting.");
        return -1;
      }
    }
  }
  float sum = 0.0;
  for (int i = 0; i < btune->config.tradeoff_nelems; ++i) {
    if (btune->config.tradeoff[i] < 0. || btune->config.tradeoff[i] > 1.) {
      BTUNE_TRACE("Unsupported tradeoff, it must be between 0. and 1., ");
      return -1;
    }
    sum += btune->config.tradeoff[i];
  }
  if (btune->config.tradeoff_nelems == 3 && sum != 1.0) {
    BTUNE_TRACE("In quality mode, tradeoff values must sum up 1.0");
    return -1;
  }

  if (cctx->schunk != NULL) {
    // If the user does not fill the config, the next fields will be empty
    // No need to do the same for dctx because btune is only used during compression
    cctx->schunk->tuner_params = (void *) &btune->config;
    cctx->schunk->storage->cparams->tuner_params = (void *) &btune->config;
  }

  if (getenv("BTUNE_TRACE") != NULL) {
    printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    char bandwidth_str[12];
    bandwidth_to_str(bandwidth_str, btune->config.bandwidth);
    printf("Btune version: %s\n", BTUNE_VERSION_STRING);
    if (btune->config.tradeoff_nelems == 1) {
      printf("Performance Mode: %s, Compression tradeoff: %f, Bandwidth: %s\n",
             perf_mode_to_str(btune->config.perf_mode),
             btune->config.tradeoff[0],
             bandwidth_str);
    } else {
      printf("Performance Mode: %s, Compression tradeoff: (%f, %f, %f), Bandwidth: %s\n",
             perf_mode_to_str(btune->config.perf_mode),
             btune->config.tradeoff[0], btune->config.tradeoff[1], btune->config.tradeoff[2],
             bandwidth_str);
    }
    printf("Behaviour: Waits - %d, Softs - %d, Hards - %d, Repeat Mode - %s\n",
           btune->config.behaviour.nwaits_before_readapt,
           btune->config.behaviour.nsofts_before_hard,
           btune->config.behaviour.nhards_before_stop,
           repeat_mode_to_str(btune->config.behaviour.repeat_mode));
  }

  btune->dctx = dctx;

  // Initialize codecs and filters
  btune_init_codecs(btune);
  add_filter(btune, BLOSC_NOFILTER);
  add_filter(btune, BLOSC_SHUFFLE);
  add_filter(btune, BLOSC_BITSHUFFLE);
  btune->splitmode = BLOSC_AUTO_SPLIT;
  btune_init_clevels(btune, 1, 9, 9);

  // State attributes
  btune->rep_index = 0;
  btune->aux_index = 0;
  btune->steps_count = 0;
  btune->nsofts = 0;
  btune->nhards = 0;
  btune->nwaitings = 0;
  btune->is_repeating = false;
  cctx->tuner_params = btune;

  // Initial compression parameters
  cparams_btune *best = malloc(sizeof(cparams_btune));
  *best = cparams_btune_default;
  btune->best = best;
  cparams_btune *aux = malloc(sizeof(cparams_btune));
  *aux = cparams_btune_default;
  btune->aux_cparams = aux;
  best->compcode = btune->codecs[0];
  aux->compcode = btune->codecs[0];
  float tradeoff_1d = btune->config.tradeoff[0];
  if (btune->config.tradeoff_nelems == 3) {
    tradeoff_1d = btune->config.tradeoff[0] + btune->config.tradeoff[2] / 2;
  }
  if ((float)2/3 <= tradeoff_1d <= 1.) {
    best->clevel = 8;
    aux->clevel = 8;
  }
  best->nthreads_comp = cctx->nthreads;
  aux->nthreads_comp = cctx->nthreads;
  if (dctx != NULL){
    btune->max_threads = (cctx->nthreads > dctx->nthreads) ? cctx->nthreads: dctx->nthreads;
    best->nthreads_decomp = dctx->nthreads;
    aux->nthreads_decomp = dctx->nthreads;
    btune->nthreads_decomp = dctx->nthreads;
  } else {
    btune->max_threads = cctx->nthreads;
    best->nthreads_decomp = cctx->nthreads;
    aux->nthreads_decomp = cctx->nthreads;
    btune->nthreads_decomp = cctx->nthreads;
  }
  // Provide some room for exploration beyond the theoretical maximum
  btune->max_threads += 8;

  // Aux arrays to calculate the mean
  btune->current_cratios = malloc(sizeof(double)) ;
  btune->current_scores = malloc(sizeof(double));

  if (btune->config.perf_mode == BTUNE_PERF_DECOMP) {
    btune->threads_for_comp = false;
  } else {
    btune->threads_for_comp = true;
  }

  // cparams_hint
  if (config->cparams_hint) {
    extract_btune_cparams(cctx, btune->best);
    extract_btune_cparams(cctx, btune->aux_cparams);
    add_codec(btune, cctx->compcode);
    if (btune->config.behaviour.nhards_before_stop > 0) {
      if (btune->config.behaviour.nsofts_before_hard > 0){
        init_soft(btune);
      } else if (btune->config.behaviour.nwaits_before_readapt > 0) {
        btune->state = WAITING;
        btune->readapt_from = WAIT;
      } else {
        init_hard(btune);
      }
    } else {
      init_without_hards(cctx);
    }
  } else {
    init_hard(btune);
    btune->config.behaviour.nhards_before_stop++;
  }
  if (btune->config.behaviour.nhards_before_stop == 1) {
    btune->step_size = SOFT_STEP_SIZE;
  } else {
    btune->step_size = HARD_STEP_SIZE;
  }

  // Initialize inference data
  btune_model_init(cctx);

  return BLOSC2_ERROR_SUCCESS;
}

// Free btune_struct
int btune_free(blosc2_context *context) {
  btune_struct *btune_params = (btune_struct *) context->tuner_params;
  if (btune_params->models_index < 0) {
    btune_model_free(context);
  }
  free(btune_params->best);
  free(btune_params->aux_cparams);
  free(btune_params->current_scores);
  free(btune_params->current_cratios);
  btune_params->interpreter = NULL;
  btune_params->metadata = NULL;
  free(btune_params);
  context->tuner_params = NULL;

  return BLOSC2_ERROR_SUCCESS;
}

// This must exist because unconditionally called by c-blosc2, otherwise there
// will be a crash
int btune_next_blocksize(blosc2_context *context) {
  return BLOSC2_ERROR_SUCCESS;
}

// Set the cparams_btune inside blosc2_context
static void set_btune_cparams(blosc2_context * context, cparams_btune * cparams){
  context->compcode = cparams->compcode;
  context->compcode_meta = cparams->compcode_meta;

  for(int i=0; i < BLOSC2_MAX_FILTERS; i++) {
      context->filters[i] = 0;
  }
  context->filters[BLOSC2_MAX_FILTERS - 1] = cparams->filter;
  // Bytedelta requires a shuffle before it
  if (cparams->filter == BLOSC_FILTER_BYTEDELTA) {
    context->filters[BLOSC2_MAX_FILTERS - 2] = BLOSC_SHUFFLE;
    context->filters_meta[BLOSC2_MAX_FILTERS - 1] = context->typesize;
  } else if (cparams->filter == BLOSC_FILTER_INT_TRUNC) {
    context->filters[BLOSC2_MAX_FILTERS - 2] = BLOSC_FILTER_INT_TRUNC;
    context->filters[BLOSC2_MAX_FILTERS - 1] = BLOSC_BITSHUFFLE;
    context->filters_meta[BLOSC2_MAX_FILTERS - 2] = cparams->filter_meta;
  }

  context->splitmode = cparams->splitmode;
  context->clevel = cparams->clevel;
  btune_struct *btune_params = (btune_struct*) context->tuner_params;

  if (cparams->blocksize) {
    context->blocksize = cparams->blocksize;
  }
  context->new_nthreads = (int16_t) cparams->nthreads_comp;
  if (btune_params->dctx != NULL) {
    btune_params->dctx->new_nthreads = (int16_t) cparams->nthreads_decomp;
  } else {
    btune_params->nthreads_decomp = cparams->nthreads_decomp;
  }
}

// Meant for lossy mode, returns whether to use the neural network or not and in that case, fills the compression params
bool pred_comp_category(btune_struct *btune_params, int *compcode, uint8_t *compmeta, uint8_t *filter,
                        uint8_t *filter_meta, int *clevel, int32_t *splitmode) {
  if (btune_params->config.tradeoff_nelems == 1) {
    // Use neural network
    return true;
  }

  bool use_model = false;
  float cratio = btune_params->config.tradeoff[0];
  float ssim = btune_params->config.tradeoff[2];
  if (0.6f <= cratio) {
    *compcode = BLOSC_CODEC_GROK;
    if (ssim <= 0.3f) {
      // codec= grok, rates = 8
      *compmeta = 8 * 10;
    } else {
      // codec= grok, rates = 5
      *compmeta = 5 * 10;
    }
  } else {
    if (0.3f <= cratio && cratio <= 0.6f) {
      if (0.3f <= ssim && ssim <= 0.6f) {
        //grok, rates 4
        *compcode = BLOSC_CODEC_GROK;
        *compmeta = 4 * 10;
      } else {
        if (ssim < 0.3f) {
          // grok, 8
          *compcode = BLOSC_CODEC_GROK;
          *compmeta = 8 * 10;
        } else {
          // neural network
          return true;
        }
      }

    } else {
      if (0.7f <= ssim && ssim < 1.0f) {
        // itrunc16-bitshuffle-8
        *compcode = BLOSC_ZSTD;
        *filter = BLOSC_FILTER_INT_TRUNC;  // + bitshuffle
        *filter_meta = 8;
        *splitmode = BLOSC_ALWAYS_SPLIT;
        *clevel = 3;
      }
      else {
        // neural network
        return true;
      }
    }
  }

  return use_model;
}

// Meant for lossy mode, returns whether to use the neural network or not and in that case, fills the compression params
bool pred_decomp_category(btune_struct *btune_params, int *compcode, uint8_t *compmeta, uint8_t *filter,
                          uint8_t *filter_meta, int *clevel, int32_t *splitmode) {
  if (btune_params->config.tradeoff_nelems == 1) {
    // Use neural network
    return true;
  }

  bool use_model = false;
  float cratio = btune_params->config.tradeoff[0];
  float speed = btune_params->config.tradeoff[1];
  float ssim = btune_params->config.tradeoff[2];
  if (cratio <= 0.1f) {
    return true;
  }
  if (cratio >= 0.6f) {
    *compcode = BLOSC_CODEC_GROK;
    if (ssim <= 0.2f) {
      *compmeta = 8 * 10;
    } else {
      if (0.6f < cratio && cratio <= 0.7f) {
        *compmeta = 7 * 10;
      } else {
        *compmeta = 5 * 10;
      }
    }
    return use_model;
  }
  if (cratio <= 0.3f) {
    if (speed == 0.f) {
      *compcode = BLOSC_CODEC_GROK;
      *compmeta = 3 * 10;
    } else {
      if (speed <= 0.1f) {
        // lossy integers images
        *compcode = BLOSC_ZSTD;
        *filter = BLOSC_FILTER_INT_TRUNC; //itrunc bitshuf 7
        *filter_meta = 7;
        *splitmode = BLOSC_ALWAYS_SPLIT;
        *clevel = 3;
      } else {
        use_model = true;
      }
    }
    return use_model;
  }
  if (cratio + ssim >= 0.9f) {
    *compcode = BLOSC_CODEC_GROK;
    *compmeta = 4 * 10;
  } else {
    if (cratio + ssim >= 0.8f) {
      *compcode = BLOSC_ZSTD;
      *filter = BLOSC_FILTER_INT_TRUNC; //itrunc bitshuff 6
      *filter_meta = 6;
      *splitmode = BLOSC_ALWAYS_SPLIT;
      *clevel = 3;
    } else {
      if (cratio <= 0.4f && ssim >= 0.1f) {
        use_model = true;
      } else {
        *compcode = BLOSC_CODEC_GROK;
        *compmeta = 8 * 10;
      }
    }
  }

  return use_model;
}

int tweaking_next_cparams(cparams_btune *cparams, btune_struct *btune_params, bool use_model,
                          int error, int compcode, uint8_t compmeta, uint8_t filter, uint8_t filter_meta, int clevel,
                          int32_t splitmode) {
  cparams->compcode_meta = compmeta;
  cparams->filter_meta = filter_meta;
  if (!use_model) {
    if (btune_params->state == STOP){
      return BLOSC2_ERROR_SUCCESS; // Tuning ended
    }
    cparams->clevel = clevel;
    cparams->compcode = compcode;
    cparams->filter = filter;
    cparams->splitmode = splitmode;
    if (btune_params->state == CODEC_FILTER) {
      btune_params->aux_index++;
      return 1; // Continue tuning parameters
    }
    if (cparams->compcode == BLOSC_CODEC_GROK && btune_params->state == CLEVEL) {
      btune_params->aux_index++;
      return 1; // Continue tuning parameters
    }
  }

  btune_config config = btune_params->config;
  switch(btune_params->state){
    // Tune codec and filter
    case CODEC_FILTER: {
      // Cycle codecs, filters and splits
      int n_filters_splits = btune_params->nfilters * 2;
      cparams->compcode = btune_params->codecs[btune_params->aux_index / n_filters_splits];
      cparams->filter = btune_params->filters[(btune_params->aux_index % n_filters_splits) / 2];

      if (btune_params->splitmode == BLOSC_AUTO_SPLIT) {
        cparams->splitmode = (btune_params->aux_index % 2) + 1;
      }
      else {
        cparams->splitmode = btune_params->splitmode;
      }

      btune_performance_mode perf_mode = config.perf_mode;
      if (error == 0) {
        cparams->clevel = clevel;
      }
      else {
        // The first tuning of ZSTD in some modes should start in clevel 3
        if (
          (perf_mode == BTUNE_PERF_COMP || perf_mode == BTUNE_PERF_BALANCED) &&
          (cparams->compcode == BLOSC_ZSTD || cparams->compcode == BLOSC_ZLIB) &&
          (btune_params->nhards == 0)
          ) {
          cparams->clevel = 3;
        }
      }

      if (btune_params->inference_ended) {
        btune_params->aux_index++;
      }
      break;
    }

      // Tune the number of threads
    case THREADS:
      btune_params->aux_index++;
      int * nthreads;
      if (btune_params->threads_for_comp) {
        nthreads = &cparams->nthreads_comp;
      } else {
        nthreads = &cparams->nthreads_decomp;
      }
      if (cparams->increasing_nthreads) {
        if (*nthreads < btune_params->max_threads) {
          *nthreads += 2;
        }
      } else {
        if (*nthreads > MIN_THREADS) {
          *nthreads -= 2;
        }
      }
      break;

      // Tune compression level
    case CLEVEL:
      btune_params->aux_index++;

      if (!has_ended_clevel(btune_params)) {
        if (cparams->increasing_clevel) {
          btune_params->clevel_index += btune_params->step_size;
        }
        else {
          btune_params->clevel_index -= btune_params->step_size;
        }
      }

      cparams->clevel = btune_params->clevels[btune_params->clevel_index];
      if (cparams->clevel == 9 && cparams->compcode == BLOSC_ZSTD) {
        cparams->clevel = 8;
      }
      break;

      // Try without compressing
    case MEMCPY:
      btune_params->aux_index++;
      cparams->clevel = 0;
      break;

      // Waiting
    case WAITING:
      btune_params->nwaitings++;
      break;

      // Stopped
    case STOP:
      return BLOSC2_ERROR_SUCCESS;
  }

  return 1; // Continue tuning parameters
}

// Tune some compression parameters based on the context
int btune_next_cparams(blosc2_context *context) {
  btune_struct *btune_params = (btune_struct*) context->tuner_params;
  btune_config config = btune_params->config;
  int compcode;
  uint8_t compmeta = 0;
  uint8_t filter = BLOSC_NOFILTER;
  uint8_t filter_meta = 0;
  int clevel = 5;
  int32_t splitmode = BLOSC_NEVER_SPLIT;
  int error = -1;

  bool use_model;
  if (config.perf_mode == BTUNE_PERF_DECOMP) {
    use_model = pred_decomp_category(btune_params, &compcode, &compmeta, &filter, &filter_meta, &clevel, &splitmode);
  } else {
    use_model = pred_comp_category(btune_params, &compcode, &compmeta, &filter, &filter_meta, &clevel, &splitmode);
  }

  if (use_model) {
    if (btune_params->inference_count != 0) {
      if (btune_params->inference_count > 0) {
        btune_params->inference_count--;
      }

      error = btune_model_inference(context, &compcode, &filter, &clevel, &splitmode);
    } else {
      if (!btune_params->inference_ended){
        error = most_predicted(btune_params, &compcode, &filter, &clevel, &splitmode);
        btune_params->inference_ended = true;
      }
    }
  }

  if (error == 0) {
    btune_params->codecs[0] = compcode;
    btune_params->ncodecs = 1;
    btune_params->filters[0] = filter;
    btune_params->nfilters = 1;
    if (config.perf_mode == BTUNE_PERF_DECOMP) {
      btune_init_clevels(btune_params, clevel, clevel, clevel);
    }
    else {
      int min = (clevel > 1) ? (clevel - 1) : clevel;
      int max = (clevel < 9) ? (clevel + 1) : clevel;
      btune_init_clevels(btune_params, min, max, clevel);
    }
    btune_params->splitmode = splitmode;
  }


  if (getenv("BTUNE_TRACE") && btune_params->steps_count == 0 && btune_params->state != STOP) {
    // printf("|    Codec   | Filter | Split | C.Level | Blocksize | C.Threads | D.Threads |"
    printf("|    Codec   | Filter | Split | C.Level | C.Threads | D.Threads |"
           "  S.Score  |  C.Ratio   |   Btune State   | Readapt | Winner\n");
  }

  *btune_params->aux_cparams = *btune_params->best;
  cparams_btune *cparams = btune_params->aux_cparams;

  int rc = tweaking_next_cparams(cparams, btune_params, use_model, error, compcode, compmeta, filter, filter_meta,
                                 clevel, splitmode);
  if (rc == 0) {
    // Stop tuning
    return rc;
  }

  set_btune_cparams(context, cparams);
  if (context->blocksize > context->sourcesize) {
    // blocksize cannot be greater than sourcesize
    context->blocksize = context->sourcesize;
  }

  return BLOSC2_ERROR_SUCCESS;
}

// Computes the score depending on the perf_mode
static double score_function(btune_struct *btune_params, double ctime, size_t cbytes,
                             double dtime) {
  double reduced_cbytes = (double)cbytes / (double) BTUNE_KB;
  switch (btune_params->config.perf_mode) {
    case BTUNE_PERF_COMP:
      return ctime + reduced_cbytes / btune_params->config.bandwidth;
    case BTUNE_PERF_DECOMP:
      return reduced_cbytes / btune_params->config.bandwidth + dtime;
    case BTUNE_PERF_BALANCED:
      return ctime + reduced_cbytes / btune_params->config.bandwidth + dtime;
    default:
      fprintf(stderr, "WARNING: unknown performance mode\n");
      return -1;
  }
}

static double mean(double const * array, int size) {
  double sum = 0;
  for (int i = 0; i < size; i++) {
    sum += array[i];
  }
  return sum / size;
}

// Determines if btune has improved depending on the tradeoff
static bool has_improved(btune_struct *btune_params, double score_coef, double cratio_coef) {
  float tradeoff_1d = btune_params->config.tradeoff[0];
  if (btune_params->config.tradeoff_nelems == 3) {
    tradeoff_1d = btune_params->config.tradeoff[0] + btune_params->config.tradeoff[2] / 2;
  }
  if (tradeoff_1d <= 1/3) {
    return (((cratio_coef >= 1) && (score_coef > 1)) ||
            ((cratio_coef > 0.5) && (score_coef > 2)) ||
            ((cratio_coef > 0.67) && (score_coef > 1.3)) ||
            ((cratio_coef > 2) && (score_coef > 0.7)));
  }
  if (tradeoff_1d <= 2/3) {
    return (((cratio_coef >= 1) && (score_coef > 1)) ||
            ((cratio_coef > 1.1) && (score_coef > 0.8)) ||
            ((cratio_coef > 1.3) && (score_coef > 0.5)));
  }
  if (tradeoff_1d <= 1.) {
    return ((cratio_coef >= 1) && (score_coef > 1));
  }
  fprintf(stderr, "WARNING: unknown tradeoff, it must be between 0. and 1.0\n");
  return false;
}


// Processes which btune_state will come next after a readapt or wait
static void process_waiting_state(blosc2_context *ctx) {
  btune_struct *btune_params = (btune_struct*) ctx->tuner_params;
  btune_behaviour behaviour = btune_params->config.behaviour;
  uint32_t minimum_hards = 0;

  if (!btune_params->config.cparams_hint) {
    minimum_hards++;
  }

  char* envvar = getenv("BTUNE_TRACE");
  if (envvar != NULL) {
    // Print the winner of the readapt
//  if (btune_params->readapt_from != WAIT && !btune_params->is_repeating) {
//    char* compname;
//    blosc_compcode_to_compname(cparams->compcode, &compname);
//    printf("| %10s | %d | %d | %d | %d | %d | %.3g | %.3gx | %s\n",
//           compname, cparams->filter, cparams->clevel,
//           (int) cparams->blocksize / BTUNE_KB,
//           cparams->nthreads_comp, cparams->nthreads_decomp,
//           cparams->score, cparams->cratio, "WINNER");
//  }
  }

  switch (btune_params->readapt_from) {
    case HARD:
      btune_params->nhards++;
      assert(btune_params->nhards > 0);
      // Last hard (initial readapts completed)
      if ((behaviour.nhards_before_stop == minimum_hards) ||
          (btune_params->nhards % behaviour.nhards_before_stop == 0)) {
        btune_params->is_repeating = true;
        // There are softs (repeat_mode no stop)
        if ((behaviour.nsofts_before_hard > 0) &&
            (behaviour.repeat_mode != BTUNE_STOP)) {
          init_soft(btune_params);
          // No softs (repeat_mode soft)
        } else if (behaviour.repeat_mode != BTUNE_REPEAT_ALL) {
          btune_params->state = STOP;
          // No softs, there are waits (repeat_mode all)
        } else if (behaviour.nwaits_before_readapt > 0) {
          btune_params->state = WAITING;
          btune_params->readapt_from = WAIT;
          // No softs, no waits and there are hards (repeat_mode all)
        } else if (behaviour.nhards_before_stop > minimum_hards) {
          init_hard(btune_params);
          // No softs, no waits no hards (repeat_mode all)
        } else {
          btune_params->state = STOP;
        }
        // Not the last hard (there are softs readapts)
      } else if (behaviour.nsofts_before_hard > 0) {
        init_soft(btune_params);
        // No softs but there are waits
      } else if (behaviour.nwaits_before_readapt > 0) {
        btune_params->state = WAITING;
        btune_params->readapt_from = WAIT;
        // No softs, no waits
      } else {
        init_hard(btune_params);
      }
      break;

    case SOFT:
      btune_params->nsofts++;
      btune_params->readapt_from = WAIT;
      assert(btune_params->nsofts > 0);
      if (behaviour.nwaits_before_readapt == 0) {
        // Last soft
        if (((behaviour.nsofts_before_hard == 0) ||
             (btune_params->nsofts % behaviour.nsofts_before_hard == 0)) &&
            !(btune_params->is_repeating && (behaviour.repeat_mode != BTUNE_REPEAT_ALL)) &&
            (behaviour.nhards_before_stop > minimum_hards)) {
          init_hard(btune_params);
          // Special, hint true, no hards, last soft, stop_mode
        } else if ((minimum_hards == 0) &&
                   (behaviour.nhards_before_stop == 0) &&
                   (btune_params->nsofts % behaviour.nsofts_before_hard == 0) &&
                   (behaviour.repeat_mode == BTUNE_STOP)) {
          btune_params->is_repeating = true;
          btune_params->state = STOP;
          // Not the last soft
        } else {
          init_soft(btune_params);
        }
      }
      break;

    case WAIT:
      // Last wait
      if ((behaviour.nwaits_before_readapt == 0) ||
          ((btune_params->nwaitings != 0) &&
           (btune_params->nwaitings % behaviour.nwaits_before_readapt == 0))) {
        // Last soft
        if (((behaviour.nsofts_before_hard == 0) ||
             ((btune_params->nsofts != 0) &&
              (btune_params->nsofts % behaviour.nsofts_before_hard == 0))) &&
            !(btune_params->is_repeating && (behaviour.repeat_mode != BTUNE_REPEAT_ALL)) &&
            (behaviour.nhards_before_stop > minimum_hards)) {

          init_hard(btune_params);
          // Not last soft
        } else if ((behaviour.nsofts_before_hard > 0) &&
                   !(btune_params->is_repeating && (behaviour.repeat_mode == BTUNE_STOP))){

          init_soft(btune_params);
        }
      }
  }
  // Force soft step size on last hard
  if ((btune_params->readapt_from == HARD) &&
      (btune_params->nhards == (int)(behaviour.nhards_before_stop - 1))) {
    btune_params->step_size = SOFT_STEP_SIZE;
  }
}

// State transition handling
static void update_aux(blosc2_context * ctx, bool improved) {
  btune_struct *btune_params = (btune_struct *) ctx->tuner_params;
  cparams_btune *best = btune_params->best;
  bool first_time = btune_params->aux_index == 1;
  switch (btune_params->state) {
    case CODEC_FILTER: {
      // Reached last combination of codec filter
      int aux_index_max = btune_params->ncodecs *  btune_params->nfilters;
      if (btune_params->splitmode == BLOSC_AUTO_SPLIT) {
        aux_index_max *= 2;
      }

      if (btune_params->aux_index >= aux_index_max) {
        btune_params->aux_index = 0;

        btune_params->state = BTUNE_ENABLE_THREADS ? THREADS : CLEVEL;

        // max_threads must be greater than 1
        if ((btune_params->state == THREADS) && (btune_params->max_threads == 1)) {
          btune_params->state = CLEVEL;
          if (has_ended_clevel(btune_params)) {
            best->increasing_clevel = !best->increasing_clevel;
          }
        }
        // Control direction parameters
        if (btune_params->state == THREADS) {
          best->increasing_nthreads = !best->increasing_nthreads;
        }
      }
      break;
    }

    case THREADS:
      first_time = (btune_params->aux_index % MAX_STATE_THREADS) == 1;
      if (!improved && first_time) {
        best->increasing_nthreads = !best->increasing_nthreads;
      }
      // Can not change parameter or is not improving
      if (has_ended_threads(btune_params) || (!improved && !first_time)) {
        // If perf_mode BALANCED mark btune_params to change threads for decompression
        if (btune_params->config.perf_mode == BTUNE_PERF_BALANCED) {
          if (btune_params->aux_index < MAX_STATE_THREADS) {
            btune_params->threads_for_comp = !btune_params->threads_for_comp;
            btune_params->aux_index = MAX_STATE_THREADS;
            if (has_ended_threads(btune_params)) {
              best->increasing_nthreads = !best->increasing_nthreads;
            }
          }
        // No BALANCED mark to end
        } else {
          btune_params->aux_index = MAX_STATE_THREADS + 1;
        }
        // THREADS ended
        if (btune_params->aux_index > MAX_STATE_THREADS) {
          btune_params->aux_index = 0;
          btune_params->state = CLEVEL;
          if (has_ended_clevel(btune_params)) {
            best->increasing_clevel = !best->increasing_clevel;
          }
        }
      }
      break;

    case CLEVEL:
      if (!improved && first_time) {
        best->increasing_clevel = !best->increasing_clevel;
      }
      // Can not change parameter or is not improving
      if (has_ended_clevel(btune_params) || (!improved && !first_time)) {
        btune_params->aux_index = 0;
        btune_params->state = BTUNE_ENABLE_MEMCPY ? MEMCPY : WAITING;
      }
      break;

    case MEMCPY:
      btune_params->aux_index = 0;
      btune_params->state = WAITING;
      break;

    default:
      ;
  }
  if (btune_params->state == WAITING) {
    process_waiting_state(ctx);
  }
}

// Update btune structs with the compression results
int btune_update(blosc2_context * context, double ctime) {
  btune_struct *btune_params = (btune_struct*)(context->tuner_params);
  if (btune_params->state == STOP) {
    return BLOSC2_ERROR_SUCCESS;
  }

  btune_params->steps_count++;
  cparams_btune * cparams = btune_params->aux_cparams;

  // We come from blosc_compress_context(), so we can populate metrics now
  size_t cbytes = context->destsize;
  double dtime = 0;

  // Compute the decompression time if needed
  btune_behaviour behaviour = btune_params->config.behaviour;
  blosc_timestamp_t last, current;
  if (!((btune_params->state == WAITING) &&
      ((behaviour.nwaits_before_readapt == 0) ||
      (btune_params->nwaitings % behaviour.nwaits_before_readapt != 0))) &&
      ((btune_params->config.perf_mode == BTUNE_PERF_DECOMP) ||
      (btune_params->config.perf_mode == BTUNE_PERF_BALANCED)) &&
       // When the source is NULL (eval with prefilters), decompression is not working.
       context->dest != NULL) {
    blosc2_context * dctx;
    if (btune_params->dctx == NULL) {
      blosc2_dparams params = { btune_params->nthreads_decomp, NULL, NULL, NULL};
      dctx = blosc2_create_dctx(params);
    } else {
      dctx = btune_params->dctx;
    }
    blosc_set_timestamp(&last);
    blosc2_decompress_ctx(dctx, context->dest, context->destsize, (void*)(context->src),
                          context->sourcesize);
    blosc_set_timestamp(&current);
    dtime = blosc_elapsed_secs(last, current);
    if (btune_params->dctx == NULL) {
      blosc2_free_ctx(dctx);
    }
  }

  double score = score_function(btune_params, ctime, cbytes, dtime);
  assert(score > 0);
  double cratio = (double) context->sourcesize / (double) cbytes;

  cparams->score = score;
  cparams->cratio = cratio;
  cparams->ctime = ctime;
  cparams->dtime = dtime;
  btune_params->current_scores[btune_params->rep_index] = score;
  btune_params->current_cratios[btune_params->rep_index] = cratio;
  btune_params->rep_index++;
  if (btune_params->rep_index == 1) {
    score = mean(btune_params->current_scores, 1);
    cratio = mean(btune_params->current_cratios, 1);
    double cratio_coef = cratio / btune_params->best->cratio;
    double score_coef = btune_params->best->score / score;
    bool improved;
    // In state THREADS the improvement comes from ctime or dtime
    if (btune_params->state == THREADS) {
      if (btune_params->threads_for_comp) {
        improved = ctime < btune_params->best->ctime;
      } else {
        improved = dtime < btune_params->best->dtime;
      }
    } else {
      improved = has_improved(btune_params, score_coef, cratio_coef);
    }
    char winner = '-';
    // If the chunk is made of special values, it cannot never improve scoring
    if (cbytes <= (BLOSC2_MAX_OVERHEAD + (size_t)context->typesize)) {
      improved = false;
      winner = 'S';
    }
    if (improved) {
      winner = 'W';
    }

    if (!btune_params->is_repeating) {
      char* envvar = getenv("BTUNE_TRACE");
      if (envvar != NULL) {
        int split = (cparams->splitmode == BLOSC_ALWAYS_SPLIT) ? 1 : 0;
        const char *compname;
        blosc2_compcode_to_compname(cparams->compcode, &compname);
        // Disable printing the Blocksize because it is not currently used
        //printf("| %10s | %6d | %5d | %7d | %9d | %11d | %9d | %9d | %9.3g | %9.3gx | %15s | %7s | %c\n",
        // We also use the inverse of the score to make it easier to read
        printf("| %10s | %6d | %5d | %7d | %9d | %9d | %9.3g | %9.3gx | %15s | %7s | %c\n",
               compname, cparams->filter, split, cparams->clevel,
               //(int) cparams->blocksize / BTUNE_KB,
               cparams->nthreads_comp, cparams->nthreads_decomp,
               (double) context->sourcesize / (score * (int)(1<<30)), cratio,
               stcode_to_stname(btune_params),
               readapt_to_str(btune_params->readapt_from), winner);
      }
    }

    // We don't want to get rid of the previous best->score
    if (improved) {
      *btune_params->best = *cparams;
    }
    btune_params->rep_index = 0;
    update_aux(context, improved);
  }

  return BLOSC2_ERROR_SUCCESS;
}


int set_params_defaults(
  uint32_t bandwidth,
  uint32_t perf_mode,
  float *tradeoff,
  int tradeoff_nelems,
  bool cparams_hint,
  int use_inference,
  const char* models_dir,
  uint32_t nwaits,
  uint32_t nsofts,
  uint32_t nhards,
  uint32_t repeat_mode
) {
  BTUNE_CONFIG_DEFAULTS.bandwidth = bandwidth;
  BTUNE_CONFIG_DEFAULTS.perf_mode = perf_mode;
  BTUNE_CONFIG_DEFAULTS.tradeoff_nelems = tradeoff_nelems;
  if (tradeoff_nelems == 1) {
    BTUNE_CONFIG_DEFAULTS.tradeoff[0] = tradeoff[0];
    BTUNE_CONFIG_DEFAULTS.tradeoff[1] = BTUNE_CONFIG_DEFAULTS.tradeoff[2] = 0.0;

  } else {
    for (int i = 0; i < tradeoff_nelems; ++i) {
      BTUNE_CONFIG_DEFAULTS.tradeoff[i] = tradeoff[i];
    }
  }

  BTUNE_CONFIG_DEFAULTS.cparams_hint = cparams_hint;
  BTUNE_CONFIG_DEFAULTS.use_inference = use_inference;
  strcpy(BTUNE_CONFIG_DEFAULTS.models_dir, models_dir);
  BTUNE_CONFIG_DEFAULTS.behaviour.nwaits_before_readapt = nwaits;
  BTUNE_CONFIG_DEFAULTS.behaviour.nsofts_before_hard = nsofts;
  BTUNE_CONFIG_DEFAULTS.behaviour.nhards_before_stop = nhards;
  BTUNE_CONFIG_DEFAULTS.behaviour.repeat_mode = repeat_mode;

  return 0;
}

void btune_free_all_models(void) {
  g_models_free();
}

void btune_set_reuse_models(bool new_value) {
  set_reuse_models(new_value);
}
