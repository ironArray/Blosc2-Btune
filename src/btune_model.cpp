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


typedef struct {
  float mean;
  float std;
} norm_t;

typedef struct {
  uint8_t codec;
  uint8_t filter;
  int clevel;
  int32_t splitmode;
  unsigned long count; // Count the number of times the category is inferred
} category_t;

typedef struct {
  norm_t cratio;
  norm_t cspeed;
  category_t *categories;
  int ncategories;
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
  float comp_balance,
  int ncategories
) {
  // Fill input tensor
  float* input = interpreter->typed_input_tensor<float>(0);
  *input = cratio;
  *(input + 1) = cspeed;
  *(input + 2) = comp_balance;

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
  for (int i = 0; i < ncategories; i++) {
    float value = *output;
    output++;
    if (value > max) {
      max = value;
      best = i;
    }
  }

  return best;
}

static float normalize(float value, float mean, float std) {
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
) {
  char * trace = getenv("BTUNE_TRACE");
  blosc_timestamp_t t0, t1, t2;
  if (trace) {
    blosc_set_timestamp(&t0);
  }

  btune_struct *btune = (btune_struct *)schunk->storage->cparams->tuner_params;
  // <<< ENTROPY PROBER START
  // cparams
  blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
  cparams.compcode = ENTROPY_PROBE_ID;
  cparams.instr_codec = true;  // instrumented (cratio/cspeed)
  cparams.typesize = schunk->typesize;
  cparams.blocksize = schunk->blocksize;
  cparams.splitmode = BLOSC_NEVER_SPLIT;
  cparams.nthreads = 1;
  cparams.filters[BLOSC2_MAX_FILTERS - 1] = BLOSC_NOFILTER;
  blosc2_context *cctx = blosc2_create_cctx(cparams);

  // dparams
  blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
  dparams.nthreads = 1;
  blosc2_context *dctx = blosc2_create_dctx(dparams);
  if (btune->zeros_speed < 0) {
    // Compress zeros chunk to get a machine relative speed measure
    btune->zeros_speed = get_zeros_speed(cctx, dctx, size);
    if (btune->zeros_speed < 0) {
        fprintf(stderr, "Error %d computing zeros speed\n", (int)btune->zeros_speed);
        return btune->zeros_speed;
    }
  }

  // Compress chunk, this will output the instrumentation data
  uint8_t *cdata = (uint8_t *) malloc(size  + BLOSC2_MAX_OVERHEAD);
  int csize = blosc2_compress_ctx(cctx, src, size, cdata, size + BLOSC2_MAX_OVERHEAD);
  if (csize < 0) {
    free(cdata);
    fprintf(stderr, "Error %d compressing chunk\n", csize);
    return csize;
  }
  // Decompress so we can read the instrumentation data
  uint8_t *ddata = (uint8_t *) malloc(size);
  int dsize = blosc2_decompress_ctx(dctx, cdata, csize, ddata, size);
  free(cdata);
  BLOSC_ERROR(dsize);
  // >>> ENTROPY PROBER END
  if (trace) {
    blosc_set_timestamp(&t1);
  }

  // <<< INFERENCE START
  float cratio_mean = metadata->cratio.mean;
  float cratio_std = metadata->cratio.std;
  float cspeed_mean = metadata->cspeed.mean;
  float cspeed_std = metadata->cspeed.std;


  // Read the cratio/cspeed for every block and compute mean
  int nblocks = dsize / (int)sizeof(blosc2_instr);
  blosc2_instr *instr_data = (blosc2_instr *)ddata;

  float cratio = 0;
  float rel_speed = 0;
  bool special_val = instr_data->flags[0];
  for (int i = 0; i < nblocks; i++) {
    if (!special_val) {
      cratio += instr_data->cratio;
      float ctime = 1.f / instr_data->cspeed;
      float ftime = 1.f / instr_data->filter_speed;
      rel_speed += 1.f / (ctime + ftime) / btune->zeros_speed;
    }
    instr_data++;
    special_val = instr_data->flags[0];
  }
  cratio /= nblocks;
  rel_speed /= nblocks;
  // Normalize
  float cratio_norm = normalize(cratio, cratio_mean, cratio_std);
  float cspeed_norm = normalize(rel_speed, cspeed_mean, cspeed_std);
  // Run inference
  int best = get_best_codec(interpreter, cratio_norm, cspeed_norm, btune->config.comp_balance, metadata->ncategories);
  free(ddata);
  // >>> INFERENCE END
  if (trace) {
    blosc_set_timestamp(&t2);
    category_t cat = metadata->categories[best];
    BTUNE_TRACE(
      "Inference category=%d codec=%d filter=%d clevel=%d splitmode=%d time entropy=%f inference=%f",
      best, cat.codec, cat.filter, cat.clevel, cat.splitmode,
      (float) blosc_elapsed_secs(t0, t1),
      (float) blosc_elapsed_secs(t1, t2)
    );
  }

  return best;
}

