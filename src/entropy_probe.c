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
#include <stdint.h>
#include <string.h>

#include <blosc2.h>
#include "entropy_probe.h"


#define MAX_COPY 32U
#define MAX_DISTANCE 8191
#define MAX_FARDISTANCE (65535 + MAX_DISTANCE - 1)

// The hash length (1 << HASH_LOG) can be tuned for performance (12 -> 15)
#define HASH_LOG (13U)

#define HASH_FUNCTION(v, s, h)              \
    {                                       \
        v = (s * 2654435761U) >> (32U - h); \
    }

#define BLOSCLZ_READU16(p) *((const uint16_t *)(p))
#define BLOSCLZ_READU32(p) *((const uint32_t *)(p))

#define LITERAL2(ip, anchor, copy) \
    {                              \
        oc++;                      \
        anchor++;                  \
        ip = anchor;               \
        copy++;                    \
        if (copy == MAX_COPY) {    \
            copy = 0;              \
            oc++;                  \
        }                          \
    }

static uint8_t *get_run(uint8_t *ip, const uint8_t *ip_bound, const uint8_t *ref) {
  uint8_t x = ip[-1];
  int64_t value, value2;
  /* Broadcast the value for every byte in a 64-bit register */
  memset(&value, x, 8);
  /* safe because the outer check against ip limit */
  while (ip < (ip_bound - sizeof(int64_t))) {
    value2 = ((int64_t *) ref)[0];
    if (value != value2) {
      /* Return the byte that starts to differ */
      while (*ref++ == x)
        ip++;
      return ip;
    } else {
      ip += 8;
      ref += 8;
    }
  }
  /* Look into the remainder */
  while ((ip < ip_bound) && (*ref++ == x))
    ip++;
  return ip;
}

static uint8_t *get_match(uint8_t *ip, const uint8_t *ip_bound, const uint8_t *ref) {
  while (ip < (ip_bound - sizeof(int64_t))) {
    if (*(int64_t *) ref != *(int64_t *) ip) {
      /* Return the byte that starts to differ */
      while (*ref++ == *ip++) {
      }
      return ip;
    } else {
      ip += sizeof(int64_t);
      ref += sizeof(int64_t);
    }
  }
  /* Look into the remainder */
  while ((ip < ip_bound) && (*ref++ == *ip++)) {
  }
  return ip;
}

static uint8_t *get_run_or_match(uint8_t *ip, uint8_t *ip_bound, const uint8_t *ref, bool run) {
  if (run) {
    ip = get_run(ip, ip_bound, ref);
  } else {
    ip = get_match(ip, ip_bound, ref);
  }

  return ip;
}

