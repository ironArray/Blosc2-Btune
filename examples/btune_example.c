/*********************************************************************
  Btune for Blosc2 - Automatically choose the best codec/filter for your data

  Copyright (c) 2023-present  Blosc Development Team <blosc@blosc.org>
  https://btune.blosc.org
  Copyright (c) 2023-present  ironArray SLU <contact@ironarray.io>
  https://ironarray.io
  License: GNU Affero General Public License v3.0
  See LICENSE.txt for details about copyright and rights to use.
**********************************************************************/

#include <btune.h>
#include <blosc2/tuners-registry.h>
#include "blosc2.h"


#define KB  1024.
#define MB  (1024*KB)

#define CHUNKSIZE (64 * 1024)
#define BLOCKSIZE ( 8 * 1024)


static int compress(const char* in_fname, const char* out_fname) {

    // Open input file
    blosc2_schunk *schunk_in = blosc2_schunk_open(in_fname);
    if (schunk_in == NULL) {
        fprintf(stderr, "Input file cannot be open.\n");
        return 1;
    }

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
    remove(out_fname);
    blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
    dparams.nthreads = 1;
    blosc2_storage storage = {
        .cparams=&cparams,
        .dparams=&dparams,
        .contiguous=true,
        .urlpath=(char*)out_fname
    };
    blosc2_schunk* schunk_out = blosc2_schunk_new(&storage);
    if (schunk_out == NULL) {
        fprintf(stderr, "Output file cannot be created.\n");
        return 1;
    }

    // Statistics
    blosc_timestamp_t t0;
    blosc_set_timestamp(&t0);

    // Compress
    int chunksize = schunk_in->chunksize;
    void *data = malloc(chunksize);
    int nchunks = schunk_in->nchunks;
    for (int nchunk = 0; nchunk < nchunks; nchunk++) {
        int size = blosc2_schunk_decompress_chunk(schunk_in, nchunk, data, chunksize);
        if (blosc2_schunk_append_buffer(schunk_out, data, size) < 0) {
            fprintf(stderr, "Error in appending data to destination file");
            return 1;
        }
    }

    // Statistics
    blosc_timestamp_t t1;
    blosc_set_timestamp(&t1);
    int64_t nbytes = schunk_out->nbytes;
    int64_t cbytes = schunk_out->cbytes;
    double ttotal = blosc_elapsed_secs(t0, t1);
    printf("Compression ratio: %.1f MB -> %.1f MB (%.1fx)\n",
            (float)nbytes / MB, (float)cbytes / MB, (1. * (float)nbytes) / (float)cbytes);
    printf("Compression time: %.3g s, %.1f MB/s\n",
            ttotal, (float)nbytes / (ttotal * MB));

    // Free resources
    blosc2_schunk_free(schunk_in);
    blosc2_schunk_free(schunk_out);
    return 0;
}


int main(int argc, char* argv[]) {
    blosc2_init();

    // Input parameters
    if (argc < 3 || (argc % 2 != 1)) {
        fprintf(stderr, "btune_example <input file> <output.b2frame>\n");
        return 1;
    }

    for (int i = 0; i < (argc - 1) / 2; i++) {
        const char* in_fname = argv[1 + i*2];
        const char* out_fname = argv[1 + i*2 + 1];
        compress(in_fname, out_fname);
    }

    blosc2_destroy();

    return 0;
}
