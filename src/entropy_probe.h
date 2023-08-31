/*********************************************************************
  Btune for Blosc2 - Automatically choose the best codec/filter for your data

  Copyright (c) 2023 The Blosc Developers <blosc@blosc.org>
  https://btune.blosc.org
  License: GNU Affero General Public License v3.0
  See LICENSE.txt for details about copyright and rights to use.
**********************************************************************/

#ifndef BLOSC2_EP_H
#define BLOSC2_EP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <blosc2.h>

#define ENTROPY_PROBE_ID 244
void register_entropy_codec(blosc2_codec *codec);
#define FILTER_STOP 3
float get_zeros_speed(int32_t chunksize);
#ifdef __cplusplus
}
#endif

#endif
