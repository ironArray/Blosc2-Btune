/*********************************************************************
  Btune for Blosc2 - Automatically choose the best codec/filter for your data

  Copyright (c) 2023 The Blosc Developers <blosc@blosc.org>
  https://btune.blosc.org
  License: GNU Affero General Public License v3.0
  See LICENSE.txt for details about copyright and rights to use.
**********************************************************************/

/** @file  btune.h
 * @brief Btune header file.
 *
 * This file contains the public methods and structures needed to use Btune.
 */

#ifndef BTUNE_H
#define BTUNE_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


#if defined(_WIN32)
#include <windows.h>
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#else
#include <limits.h>
#endif

// The size of L1 cache.  32 KB is quite common nowadays.
#define L1 (32 * 1024)
// Version numbers
#define BTUNE_VERSION_MAJOR    1    /* for major interface/format changes  */
#define BTUNE_VERSION_MINOR    2    /* for minor interface/format changes  */
#define BTUNE_VERSION_RELEASE  0    /* for tweaks, bug-fixes, or development */
#define BTUNE_VERSION_STRING "1.2.0"
// Maximum number of codecs
#define BTUNE_MAX_CODECS 8
#define BTUNE_MAX_FILTERS 3
#define BTUNE_MAX_CLEVELS 9

#define BTUNE_TRACE(msg, ...) \
    do { \
         const char *__e = getenv("BTUNE_TRACE"); \
         if (!__e) { break; } \
         fprintf(stderr, "TRACE: " msg "\n", ##__VA_ARGS__); \
       } while(0)


/**
 * @brief Btune units enumeration.
 *
 * This enumeration provides the most common units of bandwidth for its use
 * in the Btune config. The bandwidth units are expressed in kB/s.
*/
enum bandwidth_units{
  BTUNE_MBPS = 1024,                                  //!< A 1 MB/s bandwidth expressed in kB/s, 1024 kB/s.
  BTUNE_MBPS10 = 10 * BTUNE_MBPS,                     //!< A 10 MB/s bandwidth expressed in kB/s, 10240 kB/s.
  BTUNE_MBPS100 = 100 * BTUNE_MBPS,                   //!< A 100 MB/s bandwidth expressed in kB/s, 102400 kB/s.
  BTUNE_GBPS = 1 * BTUNE_MBPS * BTUNE_MBPS,           //!< A 1 GB/s bandwidth expressed in kB/s, 1024^2 kB/s.
  BTUNE_GBPS10 = 10 * BTUNE_MBPS * BTUNE_MBPS,        //!< A 10 GB/s bandwidth expressed in kB/s, 10 * 1024^2 kB/s.
  BTUNE_GBPS100 = 100 * BTUNE_MBPS * BTUNE_MBPS,      //!< A 100 GB/s bandwidth expressed in kB/s, 100 * 1024^2 kB/s.
  BTUNE_TBPS = BTUNE_MBPS * BTUNE_MBPS * BTUNE_MBPS,  //!< A 1 TB/s bandwidth expressed in kB/s, 1024^3 kB/s.
};

/**
 * @brief Compression mode enumeration.
 *
 * The compression mode alters the Btune criteria for improvement.
 * Depending on this value Btune will prioritize the compression/decompression speed,
 * the compression ratio or both.
*/
#define BTUNE_COMP_HSP 0.1F      //!< Optimizes the speed, even accepting memcpy.
#define BTUNE_COMP_BALANCED 0.5F  //!< Optimizes both, the speed and compression ratio.
#define BTUNE_COMP_HCR 0.9F       //!< Optimizes the compression ratio.


/**
 * @brief Performance mode enumeration.
 *
 * The performance mode alters the Btune scoring function used for improvement.
 * Depending on this value Btune will consider for the scoring either the compression time or
 * the decompression time, or both.
*/
typedef enum {
  BTUNE_PERF_COMP,     //!< Optimizes the compression and transmission times.
  BTUNE_PERF_DECOMP,   //!< Optimizes the decompression and transmission times.
  BTUNE_PERF_BALANCED, //!< Optimizes the compression, transmission and decompression times.
  BTUNE_PERF_AUTO,     //!< Gets mode from environment variable, defaults to PERF_COMP
} btune_performance_mode;

/**
 * @brief Repeat mode enumeration.
 *
 * Changes the way Btune behaves when it has completed all the initial readaptations.
 * @see #btune_behaviour
*/
typedef enum {
  BTUNE_STOP,         //!< Btune will stop improving.
  BTUNE_REPEAT_SOFT,  //!< Btune will repeat only the soft readapts continuously.
  BTUNE_REPEAT_ALL,   //!< Btune will repeat the initial readaptations continuously.
} btune_repeat_mode;

/**
 * @brief Btune behaviour struct.
 *
 * This specifies the number of initial hard readapts,
 * the number of soft readapts between each hard readapt and the number of waits,
 * before initiating a readapt.
 * Note: a readapt is the process by which btune adjusts the compression parameters.
 * It can be of two types: \b soft, which only changes the compression level and
 * blocksize or \b hard, which also changes the codec, filters and number of threads.
*/
typedef struct {
  uint32_t nwaits_before_readapt;
  /**< Number of waiting states before a readapt.
   *
   * During a waiting state Btune will not alter any compression parameter.
  */
  uint32_t nsofts_before_hard;
  //!< Number of soft readapts before a hard readapt.
  uint32_t nhards_before_stop;
  //!< Number of initial hard readapts.
  btune_repeat_mode repeat_mode;
  /**< Btune repeat mode.
   *
   * Once completed the initial hard readapts, the repeat mode will determine
   * if Btune continues repeating readapts or stops permanently.
  */
} btune_behaviour;

/**
 * @brief Btune configuration struct.
 *
 * The btune_config struct contains all the parameters used by Btune which determine
 * how the compression parameters will be tuned.
*/
typedef struct {
  uint32_t bandwidth;
  /**< The bandwidth to which optimize in kB/s.
   *
   * Used to calculate the transmission times.
  */
  btune_performance_mode perf_mode;
  //!< The Btune performance mode.
  float tradeoff[3];
  //!< The Btune compression mode (between 0 (speed) and 1 (cratio)).
  int tradeoff_nelems;
  // Number of values for tradeoff (3 or 1). This is automatically when using BTUNE_TRADEOFF or setting it through python.
  btune_behaviour behaviour;
  //!< The Btune behaviour config.
  bool cparams_hint;
  /**< Whether use the cparams specified in the context or not.
   *
   * When true, this will force Btune to use the cparams provided inside the context, note
   * that after a hard readapt the cparams will change.
   * When false, Btune will start from a hard readapt to determine the best cparams, note
   * that this hard readapt is not considered for the number of initial hard readapts.
   * @see #btune_behaviour
  */
  int use_inference;
  //!< Number of times inference is applied. If -1, always apply inference.
  char models_dir[PATH_MAX];
  //!< The directory where the desired models and meta to use are stored.

} btune_config;

/**
 * @brief Btune default configuration.
 *
 * This default configuration of Btune is meant for optimizing memory bandwidth, compression speed,
 * decompression speed and the compression ratio (BALANCED options). It behaves as follows:
 * it starts with a hard readapt (cparams_hint false) and then repeats 5 soft readapts and
 * a hard readapt 1 times before stopping completely.
 *
 * When changing these, the ones in blosc2_btune/__init__.py should also be changed.
*/
static btune_config BTUNE_CONFIG_DEFAULTS = {
    2 * BTUNE_GBPS10,
    BTUNE_PERF_AUTO,
    {BTUNE_COMP_BALANCED, 0, 0},
    1,
    {0, 5, 10, BTUNE_STOP},
    false,
    -1,
    {0},
};

/// @cond DEV
// Internal Btune state enumeration.
typedef enum {
    CODEC_FILTER,
    THREADS,
    CLEVEL,
    MEMCPY,
    WAITING,
    STOP,
} btune_state;

// Internal Btune readapt type
typedef enum {
    WAIT,
    SOFT,
    HARD,
} readapt_type;


#endif  /* BTUNE_H */
