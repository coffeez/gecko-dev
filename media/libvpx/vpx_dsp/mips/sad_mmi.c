/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdlib.h>

#include "vpx/vpx_integer.h"
#include "vpx_ports/mem.h"

/* Sum the difference between every corresponding element of the buffers. */
static INLINE unsigned int sad(const uint8_t *a, int a_stride,
                               const uint8_t *b, int b_stride,
                               int width, int height) {
  int y, x;
  unsigned int sad = 0;

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++)
      sad += abs(a[x] - b[x]);

    a += a_stride;
    b += b_stride;
  }
  return sad;
}

// TODO(johannkoenig): this moved to vpx_dsp, should be able to clean this up.
/* Remove dependency on vp9 variance function by duplicating vp9_comp_avg_pred.
 * The function averages every corresponding element of the buffers and stores
 * the value in a third buffer, comp_pred.
 * pred and comp_pred are assumed to have stride = width
 * In the usage below comp_pred is a local array.
 */
static INLINE void avg_pred(uint8_t *comp_pred, const uint8_t *pred, int width,
                            int height, const uint8_t *ref, int ref_stride) {
  int i, j;

  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      const int tmp = pred[j] + ref[j];
      comp_pred[j] = ROUND_POWER_OF_TWO(tmp, 1);
    }
    comp_pred += width;
    pred += width;
    ref += ref_stride;
  }
}


// 64x64
//sadMxN(64, 64)
unsigned int vpx_sad64x64_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 64, 64);
}

unsigned int vpx_sad64x64_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  uint8_t comp_pred[64 * 64];
  avg_pred(comp_pred, second_pred, 64, 64, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 64, 64, 64);
}

//sadMxNxK(64, 64, 3)
void vpx_sad64x64x3_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *ref_array, int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad64x64_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(64, 64, 8)
void vpx_sad64x64x8_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *ref_array, int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad64x64_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(64, 64)
void vpx_sad64x64x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad64x64_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 64x32
//sadMxN(64, 32)
unsigned int vpx_sad64x32_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 64, 32);
}

unsigned int vpx_sad64x32_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  uint8_t comp_pred[64 * 32];
  avg_pred(comp_pred, second_pred, 64, 32, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 64, 64, 32);
}

//sadMxNx4D(64, 32)
void vpx_sad64x32x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad64x32_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 32x64
//sadMxN(32, 64)
unsigned int vpx_sad32x64_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 32, 64);
}

unsigned int vpx_sad32x64_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  uint8_t comp_pred[32 * 64];
  avg_pred(comp_pred, second_pred, 32, 64, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 32, 32, 64);
}

//sadMxNx4D(32, 64)
void vpx_sad32x64x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad32x64_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 32x32
//sadMxN(32, 32)
unsigned int vpx_sad32x32_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 32, 32);
}

unsigned int vpx_sad32x32_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  uint8_t comp_pred[32 * 32];
  avg_pred(comp_pred, second_pred, 32, 32, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 32, 32, 32);
}

//sadMxNxK(32, 32, 3)
void vpx_sad32x32x3_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *ref_array, int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad32x32_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(32, 32, 8)
void vpx_sad32x32x8_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *ref_array, int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad32x32_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(32, 32)
void vpx_sad32x32x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad32x32_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 32x16
//sadMxN(32, 16)
unsigned int vpx_sad32x16_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 32, 16);
}

unsigned int vpx_sad32x16_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  uint8_t comp_pred[32 * 16];
  avg_pred(comp_pred, second_pred, 32, 16, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 32, 32, 16);
}

//sadMxNx4D(32, 16)
void vpx_sad32x16x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad32x16_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 16x32
//sadMxN(16, 32)
unsigned int vpx_sad16x32_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 16, 32);
}

unsigned int vpx_sad16x32_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  uint8_t comp_pred[16 * 32];
  avg_pred(comp_pred, second_pred, 16, 32, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 16, 16, 32);
}

//sadMxNx4D(16, 32)
void vpx_sad16x32x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad16x32_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 16x16
//sadMxN(16, 16)
unsigned int vpx_sad16x16_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 16, 16);
}

unsigned int vpx_sad16x16_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  uint8_t comp_pred[16 * 16];
  avg_pred(comp_pred, second_pred, 16, 16, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 16, 16, 16);
}

//sadMxNxK(16, 16, 3)
void vpx_sad16x16x3_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *ref_array, int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad16x16_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(16, 16, 8)
void vpx_sad16x16x8_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *ref_array, int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad16x16_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(16, 16)
void vpx_sad16x16x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad16x16_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 16x8
//sadMxN(16, 8)
unsigned int vpx_sad16x8_mmi(const uint8_t *src, int src_stride,
                             const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 16, 8);
}

unsigned int vpx_sad16x8_avg_mmi(const uint8_t *src, int src_stride,
                                 const uint8_t *ref, int ref_stride,
                                 const uint8_t *second_pred) {
  uint8_t comp_pred[16 * 8];
  avg_pred(comp_pred, second_pred, 16, 8, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 16, 16, 8);
}

