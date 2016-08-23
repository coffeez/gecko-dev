/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <math.h>
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

void vp8_short_fdct4x4_mmi(short *input, short *output, int pitch)
{
#if 1
    int pitch_half = pitch/2;
    uint64_t tmp[1];

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
    register double ftmp12 asm ("$f24");
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
    register double ftmp12 asm ("$f12");
#endif  // _MIPS_SIM == _ABIO32

    DECLARE_ALIGNED(8, const uint64_t, ff_ph_01)    = {0x0001000100010001ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_07)    = {0x0007000700070007ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_pw_12000) = {0x00002ee000002ee0ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_pw_51000) = {0x0000c7380000c738ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_pw_14500) = {0x000038a4000038a4ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_pw_7500)  = {0x00001d4c00001d4cULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_2217)  = {0x08a908a908a908a9ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_5352)  = {0x14e814e814e814e8ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_535x)  = {0x14ea14e914e814e8ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_neg5352) = {0xeb18eb18eb18eb18ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_neg535x) = {0xeb16eb17eb18eb18ULL};

    DECLARE_ALIGNED(16, int, a[4]);
    DECLARE_ALIGNED(16, int, b[4]);
    DECLARE_ALIGNED(16, int, c[4]);
    DECLARE_ALIGNED(16, int, d[4]);

    //stage1
    a[0] = (input[0]              + input[3]             ) * 8;
    a[1] = (input[0+pitch_half]   + input[3+pitch_half]  ) * 8;
    a[2] = (input[0+2*pitch_half] + input[3+2*pitch_half]) * 8;
    a[3] = (input[0+3*pitch_half] + input[3+3*pitch_half]) * 8;

    b[0] = (input[1]              + input[2]             ) * 8;
    b[1] = (input[1+pitch_half]   + input[2+pitch_half]  ) * 8;
    b[2] = (input[1+2*pitch_half] + input[2+2*pitch_half]) * 8;
    b[3] = (input[1+3*pitch_half] + input[2+3*pitch_half]) * 8;

    c[0] = (input[1]              - input[2]             ) * 8;
    c[1] = (input[1+pitch_half]   - input[2+pitch_half]  ) * 8;
    c[2] = (input[1+2*pitch_half] - input[2+2*pitch_half]) * 8;
    c[3] = (input[1+3*pitch_half] - input[2+3*pitch_half]) * 8;

    d[0] = (input[0]              - input[3]             ) * 8;
    d[1] = (input[0+pitch_half]   - input[3+pitch_half]  ) * 8;
    d[2] = (input[0+2*pitch_half] - input[3+2*pitch_half]) * 8;
    d[3] = (input[0+3*pitch_half] - input[3+3*pitch_half]) * 8;

    __asm__ volatile (
        "gslqc1     %[ftmp2],   %[ftmp1],       0x00(%[a])      \n\t"
        "gslqc1     %[ftmp4],   %[ftmp3],       0x00(%[b])      \n\t"
        "gslqc1     %[ftmp6],   %[ftmp5],       0x00(%[c])      \n\t"
        "gslqc1     %[ftmp8],   %[ftmp7],       0x00(%[d])      \n\t"

        "paddw      %[ftmp9],   %[ftmp1],       %[ftmp3]        \n\t"
        "paddw      %[ftmp10],  %[ftmp2],       %[ftmp4]        \n\t"
        "psubw      %[ftmp11],  %[ftmp1],       %[ftmp3]        \n\t"
        "psubw      %[ftmp12],  %[ftmp2],       %[ftmp4]        \n\t"
        "packsswh   %[ftmp1],   %[ftmp9],       %[ftmp10]       \n\t"
        "packsswh   %[ftmp3],   %[ftmp11],      %[ftmp12]       \n\t"
        "packsswh   %[ftmp2],   %[ftmp5],       %[ftmp6]        \n\t"
        "packsswh   %[ftmp4],   %[ftmp7],       %[ftmp8]        \n\t"
        "li         %[tmp0],    0x0c                            \n\t"
        "mov.d      %[ftmp7],   %[ftmp2]                        \n\t"
        "mov.d      %[ftmp8],   %[ftmp4]                        \n\t"
        "mtc1       %[tmp0],    %[ftmp11]                       \n\t"

        "ldc1       %[ftmp12],  %[ff_pw_14500]                  \n\t"
        "punpcklhw  %[ftmp9],   %[ftmp7],       %[ftmp8]        \n\t"
        "punpcklhw  %[ftmp10],  %[ff_ph_2217],  %[ff_ph_5352]   \n\t"
        "pmaddhw    %[ftmp5],   %[ftmp9],       %[ftmp10]       \n\t"
        "punpckhhw  %[ftmp9],   %[ftmp7],       %[ftmp8]        \n\t"
        "punpckhhw  %[ftmp10],  %[ff_ph_2217],  %[ff_ph_5352]   \n\t"
        "pmaddhw    %[ftmp6],   %[ftmp9],       %[ftmp10]       \n\t"
        "paddw      %[ftmp5],   %[ftmp5],       %[ftmp12]       \n\t"
        "paddw      %[ftmp6],   %[ftmp6],       %[ftmp12]       \n\t"
        "psraw      %[ftmp5],   %[ftmp5],       %[ftmp11]       \n\t"
        "psraw      %[ftmp6],   %[ftmp6],       %[ftmp11]       \n\t"
        "packsswh   %[ftmp2],   %[ftmp5],       %[ftmp6]        \n\t"

        "ldc1       %[ftmp12],  %[ff_pw_7500]                   \n\t"
        "punpcklhw  %[ftmp9],   %[ftmp8],       %[ftmp7]        \n\t"
        "punpcklhw  %[ftmp10],  %[ff_ph_2217],  %[ff_ph_neg5352]\n\t"
        "pmaddhw    %[ftmp5],   %[ftmp9],       %[ftmp10]       \n\t"
        "punpckhhw  %[ftmp9],   %[ftmp8],       %[ftmp7]        \n\t"
        "punpckhhw  %[ftmp10],  %[ff_ph_2217],  %[ff_ph_neg5352]\n\t"
        "pmaddhw    %[ftmp6],   %[ftmp9],       %[ftmp10]       \n\t"
        "paddw      %[ftmp5],   %[ftmp5],       %[ftmp12]       \n\t"
        "paddw      %[ftmp6],   %[ftmp6],       %[ftmp12]       \n\t"
        "psraw      %[ftmp5],   %[ftmp5],       %[ftmp11]       \n\t"
        "psraw      %[ftmp6],   %[ftmp6],       %[ftmp11]       \n\t"
        "packsswh   %[ftmp4],   %[ftmp5],       %[ftmp6]        \n\t"

        TRANSPOSE_4H(%[ftmp1], %[ftmp2], %[ftmp3], %[ftmp4],
                     %[ftmp5], %[ftmp6], %[ftmp7], %[ftmp8],
                     %[ftmp9], %[tmp0], %[ftmp0], %[ftmp10])

        : [ftmp0]"=&f"(ftmp0),              [ftmp1]"=&f"(ftmp1),
          [ftmp2]"=&f"(ftmp2),              [ftmp3]"=&f"(ftmp3),
          [ftmp4]"=&f"(ftmp4),              [ftmp5]"=&f"(ftmp5),
          [ftmp6]"=&f"(ftmp6),              [ftmp7]"=&f"(ftmp7),
          [ftmp8]"=&f"(ftmp8),              [ftmp9]"=&f"(ftmp9),
          [ftmp10]"=&f"(ftmp10),            [ftmp11]"=&f"(ftmp11),
          [ftmp12]"=&f"(ftmp12),
          [tmp0]"=&r"(tmp[0])
        : [pitch_half]"r"((mips_reg)pitch_half),
          [a]"r"(a),                        [b]"r"(b),
          [c]"r"(c),                        [d]"r"(d),
          [ff_ph_2217]"f"(ff_ph_2217),
          [ff_ph_5352]"f"(ff_ph_5352),      [ff_ph_neg5352]"f"(ff_ph_neg5352),
          [ff_pw_14500]"m"(ff_pw_14500),    [ff_pw_7500]"m"(ff_pw_7500)
    );

    __asm__ volatile (
        "gssdlc1    %[ftmp1],   0x07(%[output])                 \n\t"
        "gssdrc1    %[ftmp1],   0x00(%[output])                 \n\t"
        "gssdlc1    %[ftmp2],   0x0f(%[output])                 \n\t"
        "gssdrc1    %[ftmp2],   0x08(%[output])                 \n\t"
        "gssdlc1    %[ftmp3],   0x17(%[output])                 \n\t"
        "gssdrc1    %[ftmp3],   0x10(%[output])                 \n\t"
        "gssdlc1    %[ftmp4],   0x1f(%[output])                 \n\t"
        "gssdrc1    %[ftmp4],   0x18(%[output])                 \n\t"
        ::[ftmp1]"f"(ftmp1),                [ftmp2]"f"(ftmp2),
          [ftmp3]"f"(ftmp3),                [ftmp4]"f"(ftmp4),
          [output]"r"(output)
        : "memory"
    );

    //stage2
    __asm__ volatile (
        "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]        \n\t"
        "gsldlc1    %[ftmp1],   0x07(%[output])                 \n\t"
        "gsldrc1    %[ftmp1],   0x00(%[output])                 \n\t"
        "gsldlc1    %[ftmp2],   0x0f(%[output])                 \n\t"
        "gsldrc1    %[ftmp2],   0x08(%[output])                 \n\t"
        "gsldlc1    %[ftmp3],   0x17(%[output])                 \n\t"
        "gsldrc1    %[ftmp3],   0x10(%[output])                 \n\t"
        "gsldlc1    %[ftmp4],   0x1f(%[output])                 \n\t"
        "gsldrc1    %[ftmp4],   0x18(%[output])                 \n\t"

        "paddh      %[ftmp5],   %[ftmp1],       %[ftmp4]        \n\t"
        "paddh      %[ftmp6],   %[ftmp2],       %[ftmp3]        \n\t"
        "psubh      %[ftmp7],   %[ftmp2],       %[ftmp3]        \n\t"
        "psubh      %[ftmp8],   %[ftmp1],       %[ftmp4]        \n\t"

        "pcmpeqh    %[ftmp0],   %[ftmp8],       %[ftmp0]        \n\t"
        "ldc1       %[ftmp9],   %[ff_ph_01]                     \n\t"
        "paddh      %[ftmp0],   %[ftmp0],       %[ftmp9]        \n\t"

        "paddh      %[ftmp1],   %[ftmp5],       %[ftmp6]        \n\t"
        "psubh      %[ftmp2],   %[ftmp5],       %[ftmp6]        \n\t"
        "ldc1       %[ftmp9],   %[ff_ph_07]                     \n\t"
        "li         %[tmp0],    0x04                            \n\t"
        "paddh      %[ftmp1],   %[ftmp1],       %[ftmp9]        \n\t"
        "paddh      %[ftmp2],   %[ftmp2],       %[ftmp9]        \n\t"
        "mtc1       %[tmp0],    %[ftmp9]                        \n\t"
        "psrah      %[ftmp1],   %[ftmp1],       %[ftmp9]        \n\t"
        "psrah      %[ftmp2],   %[ftmp2],       %[ftmp9]        \n\t"

        "li         %[tmp0],    0x10                            \n\t"
        "ldc1       %[ftmp12],  %[ff_pw_12000]                  \n\t"
        "mtc1       %[tmp0],    %[ftmp9]                        \n\t"

        "punpcklhw  %[ftmp5],   %[ftmp7],       %[ftmp8]        \n\t"
        "punpcklhw  %[ftmp6],   %[ff_ph_2217],  %[ff_ph_535x]   \n\t"
        "pmaddhw    %[ftmp10],  %[ftmp5],       %[ftmp6]        \n\t"
        "punpckhhw  %[ftmp5],   %[ftmp7],       %[ftmp8]        \n\t"
        "punpckhhw  %[ftmp6],   %[ff_ph_2217],  %[ff_ph_535x]   \n\t"
        "pmaddhw    %[ftmp11],  %[ftmp5],       %[ftmp6]        \n\t"
        "paddw      %[ftmp10],  %[ftmp10],      %[ftmp12]       \n\t"
        "paddw      %[ftmp11],  %[ftmp11],      %[ftmp12]       \n\t"
        "psraw      %[ftmp10],  %[ftmp10],      %[ftmp9]        \n\t"
        "psraw      %[ftmp11],  %[ftmp11],      %[ftmp9]        \n\t"
        "packsswh   %[ftmp3],   %[ftmp10],      %[ftmp11]       \n\t"
        "paddh      %[ftmp3],   %[ftmp3],       %[ftmp0]        \n\t"

        "ldc1       %[ftmp12],  %[ff_pw_51000]                  \n\t"
        "punpcklhw  %[ftmp5],   %[ftmp8],       %[ftmp7]        \n\t"
        "punpcklhw  %[ftmp6],   %[ff_ph_2217],  %[ff_ph_neg535x]\n\t"
        "pmaddhw    %[ftmp10],  %[ftmp5],       %[ftmp6]        \n\t"
        "punpckhhw  %[ftmp5],   %[ftmp8],       %[ftmp7]        \n\t"
        "punpckhhw  %[ftmp6],   %[ff_ph_2217],  %[ff_ph_neg535x]\n\t"
        "pmaddhw    %[ftmp11],  %[ftmp5],       %[ftmp6]        \n\t"
        "paddw      %[ftmp10],  %[ftmp10],      %[ftmp12]       \n\t"
        "paddw      %[ftmp11],  %[ftmp11],      %[ftmp12]       \n\t"
        "psraw      %[ftmp10],  %[ftmp10],      %[ftmp9]        \n\t"
        "psraw      %[ftmp11],  %[ftmp11],      %[ftmp9]        \n\t"
        "packsswh   %[ftmp4],   %[ftmp10],      %[ftmp11]       \n\t"
        : [ftmp0]"=&f"(ftmp0),              [ftmp1]"=&f"(ftmp1),
          [ftmp2]"=&f"(ftmp2),              [ftmp3]"=&f"(ftmp3),
          [ftmp4]"=&f"(ftmp4),              [ftmp5]"=&f"(ftmp5),
          [ftmp6]"=&f"(ftmp6),              [ftmp7]"=&f"(ftmp7),
          [ftmp8]"=&f"(ftmp8),              [ftmp9]"=&f"(ftmp9),
          [ftmp10]"=&f"(ftmp10),            [ftmp11]"=&f"(ftmp11),
          [ftmp12]"=&f"(ftmp12),
          [tmp0]"=r"(tmp[0])
        : [ff_ph_01]"m"(ff_ph_01),          [ff_ph_07]"m"(ff_ph_07),
          [ff_ph_2217]"f"(ff_ph_2217),
          [ff_ph_535x]"f"(ff_ph_535x),      [ff_ph_neg535x]"f"(ff_ph_neg535x),
          [ff_pw_12000]"m"(ff_pw_12000),    [ff_pw_51000]"m"(ff_pw_51000),
          [output]"r"(output)
    );

    __asm__ volatile (
        "gssdlc1    %[ftmp1],   0x07(%[output])                 \n\t"
        "gssdrc1    %[ftmp1],   0x00(%[output])                 \n\t"
        "gssdlc1    %[ftmp3],   0x0f(%[output])                 \n\t"
        "gssdrc1    %[ftmp3],   0x08(%[output])                 \n\t"
        "gssdlc1    %[ftmp2],   0x17(%[output])                 \n\t"
        "gssdrc1    %[ftmp2],   0x10(%[output])                 \n\t"
        "gssdlc1    %[ftmp4],   0x1f(%[output])                 \n\t"
        "gssdrc1    %[ftmp4],   0x18(%[output])                 \n\t"
        ::[ftmp1]"f"(ftmp1),                [ftmp2]"f"(ftmp2),
          [ftmp3]"f"(ftmp3),                [ftmp4]"f"(ftmp4),
          [output]"r"(output)
        : "memory"
    );

#else

    int a1, b1, c1, d1;
    int a2, b2, c2, d2;
    int a3, b3, c3, d3;
    int a4, b4, c4, d4;
    short *ip = input;
    short *op = output;
    int pitch_half = pitch/2;

    //stage1
    a1 = ((ip[0] + ip[3]) * 8);
    a2 = ((ip[0+pitch_half] + ip[3+pitch_half]) * 8);
    a3 = ((ip[0+2*pitch_half] + ip[3+2*pitch_half]) * 8);
    a4 = ((ip[0+3*pitch_half] + ip[3+3*pitch_half]) * 8);

    b1 = ((ip[1] + ip[2]) * 8);
    b2 = ((ip[1+pitch_half] + ip[2+pitch_half]) * 8);
    b3 = ((ip[1+2*pitch_half] + ip[2+2*pitch_half]) * 8);
    b4 = ((ip[1+3*pitch_half] + ip[2+3*pitch_half]) * 8);

    c1 = ((ip[1] - ip[2]) * 8);
    c2 = ((ip[1+pitch_half] - ip[2+pitch_half]) * 8);
    c3 = ((ip[1+2*pitch_half] - ip[2+2*pitch_half]) * 8);
    c4 = ((ip[1+3*pitch_half] - ip[2+3*pitch_half]) * 8);

    d1 = ((ip[0] - ip[3]) * 8);
    d2 = ((ip[0+pitch_half] - ip[3+pitch_half]) * 8);
    d3 = ((ip[0+2*pitch_half] - ip[3+2*pitch_half]) * 8);
    d4 = ((ip[0+3*pitch_half] - ip[3+3*pitch_half]) * 8);

    op[ 0] = a1 + b1;
    op[ 4] = a2 + b2;
    op[ 8] = a3 + b3;
    op[12] = a4 + b4;

    op[ 1] = (c1 * 2217 + d1 * 5352 +  14500)>>12;
    op[ 5] = (c2 * 2217 + d2 * 5352 +  14500)>>12;
    op[ 9] = (c3 * 2217 + d3 * 5352 +  14500)>>12;
    op[13] = (c4 * 2217 + d4 * 5352 +  14500)>>12;

    op[ 2] = a1 - b1;
    op[ 6] = a2 - b2;
    op[10] = a3 - b3;
    op[14] = a4 - b4;

    op[ 3] = (d1 * 2217 - c1 * 5352 +   7500)>>12;
    op[ 7] = (d2 * 2217 - c2 * 5352 +   7500)>>12;
    op[11] = (d3 * 2217 - c3 * 5352 +   7500)>>12;
    op[15] = (d4 * 2217 - c4 * 5352 +   7500)>>12;

    //stage2
    ip = output;
    op = output;

    a1 = ip[0] + ip[12];
    a2 = ip[1] + ip[13];
    a3 = ip[2] + ip[14];
    a4 = ip[3] + ip[15];

    b1 = ip[4] + ip[8];
    b2 = ip[5] + ip[9];
    b3 = ip[6] + ip[10];
    b4 = ip[7] + ip[11];

    c1 = ip[4] - ip[8];
    c2 = ip[5] - ip[9];
    c3 = ip[6] - ip[10];
    c4 = ip[7] - ip[11];

    d1 = ip[0] - ip[12];
    d2 = ip[1] - ip[13];
    d3 = ip[2] - ip[14];
    d4 = ip[3] - ip[15];

    op[ 0]  = ( a1 + b1 + 7)>>4;
    op[ 1]  = ( a2 + b2 + 7)>>4;
    op[ 2]  = ( a3 + b3 + 7)>>4;
    op[ 3]  = ( a4 + b4 + 7)>>4;

    op[ 8]  = ( a1 - b1 + 7)>>4;
    op[ 9]  = ( a2 - b2 + 7)>>4;
    op[10]  = ( a3 - b3 + 7)>>4;
    op[11]  = ( a4 - b4 + 7)>>4;

    op[ 4]  =((c1 * 2217 + d1 * 5352 +  12000)>>16) + (d1!=0);
    op[ 5]  =((c2 * 2217 + d2 * 5352 +  12000)>>16) + (d2!=0);
    op[ 6]  =((c3 * 2217 + d3 * 5353 +  12000)>>16) + (d3!=0);
    op[ 7]  =((c4 * 2217 + d4 * 5354 +  12000)>>16) + (d4!=0);

    op[12]  = (d1 * 2217 - c1 * 5352 +  51000)>>16;
    op[13]  = (d2 * 2217 - c2 * 5352 +  51000)>>16;
    op[14]  = (d3 * 2217 - c3 * 5353 +  51000)>>16;
    op[15]  = (d4 * 2217 - c4 * 5354 +  51000)>>16;
#endif
}

