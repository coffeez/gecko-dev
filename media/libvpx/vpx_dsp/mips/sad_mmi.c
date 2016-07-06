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

#include "vpx_ports/asmdefs_mmi.h"
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
#if 1
  unsigned int sad = 0;
  double ftmp[7];
  uint64_t tmp[2];

  __asm__ volatile (
    "li         %[tmp1],    0x20                                \n\t"
    "li         %[tmp0],    0x08                                \n\t"
    "mtc1       %[tmp1],    %[ftmp5]                            \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp6],   %[ftmp6],       %[ftmp6]            \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[src])                        \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[src])                        \n\t"
    "gsldlc1    %[ftmp3],   0x07(%[ref])                        \n\t"
    "gsldrc1    %[ftmp3],   0x00(%[ref])                        \n\t"
    "gsldlc1    %[ftmp4],   0x0f(%[ref])                        \n\t"
    "gsldrc1    %[ftmp4],   0x08(%[ref])                        \n\t"
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t"
    "pasubub    %[ftmp3],   %[ftmp3],       %[ftmp4]            \n\t"
    "paddusb    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t"
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t"
    "paddw      %[ftmp6],   %[ftmp6],       %[ftmp1]            \n\t"
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t"
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"

    "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[src])                        \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[src])                        \n\t"
    "gsldlc1    %[ftmp3],   0x07(%[ref])                        \n\t"
    "gsldrc1    %[ftmp3],   0x00(%[ref])                        \n\t"
    "gsldlc1    %[ftmp4],   0x0f(%[ref])                        \n\t"
    "gsldrc1    %[ftmp4],   0x08(%[ref])                        \n\t"
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t"
    "pasubub    %[ftmp3],   %[ftmp3],       %[ftmp4]            \n\t"
    "paddusb    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t"
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t"
    "paddw      %[ftmp6],   %[ftmp6],       %[ftmp1]            \n\t"

    "addi       %[tmp0],    %[tmp0],        -0x02               \n\t"
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t"
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp6]                            \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
    : "memory"
  );
  return sad;
#else
  int y;
  unsigned int sad = 0;

  for (y = 0; y < 16; y++) {
    sad += abs(src[0] - ref[0]);
    sad += abs(src[1] - ref[1]);
    sad += abs(src[2] - ref[2]);
    sad += abs(src[3] - ref[3]);

    sad += abs(src[4] - ref[4]);
    sad += abs(src[5] - ref[5]);
    sad += abs(src[6] - ref[6]);
    sad += abs(src[7] - ref[7]);

    sad += abs(src[8] - ref[8]);
    sad += abs(src[9] - ref[9]);
    sad += abs(src[10] - ref[10]);
    sad += abs(src[11] - ref[11]);

    sad += abs(src[12] - ref[12]);
    sad += abs(src[13] - ref[13]);
    sad += abs(src[14] - ref[14]);
    sad += abs(src[15] - ref[15]);

    src += src_stride;
    ref += ref_stride;
  }
  return sad;
#endif
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
