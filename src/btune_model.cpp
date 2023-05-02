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

#define CHECK(x) \
    if (!(x)) { \
        fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); \
        return -1; \
    }

typedef struct {
    float mean;
    float std;
} norm_t;

typedef struct {
    uint8_t codec;
    uint8_t filter;
    int clevel;
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
    float cspeed
)
{
    // Fill input tensor
    float* input = interpreter->typed_input_tensor<float>(0);
    *input = cratio;
    *(input+1) = cspeed;

    // Run inference
    CHECK(interpreter->Invoke() == kTfLiteOk);
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
//      printf("%f ", value);
        output++;
        if (value > max) {
            max = value;
            best = i;
        }
    }
//  printf("-> %d\n\n", best);

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

    blosc_timestamp_t last, current;
    blosc_set_timestamp(&last);
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
    int8_t cdata[size];
    int csize = blosc2_compress_ctx(cctx, src, size, cdata, sizeof(cdata));
    if (csize < 0) {
        printf("Error %d compressing chunk\n", csize);
        return csize;
    }
    // Decompress so we can read the instrumentation data
    int8_t ddata[size];
    int dsize = blosc2_decompress_ctx(dctx, cdata, csize, ddata, size);
    BLOSC_ERROR(dsize);
    // >>> ENTROPY PROBER END
    blosc_set_timestamp(&current);
    printf("TIME ENTROPY: %f\n", (float) blosc_elapsed_secs(last, current));

    blosc_set_timestamp(&last);
    // <<< INFERENCE START
    float cratio_mean = metadata->cratio.mean;
    float cratio_std = metadata->cratio.std;
    float cspeed_mean = metadata->cspeed.mean;
    float cspeed_std = metadata->cspeed.std;

    // Read the cratio/cspeed for every block
    int codecs[NCODECS] = {0};
    int nblocks = dsize / (int)sizeof(blosc2_instr);
    blosc2_instr *instr_data = (blosc2_instr *)ddata;
    for (int i = 0; i < nblocks; i++) {
        // Normalize
        float cratio = normalize(instr_data->cratio, cratio_mean, cratio_std);
        float cspeed = normalize(instr_data->cspeed, cspeed_mean, cspeed_std);
        //printf("block=%d cratio=%f cspeed=%f\n", i, cratio, cspeed);
        instr_data++;

        // Run inference
        int best = get_best_codec(interpreter, cratio, cspeed);
        codecs[best]++;
    }

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
    blosc_set_timestamp(&current);
    printf("TIME INFEREN: %f\n", (float) blosc_elapsed_secs(last, current));

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
    //printf("read_metadata(%s)\n", fname);
    FILE* file = fopen(fname, "rt");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open the %s file\n", fname);
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
                metadata->categories[i].codec = codec->u.integer;
                metadata->categories[i].filter = filter->u.integer;
                metadata->categories[i].clevel = clevel->u.integer;
            }
        }
    }

    free(buffer);
    fclose(file);
    return 0;
}

int btune_model_inference(blosc2_context * ctx, btune_config * config,
                          int * compcode, uint8_t * filter, int * clevel)
{
    blosc_timestamp_t last, current;
    blosc_set_timestamp(&last);

    // Read environement variables
    const char * name;
    name = config->perf_mode == BTUNE_PERF_DECOMP ? "BTUNE_METADATA_DECOMP" : "BTUNE_METADATA_COMP";
    const char * metadata_fname = getenv(name);
    if (metadata_fname == NULL) {
        BTUNE_DEBUG("Environment variable %s is not defined", name);
        return -1;
    }

    name = config->perf_mode == BTUNE_PERF_DECOMP ? "BTUNE_MODEL_DECOMP" : "BTUNE_MODEL_COMP";
    const char * model_fname = getenv(name);
    if (model_fname == NULL) {
        BTUNE_DEBUG("Environment variable %s is not defined", name);
        return -1;
    }

    // Read metadata
    metadata_t metadata;
    int error = read_metadata(metadata_fname, &metadata);
    if (error) {
        return -1;
    }

    // Load model
    std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile(model_fname);
    CHECK(model != nullptr);

    // Build the interpreter with the InterpreterBuilder.
    // Note: all Interpreters should be built with the InterpreterBuilder,
    // which allocates memory for the Interpreter and does various set up
    // tasks so that the Interpreter can read the provided model.
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model, resolver);
    std::unique_ptr<tflite::Interpreter> interpreter;
    builder(&interpreter);
    CHECK(interpreter != nullptr);

    // Allocate tensor buffers.
    CHECK(interpreter->AllocateTensors() == kTfLiteOk);
    //printf("=== Pre-invoke Interpreter State ===\n");
    //tflite::PrintInterpreterState(interpreter.get());

    blosc_set_timestamp(&current);
    printf("TIME LOAD MO: %f\n", (float) blosc_elapsed_secs(last, current));

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
    free(metadata.categories);

    return 0;
}
