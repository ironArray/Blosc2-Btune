#include <tensorflow/lite/core/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/optional_debug_tools.h>

#include <blosc2.h>
#include "context.h"
#include "entropy_probe.h"
#include "btune.h"
#include "btune_model.h"
#include "json.h"


#define NCODECS 15

typedef struct {
    float mean;
    float std;
} norm_t;

typedef struct {
    uint8_t codec;
    uint8_t filter;
    int clevel;
    int32_t splitmode;
} category_t;

typedef struct {
    norm_t cratio;
    norm_t cspeed;
    category_t *categories;
} metadata_t;


static int fsize(FILE *file) {
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

static int get_best_codec(
    tflite::Interpreter *interpreter,
    float cratio,
    float cspeed,
    float comp_balance
)
{
    // Fill input tensor
    float* input = interpreter->typed_input_tensor<float>(0);
    *input = cratio;
    *(input+1) = cspeed;
    *(input+2) = comp_balance;

    // Run inference
    if (interpreter->Invoke() != kTfLiteOk) {
        fprintf(stderr, "Error: interpreter invocation failed\n");
        return -1;
    }

    //printf("\n\n=== Post-invoke Interpreter State ===\n");
    //tflite::PrintInterpreterState(interpreter);

    // Read output buffers
    // Note: The buffer of the output tensor with index `i` of type T can
    // be accessed with `T* output = interpreter->typed_output_tensor<T>(i);`
    float* output = interpreter->typed_output_tensor<float>(0);

    int best = 0;
    float max = -1;
    for (int i = 0; i < NCODECS; i++) {
        float value = *output;
        output++;
        if (value > max) {
            max = value;
            best = i;
        }
    }

    return best;
}

static float normalize(float value, float mean, float std)
{
    value -= mean;
    value /= std;
    return value;
}

static int get_best_codec_for_chunk(
    blosc2_schunk *schunk,
    const void *src,
    size_t size,
    tflite::Interpreter *interpreter,
    metadata_t *metadata
)
{
    char * trace = getenv("BTUNE_TRACE");
    blosc_timestamp_t last, current;
    if (trace) {
        blosc_set_timestamp(&last);
    }

    // <<< ENTROPY PROBER START
    // cparams
    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    cparams.compcode = ENTROPY_PROBE_ID;
    cparams.instr_codec = true;  // instrumented (cratio/cspeed)
    cparams.blocksize = schunk->blocksize;
    cparams.splitmode = BLOSC_NEVER_SPLIT;
    cparams.filters[BLOSC2_MAX_FILTERS - 1] = BLOSC_NOFILTER;
    cparams.nthreads = 8;
    blosc2_context *cctx = blosc2_create_cctx(cparams);

    // dparams
    blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
    blosc2_context *dctx = blosc2_create_dctx(dparams);

    // Compress chunk, this will output the instrumentation data
    int8_t *cdata = (int8_t *) malloc(size * sizeof(int8_t));
    int csize = blosc2_compress_ctx(cctx, src, size, cdata, sizeof(cdata));
    if (csize < 0) {
        free(cdata);
        fprintf(stderr, "Error %d compressing chunk\n", csize);
        return csize;
    }
    // Decompress so we can read the instrumentation data
    int8_t *ddata = (int8_t *) malloc(size * sizeof(int8_t));
    int dsize = blosc2_decompress_ctx(dctx, cdata, csize, ddata, size);
    free(cdata);
    BLOSC_ERROR(dsize);
    // >>> ENTROPY PROBER END
    if (trace) {
        blosc_set_timestamp(&current);
        printf("TIME ENTROPY: %f\n", (float) blosc_elapsed_secs(last, current));
        blosc_set_timestamp(&last);
    }

    // <<< INFERENCE START
    float cratio_mean = metadata->cratio.mean;
    float cratio_std = metadata->cratio.std;
    float cspeed_mean = metadata->cspeed.mean;
    float cspeed_std = metadata->cspeed.std;
    btune_struct * btune = (btune_struct *)schunk->storage->cparams->tuner_params;

    // Read the cratio/cspeed for every block
    int codecs[NCODECS] = {0};
    int nblocks = dsize / (int)sizeof(blosc2_instr);
    blosc2_instr *instr_data = (blosc2_instr *)ddata;
    for (int i = 0; i < nblocks; i++) {
        // Normalize
        float cratio = normalize(instr_data->cratio, cratio_mean, cratio_std);
        float cspeed = normalize(instr_data->cspeed, cspeed_mean, cspeed_std);
        instr_data++;

        // Run inference
      int best = get_best_codec(interpreter, cratio, cspeed, btune->config.comp_balance);
        codecs[best]++;
    }
    free(ddata);

    // The best codec for the chunk is the codec that wins for most blocks
    int best = -1;
    int max = 0;
    for (int i = 0; i < NCODECS; i++) {
        int value = codecs[i];
        if (value > max) {
            max = value;
            best = i;
        }
    }
    // >>> INFERENCE START
    if (trace) {
        blosc_set_timestamp(&current);
        printf("TIME INFEREN: %f\n", (float) blosc_elapsed_secs(last, current));
    }

    return best;
}

static int read_dict(json_value *json, norm_t *norm)
{
    for (int i = 0; i < json->u.object.length; i++) {
        const char *name = json->u.object.values[i].name;
        json_value *value = json->u.object.values[i].value;
        if (strcmp(name, "mean") == 0) {
            norm->mean = value->u.dbl;
        }
        else if (strcmp(name, "std") == 0) {
            norm->std = value->u.dbl;
        }
    }

    return 0;
}

static int read_metadata(const char *fname, metadata_t *metadata)
{
    FILE* file = fopen(fname, "rt");
    if (file == NULL) {
        return -1;
    }

    // Parse
    int size = fsize(file);
    char *buffer = (char*)malloc(size + 1);
    size_t nread = fread(buffer, 1, size, file);
    assert(size == nread);
    buffer[size] = 0;
    json_value *json = json_parse(buffer, size);

    for (int i = 0; i < json->u.object.length; i++) {
        const char *name = json->u.object.values[i].name;
        json_value *value = json->u.object.values[i].value;
        if (strcmp(name, "cratio") == 0) {
            read_dict(value, &metadata->cratio);
        }
        else if (strcmp(name, "speed") == 0) {
            read_dict(value, &metadata->cspeed);
        }
        else if (strcmp(name, "categories") == 0) {
            metadata->categories = (category_t*)malloc(sizeof(category_t) * value->u.array.length);
            for (int i = 0; i < value->u.array.length; i++) {
                json_value *cat = value->u.array.values[i];
                json_value *codec = cat->u.array.values[0];
                json_value *filter = cat->u.array.values[1];
                json_value *clevel = cat->u.array.values[2];
                json_value *splitmode = cat->u.array.values[3];
                metadata->categories[i].codec = codec->u.integer;
                metadata->categories[i].filter = filter->u.integer;
                metadata->categories[i].clevel = clevel->u.integer;
                metadata->categories[i].splitmode = splitmode->u.integer;
            }
        }
    }

    free(buffer);
    fclose(file);
    return 0;
}

static char * concat_path(const char * dirname, const char * fname) {
    int len = strlen(dirname) + 1 + strlen(fname);
    char * dest = (char*)malloc(sizeof(char) * (len + 1));
    int n = sprintf(dest, "%s/%s", dirname, fname);
    if (n != len) {
        free(dest);
        return NULL;
    }
    return dest;
}

int btune_model_inference(
    blosc2_context * ctx, btune_config * config,  // Input args
    int * compcode, uint8_t * filter, int * clevel, int32_t * splitmode // Output args
) {
    char * trace = getenv("BTUNE_TRACE");
    blosc_timestamp_t last, current;
    if (trace) {
        blosc_set_timestamp(&last);
    }

    // Read environement variables
    const char * dirname = getenv("BTUNE_DATA_DIR");
    if (dirname == NULL) {
        BTUNE_DEBUG("Environment variable BTUNE_DATA_DIR is not defined");
        return -1;
    }

    char * metadata_fname = concat_path(
        dirname,
        config->perf_mode == BTUNE_PERF_DECOMP ? "model_decomp.json" : "model_comp.json"
    );

    char * model_fname = concat_path(
        dirname,
        config->perf_mode == BTUNE_PERF_DECOMP ? "model_decomp.tflite" : "model_comp.tflite"
    );

    // Read metadata
    metadata_t metadata;
    int error = read_metadata(metadata_fname, &metadata);
    if (error) {
        printf("WARNING: Metadata file not found in %s\n", metadata_fname);
        free(metadata_fname);
        return -1;
    }
    free(metadata_fname);

    // Load model
    std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile(model_fname);
    if (model == nullptr) {
        printf("WARNING: Model file not found in %s\n", model_fname);
        free(model_fname);
        return -1;
    }
    free(model_fname);
    printf("INFO: Model files found in the '%s/' directory\n", dirname);

    // Build the interpreter with the InterpreterBuilder.
    // Note: all Interpreters should be built with the InterpreterBuilder,
    // which allocates memory for the Interpreter and does various set up
    // tasks so that the Interpreter can read the provided model.
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model, resolver);
    std::unique_ptr<tflite::Interpreter> interpreter;
    builder(&interpreter);
    if (interpreter == nullptr) {
        fprintf(stderr, "Error: Failed to build interpreter\n");
        return -1;
    }

    // Allocate tensor buffers.
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        fprintf(stderr, "Error: Failed to allocate tensors\n");
        return -1;
    }
    //printf("=== Pre-invoke Interpreter State ===\n");
    //tflite::PrintInterpreterState(interpreter.get());

    if (trace) {
        blosc_set_timestamp(&current);
        printf("TIME LOAD MO: %f\n", (float) blosc_elapsed_secs(last, current));
    }

    const void *src = (const void*)ctx->src;
    int32_t size = ctx->srcsize;
    int best = get_best_codec_for_chunk(ctx->schunk, src, size, interpreter.get(), &metadata);
    if (best < 0) {
        return best;
    }

    // Return compcode and filter
    category_t cat = metadata.categories[best];
    *compcode = cat.codec;
    *filter = cat.filter;
    *clevel = cat.clevel;
    *splitmode = cat.splitmode;
    free(metadata.categories);

    BTUNE_DEBUG("Inference: category=%d codec=%d filter=%d clevel=%d splitmode=%d\n",
                best, *compcode, *filter, *clevel, *splitmode);
    return 0;
}
