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

uint32_t vpx_mse16x16_mmi(const uint8_t *a, int a_stride,
                          const uint8_t *b, int b_stride,
                          uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 16, 16, sse, &sum);
  return *sse;
}

uint32_t vpx_mse16x8_mmi(const uint8_t *a, int a_stride,
                         const uint8_t *b, int b_stride,
                         uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 16, 8, sse, &sum);
  return *sse;
}

uint32_t vpx_mse8x16_mmi(const uint8_t *a, int a_stride,
                         const uint8_t *b, int b_stride,
                         uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 8, 16, sse, &sum);
  return *sse;
}

uint32_t vpx_mse8x8_mmi(const uint8_t *a, int a_stride,
                        const uint8_t *b, int b_stride,
                        uint32_t *sse) {
  int sum;
  variance(a, a_stride, b, b_stride, 8, 8, sse, &sum);
  return *sse;
}
