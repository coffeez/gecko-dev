/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vpx_ports/mem.h"
#include "vpx/vpx_integer.h"

static const uint8_t bilinear_filters[8][2] = {
  { 128,   0  },
  { 112,  16  },
  {  96,  32  },
  {  80,  48  },
  {  64,  64  },
  {  48,  80  },
  {  32,  96  },
  {  16, 112  },
};

static void variance(const uint8_t *a, int  a_stride,
                     const uint8_t *b, int  b_stride,
                     int  w, int  h, uint32_t *sse, int *sum) {
  int i, j;

  *sum = 0;
  *sse = 0;

  for (i = 0; i < h; ++i) {
    for (j = 0; j < w; ++j) {
      const int diff = a[j] - b[j];
      *sum += diff;
      *sse += diff * diff;
    }

    a += a_stride;
    b += b_stride;
  }
}

void vpx_comp_avg_pred_mmi(uint8_t *comp_pred, const uint8_t *pred,
                           int width, int height,
                           const uint8_t *ref, int ref_stride) {
  int i, j;

  for (i = 0; i < height; ++i) {
    for (j = 0; j < width; ++j) {
      const int tmp = pred[j] + ref[j];
      comp_pred[j] = ROUND_POWER_OF_TWO(tmp, 1);
    }
    comp_pred += width;
    pred += width;
    ref += ref_stride;
  }
}

//VAR(64, 64)
uint32_t vpx_variance64x64_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 64, 64, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (64 * 64));
}

//VAR(64, 32)
uint32_t vpx_variance64x32_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 64, 32, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (64 * 32));
}

//VAR(32, 64)
uint32_t vpx_variance32x64_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 32, 64, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (32 * 64));
}

//VAR(32, 32)
uint32_t vpx_variance32x32_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 32, 32, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (32 * 32));
}

//VAR(32, 16)
uint32_t vpx_variance32x16_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 32, 16, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (32 * 16));
}

//VAR(16, 32)
uint32_t vpx_variance16x32_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 16, 32, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (16 * 32));
}

//VAR(16, 16)
uint32_t vpx_variance16x16_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 16, 16, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (16 * 16));
}

//VAR(16, 8)
uint32_t vpx_variance16x8_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 16, 8, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (16 * 8));
}

//VAR(8, 16)
uint32_t vpx_variance8x16_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 8, 16, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (8 * 16));
}

//VAR(8, 8)
uint32_t vpx_variance8x8_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 8, 8, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (8 * 8));
}

//VAR(8, 4)
uint32_t vpx_variance8x4_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 8, 4, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (8 * 4));
}

//VAR(4, 8)
uint32_t vpx_variance4x8_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 4, 8, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (4 * 8));
}

//VAR(4, 4)
uint32_t vpx_variance4x4_mmi(const uint8_t *a, int a_stride,
                                   const uint8_t *b, int b_stride,
                                   uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 4, 4, sse, &sum);
  return *sse - (((int64_t)sum * sum) / (4 * 4));
}
