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

#define VARIANCE_SSE_SUM_4 \
    "punpcklbh  %[ftmp3],   %[ftmp1],       %[ftmp0]            \n\t" \
    "punpcklbh  %[ftmp5],   %[ftmp2],       %[ftmp0]            \n\t" \
                                                                      \
    /* sum */                                                         \
    "punpcklhw  %[ftmp1],   %[ftmp3],       %[ftmp0]            \n\t" \
    "punpckhhw  %[ftmp2],   %[ftmp3],       %[ftmp0]            \n\t" \
    "punpcklhw  %[ftmp7],   %[ftmp5],       %[ftmp0]            \n\t" \
    "punpckhhw  %[ftmp8],   %[ftmp5],       %[ftmp0]            \n\t" \
    "psubw      %[ftmp3],   %[ftmp1],       %[ftmp7]            \n\t" \
    "psubw      %[ftmp5],   %[ftmp2],       %[ftmp8]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp3]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp5]            \n\t" \
                                                                      \
    /* *sse */                                                        \
    "pmuluw     %[ftmp1],   %[ftmp3],       %[ftmp3]            \n\t" \
    "pmuluw     %[ftmp2],   %[ftmp5],       %[ftmp5]            \n\t" \
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp1]            \n\t" \
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp2]            \n\t" \
    "dsrl       %[ftmp3],   %[ftmp3],       %[ftmp11]           \n\t" \
    "dsrl       %[ftmp5],   %[ftmp5],       %[ftmp11]           \n\t" \
    "pmuluw     %[ftmp1],   %[ftmp3],       %[ftmp3]            \n\t" \
    "pmuluw     %[ftmp2],   %[ftmp5],       %[ftmp5]            \n\t" \
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp1]            \n\t" \
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp2]            \n\t" \

#define VARIANCE_SSE_SUM_8 \
    "punpcklbh  %[ftmp3],   %[ftmp1],       %[ftmp0]            \n\t" \
    "punpckhbh  %[ftmp4],   %[ftmp1],       %[ftmp0]            \n\t" \
    "punpcklbh  %[ftmp5],   %[ftmp2],       %[ftmp0]            \n\t" \
    "punpckhbh  %[ftmp6],   %[ftmp2],       %[ftmp0]            \n\t" \
                                                                      \
    /* sum */                                                         \
    "punpcklhw  %[ftmp1],   %[ftmp3],       %[ftmp0]            \n\t" \
    "punpckhhw  %[ftmp2],   %[ftmp3],       %[ftmp0]            \n\t" \
    "punpcklhw  %[ftmp7],   %[ftmp5],       %[ftmp0]            \n\t" \
    "punpckhhw  %[ftmp8],   %[ftmp5],       %[ftmp0]            \n\t" \
    "psubw      %[ftmp3],   %[ftmp1],       %[ftmp7]            \n\t" \
    "psubw      %[ftmp5],   %[ftmp2],       %[ftmp8]            \n\t" \
    "punpcklhw  %[ftmp1],   %[ftmp4],       %[ftmp0]            \n\t" \
    "punpckhhw  %[ftmp2],   %[ftmp4],       %[ftmp0]            \n\t" \
    "punpcklhw  %[ftmp7],   %[ftmp6],       %[ftmp0]            \n\t" \
    "punpckhhw  %[ftmp8],   %[ftmp6],       %[ftmp0]            \n\t" \
    "psubw      %[ftmp4],   %[ftmp1],       %[ftmp7]            \n\t" \
    "psubw      %[ftmp6],   %[ftmp2],       %[ftmp8]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp3]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp4]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp5]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp6]            \n\t" \
                                                                      \
    /* *sse */                                                        \
    "pmuluw     %[ftmp1],   %[ftmp3],       %[ftmp3]            \n\t" \
    "pmuluw     %[ftmp2],   %[ftmp5],       %[ftmp5]            \n\t" \
    "pmuluw     %[ftmp7],   %[ftmp4],       %[ftmp4]            \n\t" \
    "pmuluw     %[ftmp8],   %[ftmp6],       %[ftmp6]            \n\t" \
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp1]            \n\t" \
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp2]            \n\t" \
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp7]            \n\t" \
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp8]            \n\t" \
    "dsrl       %[ftmp3],   %[ftmp3],       %[ftmp11]           \n\t" \
    "dsrl       %[ftmp5],   %[ftmp5],       %[ftmp11]           \n\t" \
    "dsrl       %[ftmp4],   %[ftmp4],       %[ftmp11]           \n\t" \
    "dsrl       %[ftmp6],   %[ftmp6],       %[ftmp11]           \n\t" \
    "pmuluw     %[ftmp1],   %[ftmp3],       %[ftmp3]            \n\t" \
    "pmuluw     %[ftmp2],   %[ftmp5],       %[ftmp5]            \n\t" \
    "pmuluw     %[ftmp7],   %[ftmp4],       %[ftmp4]            \n\t" \
    "pmuluw     %[ftmp8],   %[ftmp6],       %[ftmp6]            \n\t" \
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp1]            \n\t" \
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp2]            \n\t" \
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp7]            \n\t" \
    "paddw      %[ftmp10],  %[ftmp10],      %[ftmp8]            \n\t"


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
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "li         %[tmp0],    0x40                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x0f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x08(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x17(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x10(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x17(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x10(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x1f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x18(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x1f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x18(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x27(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x20(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x27(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x20(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x2f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x28(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x2f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x28(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x37(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x30(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x37(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x30(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x3f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x38(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x3f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x38(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 4096);
}

