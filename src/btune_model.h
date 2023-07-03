#ifndef BTUNE_MODEL_H
#define BTUNE_MODEL_H

#include <blosc2.h>
#include "btune-private.h"

#ifdef __cplusplus
extern "C" {
#endif

void btune_model_init(blosc2_context * ctx);

int btune_model_inference(
  blosc2_context * ctx,
  int * compcode, uint8_t * filter, int * clevel, int32_t * splitmode);

void btune_model_free(blosc2_context * ctx);

int most_predicted(btune_struct *btune_params, int *compcode,
                   uint8_t *filter, int *clevel, int32_t *splitmode);

#ifdef __cplusplus
}
#endif

#endif  /* BTUNE_MODEL_H */
