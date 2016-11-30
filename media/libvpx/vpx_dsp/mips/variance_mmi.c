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
#include "vpx_ports/asmdefs_mmi.h"


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
#if 1
  int sum = 0;
  double ftmp[12];
  uint64_t tmp[4];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp3],    0x20                                \n\t"
    "li         %[tmp0],    0x10                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "mtc1       %[tmp3],    %[ftmp9]                            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "xor        %[ftmp11],  %[ftmp11],      %[ftmp11]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    "punpcklbh  %[ftmp3],   %[ftmp1],       %[ftmp0]            \n\t"
    "punpckhbh  %[ftmp4],   %[ftmp1],       %[ftmp0]            \n\t"
    "punpcklbh  %[ftmp5],   %[ftmp2],       %[ftmp0]            \n\t"
    "punpckhbh  %[ftmp6],   %[ftmp2],       %[ftmp0]            \n\t"
    "psubsh     %[ftmp3],   %[ftmp3],       %[ftmp5]            \n\t"
    "psubsh     %[ftmp4],   %[ftmp4],       %[ftmp6]            \n\t"
    "paddsh     %[ftmp7],   %[ftmp3],       %[ftmp4]            \n\t"
    "pmullh     %[ftmp3],   %[ftmp3],       %[ftmp3]            \n\t"
    "pmullh     %[ftmp4],   %[ftmp4],       %[ftmp4]            \n\t"
    "paddush    %[ftmp8],   %[ftmp3],       %[ftmp4]            \n\t"

    "gsldlc1    %[ftmp1],   0x0f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x08(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[b])                          \n\t"
    "punpcklbh  %[ftmp3],   %[ftmp1],       %[ftmp0]            \n\t"
    "punpckhbh  %[ftmp4],   %[ftmp1],       %[ftmp0]            \n\t"
    "punpcklbh  %[ftmp5],   %[ftmp2],       %[ftmp0]            \n\t"
    "punpckhbh  %[ftmp6],   %[ftmp2],       %[ftmp0]            \n\t"
    "psubsh     %[ftmp3],   %[ftmp3],       %[ftmp5]            \n\t"
    "psubsh     %[ftmp4],   %[ftmp4],       %[ftmp6]            \n\t"
    "paddsh     %[ftmp5],   %[ftmp3],       %[ftmp4]            \n\t"
    "pmullh     %[ftmp3],   %[ftmp3],       %[ftmp3]            \n\t"
    "pmullh     %[ftmp4],   %[ftmp4],       %[ftmp4]            \n\t"
    "paddush    %[ftmp6],   %[ftmp3],       %[ftmp4]            \n\t"

    "paddsh     %[ftmp2],   %[ftmp7],       %[ftmp5]            \n\t"
    "paddush    %[ftmp3],   %[ftmp8],       %[ftmp6]            \n\t"
    "dsrl       %[ftmp4],   %[ftmp2],       %[ftmp9]            \n\t"
    "dsrl       %[ftmp5],   %[ftmp3],       %[ftmp9]            \n\t"
    "paddsh     %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t"
    "paddush    %[ftmp3],   %[ftmp3],       %[ftmp5]            \n\t"
    "punpcklhw  %[ftmp2],   %[ftmp2],       %[ftmp0]            \n\t"
    "punpcklhw  %[ftmp3],   %[ftmp3],       %[ftmp0]            \n\t"
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp2]            \n\t"
    "paddw      %[ftmp11],  %[ftmp11],      %[ftmp3]            \n\t"
    "addi       %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp10]                           \n\t"
    "mfhc1      %[tmp2],    %[ftmp10]                           \n\t"
    "add        %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "mfc1       %[tmp1],    %[ftmp11]                           \n\t"
    "mfhc1      %[tmp2],    %[ftmp11]                           \n\t"
    "add        %[sse],     %[tmp1],        %[tmp2]             \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),              [tmp3]"=&r"(tmp[3]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum),                  [sse]"=&r"(*sse)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride)
    : "memory"
  );

  return *sse;
#else
  int i, sum;

  sum = 0;
  *sse = 0;

  for (i = 0; i < 16; ++i) {
    sum += a[0] - b[0];
    sum += a[1] - b[1];
    sum += a[2] - b[2];
    sum += a[3] - b[3];

    sum += a[4] - b[4];
    sum += a[5] - b[5];
    sum += a[6] - b[6];
    sum += a[7] - b[7];

    sum += a[8] - b[8];
    sum += a[9] - b[9];
    sum += a[10] - b[10];
    sum += a[11] - b[11];

    sum += a[12] - b[12];
    sum += a[13] - b[13];
    sum += a[14] - b[14];
    sum += a[15] - b[15];

    *sse += (a[0] - b[0]) * (a[0] - b[0]);
    *sse += (a[1] - b[1]) * (a[1] - b[1]);
    *sse += (a[2] - b[2]) * (a[2] - b[2]);
    *sse += (a[3] - b[3]) * (a[3] - b[3]);

    *sse += (a[4] - b[4]) * (a[4] - b[4]);
    *sse += (a[5] - b[5]) * (a[5] - b[5]);
    *sse += (a[6] - b[6]) * (a[6] - b[6]);
    *sse += (a[7] - b[7]) * (a[7] - b[7]);

    *sse += (a[8] - b[8]) * (a[8] - b[8]);
    *sse += (a[9] - b[9]) * (a[9] - b[9]);
    *sse += (a[10] - b[10]) * (a[10] - b[10]);
    *sse += (a[11] - b[11]) * (a[11] - b[11]);

    *sse += (a[12] - b[12]) * (a[12] - b[12]);
    *sse += (a[13] - b[13]) * (a[13] - b[13]);
    *sse += (a[14] - b[14]) * (a[14] - b[14]);
    *sse += (a[15] - b[15]) * (a[15] - b[15]);

    a += a_stride;
    b += b_stride;
  }
  return *sse;
#endif
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