void vp8_short_fdct8x4_mmi(short *input, short *output, int pitch)
{
    vp8_short_fdct4x4_mmi(input,   output,    pitch);
    vp8_short_fdct4x4_mmi(input + 4, output + 16, pitch);
}

void vp8_short_walsh4x4_mmi(short *input, short *output, int pitch)
{
#if 1
    double ftmp[13];
    uint32_t tmp[1];
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_01)   = {0x0001000100010001ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_pw_01)   = {0x0000000100000001ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_pw_03)   = {0x0000000300000003ULL};
    DECLARE_ALIGNED(8, const uint64_t, ff_pw_mask) = {0x0001000000010000ULL};

    __asm__ volatile (
        "li         %[tmp0],    0x02                                \n\t"
        "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]            \n\t"
        "mtc1       %[tmp0],    %[ftmp11]                           \n\t"

        "gsldlc1    %[ftmp1],   0x07(%[ip])                         \n\t"
        "gsldrc1    %[ftmp1],   0x00(%[ip])                         \n\t"
        PTR_ADDU   "%[ip],      %[ip],          %[pitch]            \n\t"
        "gsldlc1    %[ftmp2],   0x07(%[ip])                         \n\t"
        "gsldrc1    %[ftmp2],   0x00(%[ip])                         \n\t"
        PTR_ADDU   "%[ip],      %[ip],          %[pitch]            \n\t"
        "gsldlc1    %[ftmp3],   0x07(%[ip])                         \n\t"
        "gsldrc1    %[ftmp3],   0x00(%[ip])                         \n\t"
        PTR_ADDU   "%[ip],      %[ip],          %[pitch]            \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[ip])                         \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[ip])                         \n\t"

        TRANSPOSE_4H(%[ftmp1], %[ftmp2], %[ftmp3], %[ftmp4],
                     %[ftmp5], %[ftmp6], %[ftmp7], %[ftmp8],
                     %[ftmp9], %[tmp0],  %[ftmp0], %[ftmp10])

        "psllh      %[ftmp1],   %[ftmp1],       %[ftmp11]           \n\t"
        "psllh      %[ftmp2],   %[ftmp2],       %[ftmp11]           \n\t"
        "psllh      %[ftmp3],   %[ftmp3],       %[ftmp11]           \n\t"
        "psllh      %[ftmp4],   %[ftmp4],       %[ftmp11]           \n\t"
        // a
        "paddh      %[ftmp5],   %[ftmp1],       %[ftmp3]            \n\t"
        // d
        "paddh      %[ftmp6],   %[ftmp2],       %[ftmp4]            \n\t"
        // c
        "psubh      %[ftmp7],   %[ftmp2],       %[ftmp4]            \n\t"
        // b
        "psubh      %[ftmp8],   %[ftmp1],       %[ftmp3]            \n\t"

        // a + d
        "paddh      %[ftmp1],   %[ftmp5],       %[ftmp6]            \n\t"
        // b + c
        "paddh      %[ftmp2],   %[ftmp8],       %[ftmp7]            \n\t"
        // b - c
        "psubh      %[ftmp3],   %[ftmp8],       %[ftmp7]            \n\t"
        // a - d
        "psubh      %[ftmp4],   %[ftmp5],       %[ftmp6]            \n\t"

        "dmtc1      %[ff_ph_01],%[ftmp11]                           \n\t"
        "pcmpeqh    %[ftmp6],   %[ftmp5],       %[ftmp0]            \n\t"
        "paddh      %[ftmp6],   %[ftmp6],       %[ftmp11]           \n\t"
        "paddh      %[ftmp1],   %[ftmp1],       %[ftmp6]            \n\t"

        TRANSPOSE_4H(%[ftmp1], %[ftmp2], %[ftmp3], %[ftmp4],
                     %[ftmp5], %[ftmp6], %[ftmp7], %[ftmp8],
                     %[ftmp9], %[tmp0],  %[ftmp0], %[ftmp10])

        // op[2], op[0]
        "pmaddhw    %[ftmp5],   %[ftmp1],       %[ff_pw_01]         \n\t"
        // op[3], op[1]
        "pmaddhw    %[ftmp1],   %[ftmp1],       %[ff_pw_mask]       \n\t"

        // op[6], op[4]
        "pmaddhw    %[ftmp6],   %[ftmp2],       %[ff_pw_01]         \n\t"
        // op[7], op[5]
        "pmaddhw    %[ftmp2],   %[ftmp2],       %[ff_pw_mask]       \n\t"

        // op[10], op[8]
        "pmaddhw    %[ftmp7],   %[ftmp3],       %[ff_pw_01]         \n\t"
        // op[11], op[9]
        "pmaddhw    %[ftmp3],   %[ftmp3],       %[ff_pw_mask]       \n\t"

        // op[14], op[12]
        "pmaddhw    %[ftmp8],   %[ftmp4],       %[ff_pw_01]         \n\t"
        // op[15], op[13]
        "pmaddhw    %[ftmp4],   %[ftmp4],       %[ff_pw_mask]       \n\t"

        // a1, a3
        "paddw      %[ftmp9],   %[ftmp5],       %[ftmp7]            \n\t"
        // d1, d3
        "paddw      %[ftmp10],  %[ftmp6],       %[ftmp8]            \n\t"
        // c1, c3
        "psubw      %[ftmp11],  %[ftmp6],       %[ftmp8]            \n\t"
        // b1, b3
        "psubw      %[ftmp12],  %[ftmp5],       %[ftmp7]            \n\t"

        // a1 + d1, a3 + d3
        "paddw      %[ftmp5],   %[ftmp9],       %[ftmp10]           \n\t"
        // b1 + c1, b3 + c3
        "paddw      %[ftmp6],   %[ftmp12],      %[ftmp11]           \n\t"
        // b1 - c1, b3 - c3
        "psubw      %[ftmp7],   %[ftmp12],      %[ftmp11]           \n\t"
        // a1 - d1, a3 - d3
        "psubw      %[ftmp8],   %[ftmp9],       %[ftmp10]           \n\t"

        // a2, a4
        "paddw      %[ftmp9],   %[ftmp1],       %[ftmp3]            \n\t"
        // d2, d4
        "paddw      %[ftmp10],  %[ftmp2],       %[ftmp4]            \n\t"
        // c2, c4
        "psubw      %[ftmp11],  %[ftmp2],       %[ftmp4]            \n\t"
        // b2, b4
        "psubw      %[ftmp12],  %[ftmp1],       %[ftmp3]            \n\t"

        // a2 + d2, a4 + d4
        "paddw      %[ftmp1],   %[ftmp9],       %[ftmp10]           \n\t"
        // b2 + c2, b4 + c4
        "paddw      %[ftmp2],   %[ftmp12],      %[ftmp11]           \n\t"
        // b2 - c2, b4 - c4
        "psubw      %[ftmp3],   %[ftmp12],      %[ftmp11]           \n\t"
        // a2 - d2, a4 - d4
        "psubw      %[ftmp4],   %[ftmp9],       %[ftmp10]           \n\t"

        "li         %[tmp0],    0x03                                \n\t"
        "mtc1       %[tmp0],    %[ftmp11]                           \n\t"

        "pcmpgtw    %[ftmp9],   %[ftmp0],       %[ftmp1]            \n\t"
        "and        %[ftmp9],   %[ftmp9],       %[ff_pw_01]         \n\t"
        "paddw      %[ftmp1],   %[ftmp1],       %[ftmp9]            \n\t"
        "paddw      %[ftmp1],   %[ftmp1],       %[ff_pw_03]         \n\t"
        "psraw      %[ftmp1],   %[ftmp1],       %[ftmp11]           \n\t"

        "pcmpgtw    %[ftmp9],   %[ftmp0],       %[ftmp2]            \n\t"
        "and        %[ftmp9],   %[ftmp9],       %[ff_pw_01]         \n\t"
        "paddw      %[ftmp2],   %[ftmp2],       %[ftmp9]            \n\t"
        "paddw      %[ftmp2],   %[ftmp2],       %[ff_pw_03]         \n\t"
        "psraw      %[ftmp2],   %[ftmp2],       %[ftmp11]           \n\t"

        "pcmpgtw    %[ftmp9],   %[ftmp0],       %[ftmp3]            \n\t"
        "and        %[ftmp9],   %[ftmp9],       %[ff_pw_01]         \n\t"
        "paddw      %[ftmp3],   %[ftmp3],       %[ftmp9]            \n\t"
        "paddw      %[ftmp3],   %[ftmp3],       %[ff_pw_03]         \n\t"
        "psraw      %[ftmp3],   %[ftmp3],       %[ftmp11]           \n\t"

        "pcmpgtw    %[ftmp9],   %[ftmp0],       %[ftmp4]            \n\t"
        "and        %[ftmp9],   %[ftmp9],       %[ff_pw_01]         \n\t"
        "paddw      %[ftmp4],   %[ftmp4],       %[ftmp9]            \n\t"
        "paddw      %[ftmp4],   %[ftmp4],       %[ff_pw_03]         \n\t"
        "psraw      %[ftmp4],   %[ftmp4],       %[ftmp11]           \n\t"

        "pcmpgtw    %[ftmp9],   %[ftmp0],       %[ftmp5]            \n\t"
        "and        %[ftmp9],   %[ftmp9],       %[ff_pw_01]         \n\t"
        "paddw      %[ftmp5],   %[ftmp5],       %[ftmp9]            \n\t"
        "paddw      %[ftmp5],   %[ftmp5],       %[ff_pw_03]         \n\t"
        "psraw      %[ftmp5],   %[ftmp5],       %[ftmp11]           \n\t"

        "pcmpgtw    %[ftmp9],   %[ftmp0],       %[ftmp6]            \n\t"
        "and        %[ftmp9],   %[ftmp9],       %[ff_pw_01]         \n\t"
        "paddw      %[ftmp6],   %[ftmp6],       %[ftmp9]            \n\t"
        "paddw      %[ftmp6],   %[ftmp6],       %[ff_pw_03]         \n\t"
        "psraw      %[ftmp6],   %[ftmp6],       %[ftmp11]           \n\t"

        "pcmpgtw    %[ftmp9],   %[ftmp0],       %[ftmp7]            \n\t"
        "and        %[ftmp9],   %[ftmp9],       %[ff_pw_01]         \n\t"
        "paddw      %[ftmp7],   %[ftmp7],       %[ftmp9]            \n\t"
        "paddw      %[ftmp7],   %[ftmp7],       %[ff_pw_03]         \n\t"
        "psraw      %[ftmp7],   %[ftmp7],       %[ftmp11]           \n\t"

        "pcmpgtw    %[ftmp9],   %[ftmp0],       %[ftmp8]            \n\t"
        "and        %[ftmp9],   %[ftmp9],       %[ff_pw_01]         \n\t"
        "paddw      %[ftmp8],   %[ftmp8],       %[ftmp9]            \n\t"
        "paddw      %[ftmp8],   %[ftmp8],       %[ff_pw_03]         \n\t"
        "psraw      %[ftmp8],   %[ftmp8],       %[ftmp11]           \n\t"

        "packsswh   %[ftmp1],   %[ftmp1],       %[ftmp5]            \n\t"
        "packsswh   %[ftmp2],   %[ftmp2],       %[ftmp6]            \n\t"
        "packsswh   %[ftmp3],   %[ftmp3],       %[ftmp7]            \n\t"
        "packsswh   %[ftmp4],   %[ftmp4],       %[ftmp8]            \n\t"

        "li         %[tmp0],    0x72                                \n\t"
        "mtc1       %[tmp0],    %[ftmp11]                           \n\t"
        "pshufh     %[ftmp1],   %[ftmp1],       %[ftmp11]           \n\t"
        "pshufh     %[ftmp2],   %[ftmp2],       %[ftmp11]           \n\t"
        "pshufh     %[ftmp3],   %[ftmp3],       %[ftmp11]           \n\t"
        "pshufh     %[ftmp4],   %[ftmp4],       %[ftmp11]           \n\t"

        "gssdlc1    %[ftmp1],   0x07(%[op])                         \n\t"
        "gssdrc1    %[ftmp1],   0x00(%[op])                         \n\t"
        "gssdlc1    %[ftmp2],   0x0f(%[op])                         \n\t"
        "gssdrc1    %[ftmp2],   0x08(%[op])                         \n\t"
        "gssdlc1    %[ftmp3],   0x17(%[op])                         \n\t"
        "gssdrc1    %[ftmp3],   0x10(%[op])                         \n\t"
        "gssdlc1    %[ftmp4],   0x1f(%[op])                         \n\t"
        "gssdrc1    %[ftmp4],   0x18(%[op])                         \n\t"
        : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
          [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
          [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
          [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
          [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
          [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
          [ftmp12]"=&f"(ftmp[12]),
          [tmp0]"=&r"(tmp[0]),
          [ip]"+&r"(input)
        : [op]"r"(output),
          [ff_pw_01]"f"(ff_pw_01),          [pitch]"r"((mips_reg)pitch),
          [ff_pw_03]"f"(ff_pw_03),          [ff_pw_mask]"f"(ff_pw_mask),
          [ff_ph_01]"r"(ff_ph_01)
        : "memory"
    );
#else
    int a1, b1, c1, d1;
    int a2, b2, c2, d2;
    int a3, b3, c3, d3;
    int a4, b4, c4, d4;
    short *ip = input;
    short *op = output;

    pitch = pitch / 2;

    // stage 1
    a1 = ((ip[0          ] + ip[2          ]) * 4);
    a2 = ((ip[0 + pitch  ] + ip[2 + pitch  ]) * 4);
    a3 = ((ip[0 + pitch*2] + ip[2 + pitch*2]) * 4);
    a4 = ((ip[0 + pitch*3] + ip[2 + pitch*3]) * 4);

    d1 = ((ip[1          ] + ip[3          ]) * 4);
    d2 = ((ip[1 + pitch  ] + ip[3 + pitch  ]) * 4);
    d3 = ((ip[1 + pitch*2] + ip[3 + pitch*2]) * 4);
    d4 = ((ip[1 + pitch*3] + ip[3 + pitch*3]) * 4);

    c1 = ((ip[1          ] - ip[3          ]) * 4);
    c2 = ((ip[1 + pitch  ] - ip[3 + pitch  ]) * 4);
    c3 = ((ip[1 + pitch*2] - ip[3 + pitch*2]) * 4);
    c4 = ((ip[1 + pitch*3] - ip[3 + pitch*3]) * 4);

    b1 = ((ip[0          ] - ip[2          ]) * 4);
    b2 = ((ip[0 + pitch  ] - ip[2 + pitch  ]) * 4);
    b3 = ((ip[0 + pitch*2] - ip[2 + pitch*2]) * 4);
    b4 = ((ip[0 + pitch*3] - ip[2 + pitch*3]) * 4);

    op[ 0] = a1 + d1 + (a1!=0);
    op[ 4] = a2 + d2 + (a2!=0);
    op[ 8] = a3 + d3 + (a3!=0);
    op[12] = a4 + d4 + (a4!=0);

    op[ 1] = b1 + c1;
    op[ 5] = b2 + c2;
    op[ 9] = b3 + c3;
    op[13] = b4 + c4;

    op[ 2] = b1 - c1;
    op[ 6] = b2 - c2;
    op[10] = b3 - c3;
    op[14] = b4 - c4;

    op[ 3] = a1 - d1;
    op[ 7] = a2 - d2;
    op[11] = a3 - d3;
    op[15] = a4 - d4;

    // stage 2
    a1 = op[0] + op[ 8];
    a2 = op[1] + op[ 9];
    a3 = op[2] + op[10];
    a4 = op[3] + op[11];

    d1 = op[4] + op[12];
    d2 = op[5] + op[13];
    d3 = op[6] + op[14];
    d4 = op[7] + op[15];

    c1 = op[4] - op[12];
    c2 = op[5] - op[13];
    c3 = op[6] - op[14];
    c4 = op[7] - op[15];

    b1 = op[0] - op[ 8];
    b2 = op[1] - op[ 9];
    b3 = op[2] - op[10];
    b4 = op[3] - op[11];

    op[ 0] = (a1 + d1 + ((a1 + d1)<0) + 3) >> 3;
    op[ 1] = (a2 + d2 + ((a2 + d2)<0) + 3) >> 3;
    op[ 2] = (a3 + d3 + ((a3 + d3)<0) + 3) >> 3;
    op[ 3] = (a4 + d4 + ((a4 + d4)<0) + 3) >> 3;

    op[ 4] = (b1 + c1 + ((b1 + c1)<0) + 3) >> 3;
    op[ 5] = (b2 + c2 + ((b2 + c2)<0) + 3) >> 3;
    op[ 6] = (b3 + c3 + ((b3 + c3)<0) + 3) >> 3;
    op[ 7] = (b4 + c4 + ((b4 + c4)<0) + 3) >> 3;

    op[ 8] = (b1 - c1 + ((b1 - c1)<0) + 3) >> 3;
    op[ 9] = (b2 - c2 + ((b2 - c2)<0) + 3) >> 3;
    op[10] = (b3 - c3 + ((b3 - c3)<0) + 3) >> 3;
    op[11] = (b4 - c4 + ((b4 - c4)<0) + 3) >> 3;

    op[12] = (a1 - d1 + ((a1 - d1)<0) + 3) >> 3;
    op[13] = (a2 - d2 + ((a2 - d2)<0) + 3) >> 3;
    op[14] = (a3 - d3 + ((a3 - d3)<0) + 3) >> 3;
    op[15] = (a4 - d4 + ((a4 - d4)<0) + 3) >> 3;
#endif
}
