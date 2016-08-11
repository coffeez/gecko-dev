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


#define VARIANCE_SSE_8 \
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
                                                                      \
    /* *sse */                                                        \
    "pmuluw     %[ftmp1],   %[ftmp3],       %[ftmp3]            \n\t" \
    "pmuluw     %[ftmp2],   %[ftmp5],       %[ftmp5]            \n\t" \
    "pmuluw     %[ftmp7],   %[ftmp4],       %[ftmp4]            \n\t" \
    "pmuluw     %[ftmp8],   %[ftmp6],       %[ftmp6]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp2]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp7]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp8]            \n\t" \
    "dsrl       %[ftmp3],   %[ftmp3],       %[ftmp10]           \n\t" \
    "dsrl       %[ftmp5],   %[ftmp5],       %[ftmp10]           \n\t" \
    "dsrl       %[ftmp4],   %[ftmp4],       %[ftmp10]           \n\t" \
    "dsrl       %[ftmp6],   %[ftmp6],       %[ftmp10]           \n\t" \
    "pmuluw     %[ftmp1],   %[ftmp3],       %[ftmp3]            \n\t" \
    "pmuluw     %[ftmp2],   %[ftmp5],       %[ftmp5]            \n\t" \
    "pmuluw     %[ftmp7],   %[ftmp4],       %[ftmp4]            \n\t" \
    "pmuluw     %[ftmp8],   %[ftmp6],       %[ftmp6]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp2]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp7]            \n\t" \
    "paddw      %[ftmp9],   %[ftmp9],       %[ftmp8]            \n\t"


uint32_t vpx_mse16x16_mmi(const uint8_t *a, int a_stride,
                          const uint8_t *b, int b_stride,
                          uint32_t *sse) {
  double ftmp[11];
  uint32_t tmp[1];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp10]                           \n\t"
    "li         %[tmp0],    0x10                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_8

    "gsldlc1    %[ftmp1],   0x0f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x08(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[b])                          \n\t"
    VARIANCE_SSE_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "swc1       %[ftmp9],   0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),
      [tmp0]"=&r"(tmp[0]),
      [a]"+&r"(a),                      [b]"+&r"(b)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse;
}

uint32_t vpx_mse16x8_mmi(const uint8_t *a, int a_stride,
                         const uint8_t *b, int b_stride,
                         uint32_t *sse) {
  double ftmp[11];
  uint32_t tmp[1];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp10]                           \n\t"
    "li         %[tmp0],    0x08                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_8

    "gsldlc1    %[ftmp1],   0x0f(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x08(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x0f(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x08(%[b])                          \n\t"
    VARIANCE_SSE_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "swc1       %[ftmp9],   0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),
      [tmp0]"=&r"(tmp[0]),
      [a]"+&r"(a),                      [b]"+&r"(b)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse;
}

uint32_t vpx_mse8x16_mmi(const uint8_t *a, int a_stride,
                         const uint8_t *b, int b_stride,
                         uint32_t *sse) {
  double ftmp[11];
  uint32_t tmp[1];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp10]                           \n\t"
    "li         %[tmp0],    0x10                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "swc1       %[ftmp9],   0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),
      [tmp0]"=&r"(tmp[0]),
      [a]"+&r"(a),                      [b]"+&r"(b)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse;
}

uint32_t vpx_mse8x8_mmi(const uint8_t *a, int a_stride,
                        const uint8_t *b, int b_stride,
                        uint32_t *sse) {
  double ftmp[11];
  uint32_t tmp[1];

  *sse = 0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "mtc1       %[tmp0],    %[ftmp10]                           \n\t"
    "li         %[tmp0],    0x08                                \n\t"
    "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
    "xor        %[ftmp9],   %[ftmp9],       %[ftmp9]            \n\t"
    "1:                                                         \n\t"
    "gsldlc1    %[ftmp1],   0x07(%[a])                          \n\t"
    "gsldrc1    %[ftmp1],   0x00(%[a])                          \n\t"
    "gsldlc1    %[ftmp2],   0x07(%[b])                          \n\t"
    "gsldrc1    %[ftmp2],   0x00(%[b])                          \n\t"
    VARIANCE_SSE_8

    "addiu      %[tmp0],    %[tmp0],        -0x01               \n\t"
    PTR_ADDU   "%[a],       %[a],           %[a_stride]         \n\t"
    PTR_ADDU   "%[b],       %[b],           %[b_stride]         \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "swc1       %[ftmp9],   0x00(%[sse])                        \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
      [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
      [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
      [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
      [ftmp10]"=&f"(ftmp[10]),
      [tmp0]"=&r"(tmp[0]),
      [a]"+&r"(a),                      [b]"+&r"(b)
    : [a_stride]"r"((mips_reg)a_stride),[b_stride]"r"((mips_reg)b_stride),
      [sse]"r"(sse)
    : "memory"
  );

  return *sse;
}