//sadMxNxK(16, 8, 3)
void vpx_sad16x8x3_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *ref_array, int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad16x8_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(16, 8, 8)
void vpx_sad16x8x8_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *ref_array, int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad16x8_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(16, 8)
void vpx_sad16x8x4d_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *const ref_array[], int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad16x8_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 8x16
//sadMxN(8, 16)
unsigned int vpx_sad8x16_mmi(const uint8_t *src, int src_stride,
                             const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 8, 16);
}

unsigned int vpx_sad8x16_avg_mmi(const uint8_t *src, int src_stride,
                                 const uint8_t *ref, int ref_stride,
                                 const uint8_t *second_pred) {
  uint8_t comp_pred[8 * 16];
  avg_pred(comp_pred, second_pred, 8, 16, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 8, 8, 16);
}

//sadMxNxK(8, 16, 3)
void vpx_sad8x16x3_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *ref_array, int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad8x16_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(8, 16, 8)
void vpx_sad8x16x8_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *ref_array, int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad8x16_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(8, 16)
void vpx_sad8x16x4d_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *const ref_array[], int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad8x16_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 8x8
//sadMxN(8, 8)
unsigned int vpx_sad8x8_mmi(const uint8_t *src, int src_stride,
                            const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 8, 8);
}

unsigned int vpx_sad8x8_avg_mmi(const uint8_t *src, int src_stride,
                                const uint8_t *ref, int ref_stride,
                                const uint8_t *second_pred) {
  uint8_t comp_pred[8 * 8];
  avg_pred(comp_pred, second_pred, 8, 8, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 8, 8, 8);
}

//sadMxNxK(8, 8, 3)
void vpx_sad8x8x3_mmi(const uint8_t *src, int src_stride,
                      const uint8_t *ref_array, int ref_stride,
                      uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad8x8_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(8, 8, 8)
void vpx_sad8x8x8_mmi(const uint8_t *src, int src_stride,
                      const uint8_t *ref_array, int ref_stride,
                      uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad8x8_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(8, 8)
void vpx_sad8x8x4d_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *const ref_array[], int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad8x8_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 8x4
//sadMxN(8, 4)
unsigned int vpx_sad8x4_mmi(const uint8_t *src, int src_stride,
                            const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 8, 4);
}

unsigned int vpx_sad8x4_avg_mmi(const uint8_t *src, int src_stride,
                                const uint8_t *ref, int ref_stride,
                                const uint8_t *second_pred) {
  uint8_t comp_pred[8 * 4];
  avg_pred(comp_pred, second_pred, 8, 4, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 8, 8, 4);
}

//sadMxNxK(8, 4, 8)
void vpx_sad8x4x8_mmi(const uint8_t *src, int src_stride,
                      const uint8_t *ref_array, int ref_stride,
                      uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad8x4_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(8, 4)
void vpx_sad8x4x4d_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *const ref_array[], int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad8x4_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 4x8
//sadMxN(4, 8)
unsigned int vpx_sad4x8_mmi(const uint8_t *src, int src_stride,
                            const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 4, 8);
}

unsigned int vpx_sad4x8_avg_mmi(const uint8_t *src, int src_stride,
                                const uint8_t *ref, int ref_stride,
                                const uint8_t *second_pred) {
  uint8_t comp_pred[4 * 8];
  avg_pred(comp_pred, second_pred, 4, 8, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 4, 4, 8);
}

//sadMxNxK(4, 8, 8)
void vpx_sad4x8x8_mmi(const uint8_t *src, int src_stride,
                      const uint8_t *ref_array, int ref_stride,
                      uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad4x8_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(4, 8)
void vpx_sad4x8x4d_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *const ref_array[], int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad4x8_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 4x4
//sadMxN(4, 4)
unsigned int vpx_sad4x4_mmi(const uint8_t *src, int src_stride,
                            const uint8_t *ref, int ref_stride) {
  return sad(src, src_stride, ref, ref_stride, 4, 4);
}

unsigned int vpx_sad4x4_avg_mmi(const uint8_t *src, int src_stride,
                                const uint8_t *ref, int ref_stride,
                                const uint8_t *second_pred) {
  uint8_t comp_pred[4 * 4];
  avg_pred(comp_pred, second_pred, 4, 4, ref, ref_stride);
  return sad(src, src_stride, comp_pred, 4, 4, 4);
}

//sadMxNxK(4, 4, 3)
void vpx_sad4x4x3_mmi(const uint8_t *src, int src_stride,
                      const uint8_t *ref_array, int ref_stride,
                      uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad4x4_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(4, 4, 8)
void vpx_sad4x4x8_mmi(const uint8_t *src, int src_stride,
                      const uint8_t *ref_array, int ref_stride,
                      uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad4x4_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(4, 4)
void vpx_sad4x4x4d_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *const ref_array[], int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad4x4_mmi(src, src_stride, ref_array[i], ref_stride);
}
