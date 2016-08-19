/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp8_rtcd.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/asmdefs_mmi.h"

#define TRANSPOSE_4H(f2, f4, f6, f8, f10, f12, f14, f16, f18, r8, f0, f30) \
        "li "#r8", 0x93                          \n\t" \
        "xor "#f0", "#f0","#f0"                  \n\t" \
        "mtc1 "#r8", "#f30"                      \n\t" \
        "punpcklhw "#f10", "#f2", "#f0"          \n\t" \
        "punpcklhw "#f18", "#f4", "#f0"          \n\t" \
        "pshufh "#f18", "#f18", "#f30"           \n\t" \
        "or "#f10", "#f10", "#f18"               \n\t" \
        "punpckhhw "#f12", "#f2", "#f0"          \n\t" \
        "punpckhhw "#f18", "#f4", "#f0"          \n\t" \
        "pshufh "#f18", "#f18", "#f30"           \n\t" \
        "or "#f12", "#f12", "#f18"               \n\t" \
        "punpcklhw "#f14", "#f6", "#f0"          \n\t" \
        "punpcklhw "#f18", "#f8", "#f0"          \n\t" \
        "pshufh "#f18", "#f18", "#f30"           \n\t" \
        "or "#f14", "#f14", "#f18"               \n\t" \
        "punpckhhw "#f16", "#f6", "#f0"          \n\t" \
        "punpckhhw "#f18", "#f8", "#f0"          \n\t" \
        "pshufh "#f18", "#f18", "#f30"           \n\t" \
        "or "#f16", "#f16", "#f18"               \n\t" \
        "punpcklwd "#f2", "#f10", "#f14"         \n\t" \
        "punpckhwd "#f4", "#f10", "#f14"         \n\t" \
        "punpcklwd "#f6", "#f12", "#f16"         \n\t" \
        "punpckhwd "#f8", "#f12", "#f16"         \n\t"

/****************************************************************************
 * Notes:
 *
 * This implementation makes use of 16 bit fixed point verio of two multiply
 * constants:
 *         1.   sqrt(2) * cos (pi/8)
 *         2.   sqrt(2) * sin (pi/8)
 * Becuase the first constant is bigger than 1, to maintain the same 16 bit
 * fixed point precision as the second one, we use a trick of
 *         x * a = x + x*(a-1)
 * so
 *         x * sqrt(2) * cos (pi/8) = x + x * (sqrt(2) *cos(pi/8)-1).
 **************************************************************************/

void vp8_short_idct4x4llm_mmi(short *input, unsigned char *pred_ptr,
                              int pred_stride, unsigned char *dst_ptr,
                              int dst_stride)
{
#if 1
    uint32_t tmp[0];
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_04)   = {0x0004000400040004ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_4e7b) = {0x4e7b4e7b4e7b4e7bULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_22a3) = {0x22a322a322a322a3ULL};
#if _MIPS_SIM == _ABIO32
    register double ftmp0 asm ("$f0");
    register double ftmp1 asm ("$f2");
    register double ftmp2 asm ("$f4");
    register double ftmp3 asm ("$f6");
    register double ftmp4 asm ("$f8");
    register double ftmp5 asm ("$f10");
    register double ftmp6 asm ("$f12");
    register double ftmp7 asm ("$f14");
    register double ftmp8 asm ("$f16");
    register double ftmp9 asm ("$f18");
    register double ftmp10 asm ("$f20");
    register double ftmp11 asm ("$f22");
#else
    register double ftmp0 asm ("$f0");
    register double ftmp1 asm ("$f1");
    register double ftmp2 asm ("$f2");
    register double ftmp3 asm ("$f3");
    register double ftmp4 asm ("$f4");
    register double ftmp5 asm ("$f5");
    register double ftmp6 asm ("$f6");
    register double ftmp7 asm ("$f7");
    register double ftmp8 asm ("$f8");
    register double ftmp9 asm ("$f9");
    register double ftmp10 asm ("$f10");
    register double ftmp11 asm ("$f11");