// Get a guess for the compressed size of a buffer
static float get_cratio(const uint8_t *ibase, int maxlen, int minlen, int ipshift) {
  const uint8_t *ip = ibase;
  int32_t oc = 0;
  const uint16_t hashlen = (1U << (uint8_t)HASH_LOG);
  uint32_t htab[1U << (uint8_t)HASH_LOG];
  uint32_t hval;
  uint32_t seq;
  uint8_t copy;
  // Make a tradeoff between testing too much and too little
  uint16_t limit = (maxlen > hashlen) ? hashlen : maxlen;
  const uint8_t *ip_bound = ibase + limit - 1;
  const uint8_t *ip_limit = ibase + limit - 12;

  // Initialize the hash table to distances of 0
  memset(htab, 0, hashlen * sizeof(uint32_t));

  /* we start with literal copy */
  copy = 4;
  oc += 5;

  /* main loop */
  while (ip < ip_limit) {
    const uint8_t *ref;
    unsigned distance;
    const uint8_t *anchor = ip; /* comparison starting-point */

    /* find potential match */
    seq = BLOSCLZ_READU32(ip);
    HASH_FUNCTION(hval, seq, HASH_LOG)
    ref = ibase + htab[hval];

    /* calculate distance to the match */
    distance = (unsigned int) (anchor - ref);

    /* update hash table */
    htab[hval] = (uint32_t)(anchor - ibase);

    if (distance == 0 || (distance >= MAX_FARDISTANCE)) {
      LITERAL2(ip, anchor, copy)
      continue;
    }

    /* is this a match? check the first 4 bytes */
    if (BLOSCLZ_READU32(ref) == BLOSCLZ_READU32(ip)) {
      ref += 4;
    } else {
      /* no luck, copy as a literal */
      LITERAL2(ip, anchor, copy)
      continue;
    }

    /* last matched byte */
    ip = anchor + 4;

    /* distance is biased */
    distance--;

    /* get runs or matches; zero distance means a run */
    ip = get_run_or_match((uint8_t *) ip, (uint8_t *) ip_bound, ref, !distance);

    ip -= ipshift;
    int32_t len = (int32_t)(ip - anchor);
    if (len < minlen) {
      LITERAL2(ip, anchor, copy)
      continue;
    }

    /* if we haven't copied anything, adjust the output counter */
    if (!copy)
      oc--;
    /* reset literal counter */
    copy = 0;

    /* encode the match */
    if (distance < MAX_DISTANCE) {
      if (len >= 7) {
        oc += ((len - 7) / 255) + 1;
      }
      oc += 2;
    } else {
      /* far away, but not yet in the another galaxy... */
      if (len >= 7) {
        oc += ((len - 7) / 255) + 1;
      }
      oc += 4;
    }

    /* update the hash at match boundary */
    seq = BLOSCLZ_READU32(ip);
    HASH_FUNCTION(hval, seq, HASH_LOG)
    htab[hval] = (uint32_t)(ip++ - ibase);
    ip++;
    /* assuming literal copy */
    oc++;
  }

  float ic = (float) (ip - ibase);
  return ic / (float) oc;
}

static int encoder(const uint8_t *input, int32_t input_len,
                   uint8_t *output, int32_t output_len,
                   uint8_t meta,
                   blosc2_cparams *cparams, const void *chunk) {
  // Get the cratio.  minlen and ipshift are decent defaults, but one can try
  // with (4, 4) or (3, 4) or (4, 3)
  float cratio = get_cratio(input, input_len, 3, 3);
  int cbytes = (int) ((float) input_len / cratio);
  if (cbytes > input_len) {
    cbytes = input_len;
  }
  return cbytes;
}

void register_entropy_codec(blosc2_codec *codec) {
  codec->compcode = ENTROPY_PROBE_ID;
  codec->version = 1;
  codec->complib = 1;
  codec->compname = "entropy_probe";
  codec->encoder = encoder;
  codec->decoder = NULL;
  blosc2_register_codec(codec);
}


// Get entropy speed for a zeros chunk
float get_zeros_speed(int32_t chunksize) {
  blosc_timestamp_t t0, t1;
  // Build artificial zeros chunk
  uint8_t *zeros_chunk = (uint8_t *) malloc(chunksize);
  memset(zeros_chunk, 0, chunksize);
  // Compress
  uint8_t *cdata = (uint8_t *) malloc(chunksize + BLOSC2_MAX_OVERHEAD);
  blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
  cparams.nthreads = 4;
  blosc2_context *cctx = blosc2_create_cctx(cparams);
  blosc_set_timestamp(&t0);
  int csize = blosc2_compress_ctx(cctx, zeros_chunk, chunksize, cdata, chunksize + BLOSC2_MAX_OVERHEAD);
  blosc_set_timestamp(&t1);
  blosc2_free_ctx(cctx);

  free(zeros_chunk);
  free(cdata);

  if (csize < 0) {
    fprintf(stderr, "Error %d compressing zeros chunk\n", csize);
    return csize;
  }

  return (float) chunksize / (float) blosc_elapsed_secs(t0, t1);
}
