#ifndef BTUNE_MODEL_H
#define BTUNE_MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

void btune_model_init(blosc2_context * ctx);

int btune_model_inference(
  blosc2_context * ctx,
  int * compcode, uint8_t * filter, int * clevel, int32_t * splitmode);

void btune_model_free(blosc2_context * ctx);

#ifdef __cplusplus
}
#endif

#endif  /* BTUNE_MODEL_H */