#endif  // _MIPS_SIM == _ABIO32

    __asm__ volatile (
        "li         %[tmp0],    0x02                                \n\t"
        "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
        "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"

        "gsldlc1    %[ftmp1],   0x07(%[ip])                         \n\t"
        "gsldrc1    %[ftmp1],   0x00(%[ip])                         \n\t"
        "gsldlc1    %[ftmp2],   0x0f(%[ip])                         \n\t"
        "gsldrc1    %[ftmp2],   0x08(%[ip])                         \n\t"
        "gsldlc1    %[ftmp3],   0x17(%[ip])                         \n\t"
        "gsldrc1    %[ftmp3],   0x10(%[ip])                         \n\t"
        "gsldlc1    %[ftmp4],   0x1f(%[ip])                         \n\t"
        "gsldrc1    %[ftmp4],   0x18(%[ip])                         \n\t"

        // ip[0...3] + ip[8...11]
        "paddh      %[ftmp5],   %[ftmp1],       %[ftmp3]            \n\t"
        // ip[0...3] - ip[8...11]
        "psubh      %[ftmp6],   %[ftmp1],       %[ftmp3]            \n\t"
        // (ip[12...15] * sinpi8sqrt2) >> 16
        "psllh      %[ftmp9],   %[ftmp4],       %[ftmp11]           \n\t"
        "pmulhh     %[ftmp7],   %[ftmp9],       %[ff_ph_22a3]       \n\t"
        // (ip[ 4... 7] * sinpi8sqrt2) >> 16
        "psllh      %[ftmp9],   %[ftmp2],       %[ftmp11]           \n\t"
        "pmulhh     %[ftmp8],   %[ftmp9],       %[ff_ph_22a3]       \n\t"
        // ip[ 4... 7] + ((ip[ 4... 7] * cospi8sqrt2minus1) >> 16)
        "pmulhh     %[ftmp9],   %[ftmp2],       %[ff_ph_4e7b]       \n\t"
        "paddh      %[ftmp9],   %[ftmp9],       %[ftmp2]            \n\t"
        // ip[12...15] + ((ip[12...15] * cospi8sqrt2minus1) >> 16)
        "pmulhh     %[ftmp10],  %[ftmp4],       %[ff_ph_4e7b]       \n\t"
        "paddh      %[ftmp10],  %[ftmp10],      %[ftmp4]            \n\t"

        "paddh      %[ftmp1],   %[ftmp5],       %[ftmp7]            \n\t"
        "paddh      %[ftmp1],   %[ftmp1],       %[ftmp9]            \n\t"
        "paddh      %[ftmp2],   %[ftmp6],       %[ftmp8]            \n\t"
        "psubh      %[ftmp2],   %[ftmp2],       %[ftmp10]           \n\t"
        "psubh      %[ftmp3],   %[ftmp6],       %[ftmp8]            \n\t"
        "paddh      %[ftmp3],   %[ftmp3],       %[ftmp10]           \n\t"
        "psubh      %[ftmp4],   %[ftmp5],       %[ftmp7]            \n\t"
        "psubh      %[ftmp4],   %[ftmp4],       %[ftmp9]            \n\t"

        TRANSPOSE_4H(%[ftmp1], %[ftmp2], %[ftmp3], %[ftmp4],
                     %[ftmp5], %[ftmp6], %[ftmp7], %[ftmp8],
                     %[ftmp9], %[tmp0],  %[ftmp0], %[ftmp10])

        // a
        "paddh      %[ftmp5],   %[ftmp1],       %[ftmp3]            \n\t"
        // b
        "psubh      %[ftmp6],   %[ftmp1],       %[ftmp3]            \n\t"
        // c
        "psllh      %[ftmp9],   %[ftmp2],       %[ftmp11]           \n\t"
        "pmulhh     %[ftmp9],   %[ftmp9],       %[ff_ph_22a3]       \n\t"
        "psubh      %[ftmp7],   %[ftmp9],       %[ftmp4]            \n\t"
        "pmulhh     %[ftmp10],  %[ftmp4],       %[ff_ph_4e7b]       \n\t"
        "psubh      %[ftmp7],   %[ftmp7],       %[ftmp10]           \n\t"
        // d
        "psllh      %[ftmp9],   %[ftmp4],       %[ftmp11]           \n\t"
        "pmulhh     %[ftmp9],   %[ftmp9],       %[ff_ph_22a3]       \n\t"
        "paddh      %[ftmp8],   %[ftmp9],       %[ftmp2]            \n\t"
        "pmulhh     %[ftmp10],  %[ftmp2],       %[ff_ph_4e7b]       \n\t"
        "paddh      %[ftmp8],   %[ftmp8],       %[ftmp10]           \n\t"

        "li         %[tmp0],    0x03                                \n\t"
        "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
        // a + d
        "paddh      %[ftmp1],   %[ftmp5],       %[ftmp8]            \n\t"
        "paddh      %[ftmp1],   %[ftmp1],       %[ff_ph_04]         \n\t"
        "psrah      %[ftmp1],   %[ftmp1],       %[ftmp11]           \n\t"
        // b + c
        "paddh      %[ftmp2],   %[ftmp6],       %[ftmp7]            \n\t"
        "paddh      %[ftmp2],   %[ftmp2],       %[ff_ph_04]         \n\t"
        "psrah      %[ftmp2],   %[ftmp2],       %[ftmp11]           \n\t"
        // b - c
        "psubh      %[ftmp3],   %[ftmp6],       %[ftmp7]            \n\t"
        "paddh      %[ftmp3],   %[ftmp3],       %[ff_ph_04]         \n\t"
        "psrah      %[ftmp3],   %[ftmp3],       %[ftmp11]           \n\t"
        // a - d
        "psubh      %[ftmp4],   %[ftmp5],       %[ftmp8]            \n\t"
        "paddh      %[ftmp4],   %[ftmp4],       %[ff_ph_04]         \n\t"
        "psrah      %[ftmp4],   %[ftmp4],       %[ftmp11]           \n\t"

        TRANSPOSE_4H(%[ftmp1], %[ftmp2], %[ftmp3], %[ftmp4],
                     %[ftmp5], %[ftmp6], %[ftmp7], %[ftmp8],
                     %[ftmp9], %[tmp0],  %[ftmp0], %[ftmp10])

#if _MIPS_SIM == _ABIO32
        "ulw        %[tmp0],    0x00(%[pred_prt])                   \n\t"
        "mtc1       %[tmp0],    %[ftmp5]                            \n\t"
#else
        "gslwlc1    %[ftmp5],   0x03(%[pred_ptr])                   \n\t"
        "gslwrc1    %[ftmp5],   0x00(%[pred_ptr])                   \n\t"
#endif
        "punpcklbh  %[ftmp5],   %[ftmp5],       %[ftmp0]            \n\t"
        "paddh      %[ftmp1],   %[ftmp1],       %[ftmp5]            \n\t"
        "packushb   %[ftmp1],   %[ftmp1],       %[ftmp0]            \n\t"
        "gsswlc1    %[ftmp1],   0x03(%[dst_ptr])                    \n\t"
        "gsswrc1    %[ftmp1],   0x00(%[dst_ptr])                    \n\t"
        PTR_ADDU   "%[pred_ptr],%[pred_ptr],    %[pred_stride]      \n\t"
        PTR_ADDU   "%[dst_ptr], %[dst_ptr],     %[dst_stride]       \n\t"

#if _MIPS_SIM == _ABIO32
        "ulw        %[tmp0],    0x00(%[pred_prt])                   \n\t"
        "mtc1       %[tmp0],    %[ftmp6]                            \n\t"
#else
        "gslwlc1    %[ftmp6],   0x03(%[pred_ptr])                   \n\t"
        "gslwrc1    %[ftmp6],   0x00(%[pred_ptr])                   \n\t"
#endif
        "punpcklbh  %[ftmp6],   %[ftmp6],       %[ftmp0]            \n\t"
        "paddh      %[ftmp2],   %[ftmp2],       %[ftmp6]            \n\t"
        "packushb   %[ftmp2],   %[ftmp2],       %[ftmp0]            \n\t"
        "gsswlc1    %[ftmp2],   0x03(%[dst_ptr])                    \n\t"
        "gsswrc1    %[ftmp2],   0x00(%[dst_ptr])                    \n\t"
        PTR_ADDU   "%[pred_ptr],%[pred_ptr],    %[pred_stride]      \n\t"
        PTR_ADDU   "%[dst_ptr], %[dst_ptr],     %[dst_stride]       \n\t"

#if _MIPS_SIM == _ABIO32
        "ulw        %[tmp0],    0x00(%[pred_prt])                   \n\t"
        "mtc1       %[tmp0],    %[ftmp7]                            \n\t"
#else
        "gslwlc1    %[ftmp7],   0x03(%[pred_ptr])                   \n\t"
        "gslwrc1    %[ftmp7],   0x00(%[pred_ptr])                   \n\t"
#endif
        "punpcklbh  %[ftmp7],   %[ftmp7],       %[ftmp0]            \n\t"
        "paddh      %[ftmp3],   %[ftmp3],       %[ftmp7]            \n\t"
        "packushb   %[ftmp3],   %[ftmp3],       %[ftmp0]            \n\t"
        "gsswlc1    %[ftmp3],   0x03(%[dst_ptr])                    \n\t"
        "gsswrc1    %[ftmp3],   0x00(%[dst_ptr])                    \n\t"
        PTR_ADDU   "%[pred_ptr],%[pred_ptr],    %[pred_stride]      \n\t"
        PTR_ADDU   "%[dst_ptr], %[dst_ptr],     %[dst_stride]       \n\t"

#if _MIPS_SIM == _ABIO32
        "ulw        %[tmp0],    0x00(%[pred_prt])                   \n\t"
        "mtc1       %[tmp0],    %[ftmp8]                            \n\t"
#else
        "gslwlc1    %[ftmp8],   0x03(%[pred_ptr])                   \n\t"
        "gslwrc1    %[ftmp8],   0x00(%[pred_ptr])                   \n\t"
#endif
        "punpcklbh  %[ftmp8],   %[ftmp8],       %[ftmp0]            \n\t"
        "paddh      %[ftmp4],   %[ftmp4],       %[ftmp8]            \n\t"
        "packushb   %[ftmp4],   %[ftmp4],       %[ftmp0]            \n\t"
        "gsswlc1    %[ftmp4],   0x03(%[dst_ptr])                    \n\t"
        "gsswrc1    %[ftmp4],   0x00(%[dst_ptr])                    \n\t"
        : [ftmp0]"=&f"(ftmp0),                [ftmp1]"=&f"(ftmp1),
          [ftmp2]"=&f"(ftmp2),                [ftmp3]"=&f"(ftmp3),
          [ftmp4]"=&f"(ftmp4),                [ftmp5]"=&f"(ftmp5),
          [ftmp6]"=&f"(ftmp6),                [ftmp7]"=&f"(ftmp7),
          [ftmp8]"=&f"(ftmp8),                [ftmp9]"=&f"(ftmp9),
          [ftmp10]"=&f"(ftmp10),              [ftmp11]"=&f"(ftmp11),
          [tmp0]"=&r"(tmp[0]),
          [pred_ptr]"+&r"(pred_ptr),          [dst_ptr]"+&r"(dst_ptr)
        : [ip]"r"(input),
          [ff_ph_22a3]"f"(ff_ph_22a3),        [ff_ph_4e7b]"f"(ff_ph_4e7b),
          [ff_ph_04]"f"(ff_ph_04),
          [pred_stride]"r"((mips_reg)pred_stride),
          [dst_stride]"r"((mips_reg)dst_stride)
        : "memory"
    );
