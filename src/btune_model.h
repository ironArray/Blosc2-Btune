#ifndef BTUNE_MODEL_H
#define BTUNE_MODEL_H

#include "btune.h"

#ifdef __cplusplus
extern "C" {
#endif

metadata_t * btune_model_read_metadata();
int btune_model_inference(blosc2_context * ctx, metadata_t *metadata, btune_comp_mode btune_comp,
                          int * compcode, uint8_t * filter, int * clevel);

#ifdef __cplusplus
}
#endif

#endif  /* BTUNE_MODEL_H */