//VAR(64, 32)
uint32_t vpx_variance64x32_mmi(const uint8_t *a, int a_stride,
                               const uint8_t *b, int b_stride,
                               uint32_t *sse) {
  int sum;
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x0f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x08(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x17(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x10(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x17(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x10(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x1f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x18(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x1f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x18(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x27(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x20(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x27(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x20(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x2f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x28(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x2f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x28(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x37(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x30(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x37(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x30(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x3f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x38(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x3f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x38(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 2048);
}

//VAR(32, 64)
uint32_t vpx_variance32x64_mmi(const uint8_t *a, int a_stride,
                               const uint8_t *b, int b_stride,
                               uint32_t *sse) {
  int sum;
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "li         %[tmp0],    0x40                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x0f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x08(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x17(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x10(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x17(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x10(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x1f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x18(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x1f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x18(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 2048);
}

//VAR(32, 32)
uint32_t vpx_variance32x32_mmi(const uint8_t *a, int a_stride,
                               const uint8_t *b, int b_stride,
                               uint32_t *sse) {
  int sum;
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x0f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x08(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x17(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x10(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x17(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x10(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x1f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x18(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x1f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x18(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 1024);
}

//VAR(32, 16)
uint32_t vpx_variance32x16_mmi(const uint8_t *a, int a_stride,
                               const uint8_t *b, int b_stride,
                               uint32_t *sse) {
  int sum;
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "li         %[tmp0],    0x10                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x0f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x08(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x17(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x10(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x17(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x10(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x1f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x18(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x1f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x18(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 512);
}

//VAR(16, 32)
uint32_t vpx_variance16x32_mmi(const uint8_t *a, int a_stride,
                               const uint8_t *b, int b_stride,
                               uint32_t *sse) {
  int sum;
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x0f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x08(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 512);
}

//VAR(16, 16)
uint32_t vpx_variance16x16_mmi(const uint8_t *a, int a_stride,
                               const uint8_t *b, int b_stride,
                               uint32_t *sse) {
  int sum;
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "li         %[tmp0],    0x10                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x0f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x08(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 256);
}

//VAR(16, 8)
uint32_t vpx_variance16x8_mmi(const uint8_t *a, int a_stride,
                              const uint8_t *b, int b_stride,
                              uint32_t *sse) {
  int sum;
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "li         %[tmp0],    0x08                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "gsldlc1    %[ftmp1],   0x0f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x08(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 128);
}

//VAR(8, 16)
uint32_t vpx_variance8x16_mmi(const uint8_t *a, int a_stride,
                              const uint8_t *b, int b_stride,
                              uint32_t *sse) {
  int sum;
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "li         %[tmp0],    0x10                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 128);
}

//VAR(8, 8)
uint32_t vpx_variance8x8_mmi(const uint8_t *a, int a_stride,
                             const uint8_t *b, int b_stride,
                             uint32_t *sse) {
  int sum;
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "li         %[tmp0],    0x08                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 64);
}

//VAR(8, 4)
uint32_t vpx_variance8x4_mmi(const uint8_t *a, int a_stride,
                             const uint8_t *b, int b_stride,
                             uint32_t *sse) {
  int sum;
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "li         %[tmp0],    0x04                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 32);
}

//VAR(4, 8)
uint32_t vpx_variance4x8_mmi(const uint8_t *a, int a_stride,
                             const uint8_t *b, int b_stride,
                             uint32_t *sse) {
  int sum;
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "li         %[tmp0],    0x08                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_4

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 32);
}

//VAR(4, 4)
uint32_t vpx_variance4x4_mmi(const uint8_t *a, int a_stride,
                             const uint8_t *b, int b_stride,
                             uint32_t *sse) {
  int sum;
  double ftmp[12];
  uint32_t tmp[3];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
    "li         %[tmp0],    0x04                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "xor        %[ftmp10],  %[ftmp10],      %[ftmp10]           \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_SUM_4

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[tmp1],    %[ftmp9]                            \n\t"
    "mfhc1      %[tmp2],    %[ftmp9]                            \n\t"
    "addu       %[sum],     %[tmp1],        %[tmp2]             \n\t"
    "swc1       %[ftmp10],  0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [a]"+&r"(a),                      [b]"+&r"(b),
      [sum]"=&r"(sum)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse - (((int64_t)sum * sum) / 16);
}