#else
    int r, c;
    short a1, b1, c1, d1;
    short a2, b2, c2, d2;
    short a3, b3, c3, d3;
    short a4, b4, c4, d4;
    short output[16];
    short *ip = input;
    short *op = output;
    static const int cospi8sqrt2minus1 = 20091;
    static const int sinpi8sqrt2       = 35468;

    op[ 0] = ip[ 0] + ip[ 8] + ((ip[12] * sinpi8sqrt2) >> 16) + (ip[ 4] + ((ip[ 4] * cospi8sqrt2minus1) >> 16));
    op[ 1] = ip[ 1] + ip[ 9] + ((ip[13] * sinpi8sqrt2) >> 16) + (ip[ 5] + ((ip[ 5] * cospi8sqrt2minus1) >> 16));
    op[ 2] = ip[ 2] + ip[10] + ((ip[14] * sinpi8sqrt2) >> 16) + (ip[ 6] + ((ip[ 6] * cospi8sqrt2minus1) >> 16));
    op[ 3] = ip[ 3] + ip[11] + ((ip[15] * sinpi8sqrt2) >> 16) + (ip[ 7] + ((ip[ 7] * cospi8sqrt2minus1) >> 16));

    op[ 4] = ip[ 0] - ip[ 8] + ((ip[ 4] * sinpi8sqrt2) >> 16) - (ip[12] + ((ip[12] * cospi8sqrt2minus1) >> 16));
    op[ 5] = ip[ 1] - ip[ 9] + ((ip[ 5] * sinpi8sqrt2) >> 16) - (ip[13] + ((ip[13] * cospi8sqrt2minus1) >> 16));
    op[ 6] = ip[ 2] - ip[10] + ((ip[ 6] * sinpi8sqrt2) >> 16) - (ip[14] + ((ip[14] * cospi8sqrt2minus1) >> 16));
    op[ 7] = ip[ 3] - ip[11] + ((ip[ 7] * sinpi8sqrt2) >> 16) - (ip[15] + ((ip[15] * cospi8sqrt2minus1) >> 16));

    op[ 8] = ip[ 0] - ip[ 8] - ((ip[ 4] * sinpi8sqrt2) >> 16) + (ip[12] + ((ip[12] * cospi8sqrt2minus1) >> 16));
    op[ 9] = ip[ 1] - ip[ 9] - ((ip[ 5] * sinpi8sqrt2) >> 16) + (ip[13] + ((ip[13] * cospi8sqrt2minus1) >> 16));
    op[10] = ip[ 2] - ip[10] - ((ip[ 6] * sinpi8sqrt2) >> 16) + (ip[14] + ((ip[14] * cospi8sqrt2minus1) >> 16));
    op[11] = ip[ 3] - ip[11] - ((ip[ 7] * sinpi8sqrt2) >> 16) + (ip[15] + ((ip[15] * cospi8sqrt2minus1) >> 16));

    op[12] = ip[ 0] + ip[ 8] - ((ip[12] * sinpi8sqrt2) >> 16) - (ip[ 4] + ((ip[ 4] * cospi8sqrt2minus1) >> 16));
    op[13] = ip[ 1] + ip[ 9] - ((ip[13] * sinpi8sqrt2) >> 16) - (ip[ 5] + ((ip[ 5] * cospi8sqrt2minus1) >> 16));
    op[14] = ip[ 2] + ip[10] - ((ip[14] * sinpi8sqrt2) >> 16) - (ip[ 6] + ((ip[ 6] * cospi8sqrt2minus1) >> 16));
    op[15] = ip[ 3] + ip[11] - ((ip[15] * sinpi8sqrt2) >> 16) - (ip[ 7] + ((ip[ 7] * cospi8sqrt2minus1) >> 16));

    ip = output;
    op = output;

    a1 = ip[ 0] + ip[ 2];
    a2 = ip[ 4] + ip[ 6];
    a3 = ip[ 8] + ip[10];
    a4 = ip[12] + ip[14];

    b1 = ip[ 0] - ip[ 2];
    b2 = ip[ 4] - ip[ 6];
    b3 = ip[ 8] - ip[10];
    b4 = ip[12] - ip[14];

    c1 = ((ip[ 1] * sinpi8sqrt2) >> 16) - (ip[ 3] + ((ip[ 3] * cospi8sqrt2minus1) >> 16));
    c2 = ((ip[ 5] * sinpi8sqrt2) >> 16) - (ip[ 7] + ((ip[ 7] * cospi8sqrt2minus1) >> 16));
    c3 = ((ip[ 9] * sinpi8sqrt2) >> 16) - (ip[11] + ((ip[11] * cospi8sqrt2minus1) >> 16));
    c4 = ((ip[13] * sinpi8sqrt2) >> 16) - (ip[15] + ((ip[15] * cospi8sqrt2minus1) >> 16));

    d1 = (ip[ 1] + ((ip[ 1] * cospi8sqrt2minus1) >> 16)) + ((ip[ 3] * sinpi8sqrt2) >> 16);
    d2 = (ip[ 5] + ((ip[ 5] * cospi8sqrt2minus1) >> 16)) + ((ip[ 7] * sinpi8sqrt2) >> 16);
    d3 = (ip[ 9] + ((ip[ 9] * cospi8sqrt2minus1) >> 16)) + ((ip[11] * sinpi8sqrt2) >> 16);
    d4 = (ip[13] + ((ip[13] * cospi8sqrt2minus1) >> 16)) + ((ip[15] * sinpi8sqrt2) >> 16);

    op[ 0] = (a1 + d1 + 4) >> 3;
    op[ 4] = (a2 + d2 + 4) >> 3;
    op[ 8] = (a3 + d3 + 4) >> 3;
    op[12] = (a4 + d4 + 4) >> 3;

    op[ 1] = (b1 + c1 + 4) >> 3;
    op[ 5] = (b2 + c2 + 4) >> 3;
    op[ 9] = (b3 + c3 + 4) >> 3;
    op[13] = (b4 + c4 + 4) >> 3;

    op[ 2] = (b1 - c1 + 4) >> 3;
    op[ 6] = (b2 - c2 + 4) >> 3;
    op[10] = (b3 - c3 + 4) >> 3;
    op[14] = (b4 - c4 + 4) >> 3;

    op[ 3] = (a1 - d1 + 4) >> 3;
    op[ 7] = (a2 - d2 + 4) >> 3;
    op[11] = (a3 - d3 + 4) >> 3;
    op[15] = (a4 - d4 + 4) >> 3;

    ip = output;
    for (r = 0; r < 4; r++)
      {
        for (c = 0; c < 4; c++)
          {
            int a = ip[c] + pred_ptr[c] ;

            if (a < 0) a = 0;
            if (a > 255) a = 255;
            dst_ptr[c] = (unsigned char) a ;
          }
        ip += 4;
        dst_ptr += dst_stride;
        pred_ptr += pred_stride;
      }
