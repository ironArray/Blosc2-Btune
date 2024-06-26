/*********************************************************************
  Btune for Blosc2 - Automatically choose the best codec/filter for your data

  Copyright (c) 2023-present  Blosc Development Team <blosc@blosc.org>
  https://btune.blosc.org
  Copyright (c) 2023-present  ironArray SLU <contact@ironarray.io>
  https://ironarray.io
  License: GNU Affero General Public License v3.0
  See LICENSE.txt for details about copyright and rights to use.
**********************************************************************/

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

void g_models_free(void);

void set_reuse_models(bool new_value);

#ifdef __cplusplus
}
#endif

#endif  /* BTUNE_MODEL_H */