static int read_dict(json_value *json, norm_t *norm) {
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

static int read_metadata(const char *fname, metadata_t *metadata) {
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
      metadata->ncategories = value->u.array.length;
      metadata->categories = (category_t*)calloc(value->u.array.length, sizeof(category_t));
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

static void * load_metadata(btune_config * config, const char * dirname) {
  char * metadata_fname = concat_path(
    dirname,
    config->perf_mode == BTUNE_PERF_DECOMP ? "model_decomp.json" : "model_comp.json"
  );

  // Read metadata
  metadata_t * metadata = (metadata_t*)malloc(sizeof(metadata_t));
  int error = read_metadata(metadata_fname, metadata);
  if (error) {
    printf("WARNING: Metadata file not found in %s\n", metadata_fname);
    free(metadata_fname);
    return NULL;
  }
  free(metadata_fname);
  return (void *)metadata;
}

static void * load_model(btune_config * config, const char * dirname) {
  char * model_fname = concat_path(
    dirname,
    config->perf_mode == BTUNE_PERF_DECOMP ? "model_decomp.tflite" : "model_comp.tflite"
  );

  // Load model
  std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile(model_fname);
  if (model == nullptr) {
    printf("WARNING: Model file not found in %s\n", model_fname);
    free(model_fname);
    return NULL;
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
    return NULL;
  }

  // Allocate tensor buffers.
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    fprintf(stderr, "Error: Failed to allocate tensors\n");
    return NULL;
  }
  //printf("=== Pre-invoke Interpreter State ===\n");
  //tflite::PrintInterpreterState(interpreter.get());

  return (void*)interpreter.release();
}

void btune_model_init(blosc2_context * ctx) {
  // Trace time
  bool trace = getenv("BTUNE_TRACE");
  blosc_timestamp_t t0, t1;
  if (trace) {
    blosc_set_timestamp(&t0);
  }

  // Read BTUNE_INFERENCE
  btune_struct *btune_params = (btune_struct*) ctx->tuner_params;
  const char * inference = getenv("BTUNE_INFERENCE");
  btune_params->inference_count = 1;
  if (inference != NULL) {
    sscanf(inference, "%d", &btune_params->inference_count);
    if (btune_params->inference_count == 0) {
      btune_params->inference_count = -1; // Means all chunks
    }
  }

  // Load model and metadata
  const char * dirname = getenv("BTUNE_MODELS_DIR");
  if (dirname == NULL) {
    BTUNE_TRACE("Environment variable BTUNE_MODELS_DIR is not defined");
    btune_params->inference_count = 0;
    return;
  }
  btune_config *config = &btune_params->config;
  btune_params->interpreter = load_model(config, dirname);
  btune_params->metadata = load_metadata(config, dirname);

  if (btune_params->interpreter == NULL || btune_params->metadata == NULL) {
    btune_params->inference_count = 0;
    return;
  }

  // Trace time
  if (trace) {
    blosc_set_timestamp(&t1);
    printf("TRACE: time load model: %f\n", (float) blosc_elapsed_secs(t0, t1));
  }
}

int btune_model_inference(
    blosc2_context * ctx,
    int * compcode, uint8_t * filter, int * clevel, int32_t * splitmode
) {

  btune_struct *btune_params = (btune_struct*) ctx->tuner_params;
  if (btune_params->interpreter == NULL || btune_params->metadata == NULL) {
    return -1;
  }

  // Get best category
  tflite::Interpreter * interpreter = (tflite::Interpreter *)btune_params->interpreter;
  metadata_t * metadata = (metadata_t*)btune_params->metadata;

  const void *src = (const void*)ctx->src;
  int32_t size = ctx->srcsize;
  int best = get_best_codec_for_chunk(ctx->schunk, src, size, interpreter, metadata);
  if (best < 0) {
    return best;
  }

  // Return
  category_t cat = metadata->categories[best];
  cat.count++;
  *compcode = cat.codec;
  *filter = cat.filter;
  *clevel = cat.clevel;
  *splitmode = cat.splitmode;

  return 0;
}

void btune_model_free(blosc2_context * ctx) {
  btune_struct *btune_params = (btune_struct *) ctx->tuner_params;

  delete btune_params->interpreter;
  btune_params->interpreter = NULL;

  metadata_t * metadata = (metadata_t *) btune_params->metadata;
  if (metadata != nullptr) {
    free(metadata->categories);
    free(metadata);
    btune_params->metadata = NULL;
  }
}