#endif
}

void vp8_dc_only_idct_add_mmi(short input_dc, unsigned char *pred_ptr,
                              int pred_stride, unsigned char *dst_ptr,
                              int dst_stride)
{
#if 1
    int a1 = ((input_dc + 4) >> 3);
    double ftmp[5];
    int low32;

    __asm__ volatile (
        "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]        \n\t"
        "pshufh     %[a1],      %[a1],          %[ftmp0]        \n\t"
        "ulw        %[low32],   0x00(%[pred_ptr])               \n\t"
        "mtc1       %[low32],   %[ftmp1]                        \n\t"
        "punpcklbh  %[ftmp2],   %[ftmp1],       %[ftmp0]        \n\t"
        "paddsh     %[ftmp2],   %[ftmp2],       %[a1]           \n\t"
        "packushb   %[ftmp1],   %[ftmp2],       %[ftmp0]        \n\t"
        "gsswlc1    %[ftmp1],   0x03(%[dst_ptr])                \n\t"
        "gsswrc1    %[ftmp1],   0x00(%[dst_ptr])                \n\t"
        PTR_ADDU   "%[pred_ptr],%[pred_ptr],    %[pred_stride]  \n\t"
        PTR_ADDU   "%[dst_ptr], %[dst_ptr],     %[dst_stride]   \n\t"

        "ulw        %[low32],   0x00(%[pred_ptr])               \n\t"
        "mtc1       %[low32],   %[ftmp1]                        \n\t"
        "punpcklbh  %[ftmp2],   %[ftmp1],       %[ftmp0]        \n\t"
        "paddsh     %[ftmp2],   %[ftmp2],       %[a1]           \n\t"
        "packushb   %[ftmp1],   %[ftmp2],       %[ftmp0]        \n\t"
        "gsswlc1    %[ftmp1],   0x03(%[dst_ptr])                \n\t"
        "gsswrc1    %[ftmp1],   0x00(%[dst_ptr])                \n\t"
        PTR_ADDU   "%[pred_ptr],%[pred_ptr],    %[pred_stride]  \n\t"
        PTR_ADDU   "%[dst_ptr], %[dst_ptr],     %[dst_stride]   \n\t"

        "ulw        %[low32],   0x00(%[pred_ptr])               \n\t"
        "mtc1       %[low32],   %[ftmp1]                        \n\t"
        "punpcklbh  %[ftmp2],   %[ftmp1],       %[ftmp0]        \n\t"
        "paddsh     %[ftmp2],   %[ftmp2],       %[a1]           \n\t"
        "packushb   %[ftmp1],   %[ftmp2],       %[ftmp0]        \n\t"
        "gsswlc1    %[ftmp1],   0x03(%[dst_ptr])                \n\t"
        "gsswrc1    %[ftmp1],   0x00(%[dst_ptr])                \n\t"
        PTR_ADDU   "%[pred_ptr],%[pred_ptr],    %[pred_stride]  \n\t"
        PTR_ADDU   "%[dst_ptr], %[dst_ptr],     %[dst_stride]   \n\t"

        "ulw        %[low32],   0x00(%[pred_ptr])               \n\t"
        "mtc1       %[low32],   %[ftmp1]                        \n\t"
        "punpcklbh  %[ftmp2],   %[ftmp1],       %[ftmp0]        \n\t"
        "paddsh     %[ftmp2],   %[ftmp2],       %[a1]           \n\t"
        "packushb   %[ftmp1],   %[ftmp2],       %[ftmp0]        \n\t"
        "gsswlc1    %[ftmp1],   0x03(%[dst_ptr])                \n\t"
        "gsswrc1    %[ftmp1],   0x00(%[dst_ptr])                \n\t"
        : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
          [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
          [ftmp4]"=&f"(ftmp[4]),
          [low32]"=&r"(low32),
          [dst_ptr]"+&r"(dst_ptr),          [pred_ptr]"+&r"(pred_ptr)
        : [dst_stride]"r"((mips_reg)dst_stride),
          [pred_stride]"r"((mips_reg)pred_stride),
          [a1]"f"(a1)
        : "memory"
    );
#else
    int a1 = ((input_dc + 4) >> 3);
    int r, c;

    for (r = 0; r < 4; r++)
    {
        for (c = 0; c < 4; c++)
        {
            int a = a1 + pred_ptr[c] ;

            if (a < 0)
                a = 0;

            if (a > 255)
                a = 255;

            dst_ptr[c] = (unsigned char) a ;
        }

        dst_ptr += dst_stride;
        pred_ptr += pred_stride;
    }

#endif
}

void vp8_short_inv_walsh4x4_mmi(short *input, short *mb_dqcoeff)
{
#if 1
    int i;
    short output[16];
    uint32_t tmp[1];
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_03) = {0x0003000300030003ULL};
#if _MIPS_SIM == _ABIO32
    register double ftmp0 asm ("$f0");
    register double ftmp1 asm ("$f2");
    register double ftmp2 asm ("$f4");
    register double ftmp3 asm ("$f6");
    register double ftmp4 asm ("$f8");
    register double ftmp5 asm ("$f10");
    register double ftmp6 asm ("$f12");
    register double ftmp7 asm ("$f14");
    register double ftmp8 asm ("$f16");
    register double ftmp9 asm ("$f18");
    register double ftmp10 asm ("$f20");
    register double ftmp11 asm ("$f22");
#else
    register double ftmp0 asm ("$f0");
    register double ftmp1 asm ("$f1");
    register double ftmp2 asm ("$f2");
    register double ftmp3 asm ("$f3");
    register double ftmp4 asm ("$f4");
    register double ftmp5 asm ("$f5");
    register double ftmp6 asm ("$f6");
    register double ftmp7 asm ("$f7");
    register double ftmp8 asm ("$f8");
    register double ftmp9 asm ("$f9");
    register double ftmp10 asm ("$f10");
    register double ftmp11 asm ("$f11");
#endif  // _MIPS_SIM == _ABIO32

    __asm__ volatile (
      "li         %[tmp0],    0x03                                \n\t"
      "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
      "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
      "gsldlc1    %[ftmp1],   0x07(%[ip])                         \n\t"
      "gsldrc1    %[ftmp1],   0x00(%[ip])                         \n\t"
      "gsldlc1    %[ftmp2],   0x0f(%[ip])                         \n\t"
      "gsldrc1    %[ftmp2],   0x08(%[ip])                         \n\t"
      "gsldlc1    %[ftmp3],   0x17(%[ip])                         \n\t"
      "gsldrc1    %[ftmp3],   0x10(%[ip])                         \n\t"
      "gsldlc1    %[ftmp4],   0x1f(%[ip])                         \n\t"
      "gsldrc1    %[ftmp4],   0x18(%[ip])                         \n\t"
      "paddh      %[ftmp5],   %[ftmp1],       %[ftmp2]            \n\t"
      "psubh      %[ftmp6],   %[ftmp1],       %[ftmp2]            \n\t"
      "paddh      %[ftmp7],   %[ftmp3],       %[ftmp4]            \n\t"
      "psubh      %[ftmp8],   %[ftmp3],       %[ftmp4]            \n\t"

      "paddh      %[ftmp1],   %[ftmp5],       %[ftmp7]            \n\t"
      "psubh      %[ftmp2],   %[ftmp5],       %[ftmp7]            \n\t"
      "psubh      %[ftmp3],   %[ftmp6],       %[ftmp8]            \n\t"
      "paddh      %[ftmp4],   %[ftmp6],       %[ftmp8]            \n\t"

      TRANSPOSE_4H(%[ftmp1], %[ftmp2], %[ftmp3], %[ftmp4],
                   %[ftmp5], %[ftmp6], %[ftmp7], %[ftmp8],
                   %[ftmp9], %[tmp0],  %[ftmp0], %[ftmp10])

      // a
      "paddh      %[ftmp5],   %[ftmp1],       %[ftmp4]            \n\t"
      // d
      "psubh      %[ftmp6],   %[ftmp1],       %[ftmp4]            \n\t"
      // b
      "paddh      %[ftmp7],   %[ftmp2],       %[ftmp3]            \n\t"
      // c
      "psubh      %[ftmp8],   %[ftmp2],       %[ftmp3]            \n\t"

      "paddh      %[ftmp1],   %[ftmp5],       %[ftmp7]            \n\t"
      "paddh      %[ftmp2],   %[ftmp6],       %[ftmp8]            \n\t"
      "psubh      %[ftmp3],   %[ftmp5],       %[ftmp7]            \n\t"
      "psubh      %[ftmp4],   %[ftmp6],       %[ftmp8]            \n\t"

      "paddh      %[ftmp1],   %[ftmp1],       %[ff_ph_03]         \n\t"
      "psrah      %[ftmp1],   %[ftmp1],       %[ftmp11]           \n\t"
      "paddh      %[ftmp2],   %[ftmp2],       %[ff_ph_03]         \n\t"
      "psrah      %[ftmp2],   %[ftmp2],       %[ftmp11]           \n\t"
      "paddh      %[ftmp3],   %[ftmp3],       %[ff_ph_03]         \n\t"
      "psrah      %[ftmp3],   %[ftmp3],       %[ftmp11]           \n\t"
      "paddh      %[ftmp4],   %[ftmp4],       %[ff_ph_03]         \n\t"
      "psrah      %[ftmp4],   %[ftmp4],       %[ftmp11]           \n\t"

      TRANSPOSE_4H(%[ftmp1], %[ftmp2], %[ftmp3], %[ftmp4],
                   %[ftmp5], %[ftmp6], %[ftmp7], %[ftmp8],
                   %[ftmp9], %[tmp0],  %[ftmp0], %[ftmp10])

      "gssdlc1    %[ftmp1],   0x07(%[op])                         \n\t"
      "gssdrc1    %[ftmp1],   0x00(%[op])                         \n\t"
      "gssdlc1    %[ftmp2],   0x0f(%[op])                         \n\t"
      "gssdrc1    %[ftmp2],   0x08(%[op])                         \n\t"
      "gssdlc1    %[ftmp3],   0x17(%[op])                         \n\t"
      "gssdrc1    %[ftmp3],   0x10(%[op])                         \n\t"
      "gssdlc1    %[ftmp4],   0x1f(%[op])                         \n\t"
      "gssdrc1    %[ftmp4],   0x18(%[op])                         \n\t"
      : [ftmp0]"=&f"(ftmp0),                [ftmp1]"=&f"(ftmp1),
        [ftmp2]"=&f"(ftmp2),                [ftmp3]"=&f"(ftmp3),
        [ftmp4]"=&f"(ftmp4),                [ftmp5]"=&f"(ftmp5),
        [ftmp6]"=&f"(ftmp6),                [ftmp7]"=&f"(ftmp7),
        [ftmp8]"=&f"(ftmp8),                [ftmp9]"=&f"(ftmp9),
        [ftmp10]"=&f"(ftmp10),              [ftmp11]"=&f"(ftmp11),
        [tmp0]"=&r"(tmp[0])
      : [ip]"r"(input),                     [op]"r"(output),
        [ff_ph_03]"f"(ff_ph_03)
      : "memory"
    );

    for(i = 0; i < 16; i++)
    {
        mb_dqcoeff[i * 16] = output[i];
    }
#else
    short output[16];
    int i;
    int a1, b1, c1, d1;
    int a2, b2, c2, d2;
    short *ip = input;
    short *op = output;

    for (i = 0; i < 4; i++)
    {
        a1 = ip[0] + ip[12];
        b1 = ip[4] + ip[8];
        c1 = ip[4] - ip[8];
        d1 = ip[0] - ip[12];

        op[0] = a1 + b1;
        op[4] = c1 + d1;
        op[8] = a1 - b1;
        op[12] = d1 - c1;
        ip++;
        op++;
    }

    ip = output;
    op = output;

    for (i = 0; i < 4; i++)
    {
        a1 = ip[0] + ip[3];
        b1 = ip[1] + ip[2];
        c1 = ip[1] - ip[2];
        d1 = ip[0] - ip[3];

        a2 = a1 + b1;
        b2 = c1 + d1;
        c2 = a1 - b1;
        d2 = d1 - c1;

        op[0] = (a2 + 3) >> 3;
        op[1] = (b2 + 3) >> 3;
        op[2] = (c2 + 3) >> 3;
        op[3] = (d2 + 3) >> 3;

        ip += 4;
        op += 4;
    }

    for(i = 0; i < 16; i++)
    {
        mb_dqcoeff[i * 16] = output[i];
    }
#endif
}

void vp8_short_inv_walsh4x4_1_mmi(short *input, short *mb_dqcoeff)
{
    int i;
    int a1;

    a1 = ((input[0] + 3) >> 3);
    for(i = 0; i < 16; i++)
    {
        mb_dqcoeff[i * 16] = a1;
    }
}
