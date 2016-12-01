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
#include "vpx_mem/vpx_mem.h"

#include "vpx_ports/asmdefs_mmi.h"
#include "vp8/encoder/onyx_int.h"
#include "vp8/encoder/quantize.h"
#include "vp8/common/quant_common.h"

void vp8_fast_quantize_b_mmi(BLOCK *b, BLOCKD *d)
{
#if 1
    int eob;
    short *coeff_ptr   = b->coeff;
    short *round_ptr   = b->round;
    short *quant_ptr   = b->quant_fast;
    short *qcoeff_ptr  = d->qcoeff;
    short *dqcoeff_ptr = d->dqcoeff;
    short *dequant_ptr = d->dequant;

    double ftmp[10];
    uint64_t tmp[1];
    short y[16];

    eob = -1;

    __asm__ volatile (
        "li         %[tmp0],    0x0f                            \n\t"
        "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]        \n\t"
        "mtc1       %[tmp0],    %[ftmp9]                        \n\t"

        "gsldlc1    %[ftmp1],   0x07(%[coeff_ptr])              \n\t"
        "gsldrc1    %[ftmp1],   0x00(%[coeff_ptr])              \n\t"
        "gsldlc1    %[ftmp2],   0x0f(%[coeff_ptr])              \n\t"
        "gsldrc1    %[ftmp2],   0x08(%[coeff_ptr])              \n\t"
        "psrah      %[ftmp3],   %[ftmp1],       %[ftmp9]        \n\t"
        "psrah      %[ftmp4],   %[ftmp2],       %[ftmp9]        \n\t"
        "xor        %[ftmp1],   %[ftmp3],       %[ftmp1]        \n\t"
        "xor        %[ftmp2],   %[ftmp4],       %[ftmp2]        \n\t"
        "psubh      %[ftmp1],   %[ftmp1],       %[ftmp3]        \n\t"
        "psubh      %[ftmp2],   %[ftmp2],       %[ftmp4]        \n\t"
        "gsldlc1    %[ftmp5],   0x07(%[round_ptr])              \n\t"
        "gsldrc1    %[ftmp5],   0x00(%[round_ptr])              \n\t"
        "gsldlc1    %[ftmp6],   0x0f(%[round_ptr])              \n\t"
        "gsldrc1    %[ftmp6],   0x08(%[round_ptr])              \n\t"
        "gsldlc1    %[ftmp7],   0x07(%[quant_ptr])              \n\t"
        "gsldrc1    %[ftmp7],   0x00(%[quant_ptr])              \n\t"
        "gsldlc1    %[ftmp8],   0x0f(%[quant_ptr])              \n\t"
        "gsldrc1    %[ftmp8],   0x08(%[quant_ptr])              \n\t"
        "paddh      %[ftmp5],   %[ftmp5],       %[ftmp1]        \n\t"
        "paddh      %[ftmp6],   %[ftmp6],       %[ftmp2]        \n\t"
        "pmulhuh    %[ftmp5],   %[ftmp5],       %[ftmp7]        \n\t"
        "pmulhuh    %[ftmp6],   %[ftmp6],       %[ftmp8]        \n\t"
        "gssdlc1    %[ftmp5],   0x07(%[y])                      \n\t"
        "gssdrc1    %[ftmp5],   0x00(%[y])                      \n\t"
        "gssdlc1    %[ftmp6],   0x0f(%[y])                      \n\t"
        "gssdrc1    %[ftmp6],   0x08(%[y])                      \n\t"
        "xor        %[ftmp5],   %[ftmp5],       %[ftmp3]        \n\t"
        "xor        %[ftmp6],   %[ftmp6],       %[ftmp4]        \n\t"
        "psubh      %[ftmp5],   %[ftmp5],       %[ftmp3]        \n\t"
        "psubh      %[ftmp6],   %[ftmp6],       %[ftmp4]        \n\t"
        "gssdlc1    %[ftmp5],   0x07(%[qcoeff_ptr])             \n\t"
        "gssdrc1    %[ftmp5],   0x00(%[qcoeff_ptr])             \n\t"
        "gssdlc1    %[ftmp6],   0x0f(%[qcoeff_ptr])             \n\t"
        "gssdrc1    %[ftmp6],   0x08(%[qcoeff_ptr])             \n\t"
        "gsldlc1    %[ftmp7],   0x07(%[dequant_ptr])            \n\t"
        "gsldrc1    %[ftmp7],   0x00(%[dequant_ptr])            \n\t"
        "gsldlc1    %[ftmp8],   0x0f(%[dequant_ptr])            \n\t"
        "gsldrc1    %[ftmp8],   0x08(%[dequant_ptr])            \n\t"
        "pmullh     %[ftmp5],   %[ftmp5],       %[ftmp7]        \n\t"
        "pmullh     %[ftmp6],   %[ftmp6],       %[ftmp8]        \n\t"
        "gssdlc1    %[ftmp5],   0x07(%[dqcoeff_ptr])            \n\t"
        "gssdrc1    %[ftmp5],   0x00(%[dqcoeff_ptr])            \n\t"
        "gssdlc1    %[ftmp6],   0x0f(%[dqcoeff_ptr])            \n\t"
        "gssdrc1    %[ftmp6],   0x08(%[dqcoeff_ptr])            \n\t"

        "gsldlc1    %[ftmp1],   0x17(%[coeff_ptr])              \n\t"
        "gsldrc1    %[ftmp1],   0x10(%[coeff_ptr])              \n\t"
        "gsldlc1    %[ftmp2],   0x1f(%[coeff_ptr])              \n\t"
        "gsldrc1    %[ftmp2],   0x18(%[coeff_ptr])              \n\t"
        "psrah      %[ftmp3],   %[ftmp1],       %[ftmp9]        \n\t"
        "psrah      %[ftmp4],   %[ftmp2],       %[ftmp9]        \n\t"
        "xor        %[ftmp1],   %[ftmp3],       %[ftmp1]        \n\t"
        "xor        %[ftmp2],   %[ftmp4],       %[ftmp2]        \n\t"
        "psubh      %[ftmp1],   %[ftmp1],       %[ftmp3]        \n\t"
        "psubh      %[ftmp2],   %[ftmp2],       %[ftmp4]        \n\t"
        "gsldlc1    %[ftmp5],   0x17(%[round_ptr])              \n\t"
        "gsldrc1    %[ftmp5],   0x10(%[round_ptr])              \n\t"
        "gsldlc1    %[ftmp6],   0x1f(%[round_ptr])              \n\t"
        "gsldrc1    %[ftmp6],   0x18(%[round_ptr])              \n\t"
        "gsldlc1    %[ftmp7],   0x17(%[quant_ptr])              \n\t"
        "gsldrc1    %[ftmp7],   0x10(%[quant_ptr])              \n\t"
        "gsldlc1    %[ftmp8],   0x1f(%[quant_ptr])              \n\t"
        "gsldrc1    %[ftmp8],   0x18(%[quant_ptr])              \n\t"
        "paddh      %[ftmp5],   %[ftmp5],       %[ftmp1]        \n\t"
        "paddh      %[ftmp6],   %[ftmp6],       %[ftmp2]        \n\t"
        "pmulhuh    %[ftmp5],   %[ftmp5],       %[ftmp7]        \n\t"
        "pmulhuh    %[ftmp6],   %[ftmp6],       %[ftmp8]        \n\t"
        "gssdlc1    %[ftmp5],   0x17(%[y])                      \n\t"
        "gssdrc1    %[ftmp5],   0x10(%[y])                      \n\t"
        "gssdlc1    %[ftmp6],   0x1f(%[y])                      \n\t"
        "gssdrc1    %[ftmp6],   0x18(%[y])                      \n\t"
        "xor        %[ftmp5],   %[ftmp5],       %[ftmp3]        \n\t"
        "xor        %[ftmp6],   %[ftmp6],       %[ftmp4]        \n\t"
        "psubh      %[ftmp5],   %[ftmp5],       %[ftmp3]        \n\t"
        "psubh      %[ftmp6],   %[ftmp6],       %[ftmp4]        \n\t"
        "gssdlc1    %[ftmp5],   0x17(%[qcoeff_ptr])             \n\t"
        "gssdrc1    %[ftmp5],   0x10(%[qcoeff_ptr])             \n\t"
        "gssdlc1    %[ftmp6],   0x1f(%[qcoeff_ptr])             \n\t"
        "gssdrc1    %[ftmp6],   0x18(%[qcoeff_ptr])             \n\t"
        "gsldlc1    %[ftmp7],   0x17(%[dequant_ptr])            \n\t"
        "gsldrc1    %[ftmp7],   0x10(%[dequant_ptr])            \n\t"
        "gsldlc1    %[ftmp8],   0x1f(%[dequant_ptr])            \n\t"
        "gsldrc1    %[ftmp8],   0x18(%[dequant_ptr])            \n\t"
        "pmullh     %[ftmp5],   %[ftmp5],       %[ftmp7]        \n\t"
        "pmullh     %[ftmp6],   %[ftmp6],       %[ftmp8]        \n\t"
        "gssdlc1    %[ftmp5],   0x17(%[dqcoeff_ptr])            \n\t"
        "gssdrc1    %[ftmp5],   0x10(%[dqcoeff_ptr])            \n\t"
        "gssdlc1    %[ftmp6],   0x1f(%[dqcoeff_ptr])            \n\t"
        "gssdrc1    %[ftmp6],   0x18(%[dqcoeff_ptr])            \n\t"
        : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
          [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
          [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
          [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
          [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
          [tmp0]"=&r"(tmp[0])
        : [coeff_ptr]"r"((mips_reg)coeff_ptr),
          [qcoeff_ptr]"r"((mips_reg)qcoeff_ptr),
          [dequant_ptr]"r"((mips_reg)dequant_ptr),
          [round_ptr]"r"((mips_reg)round_ptr),
          [quant_ptr]"r"((mips_reg)quant_ptr),
          [dqcoeff_ptr]"r"((mips_reg)dqcoeff_ptr),
          [y]"r"(y)
        : "memory"
    );

    if (y[0]) eob = 0;
    if (y[1]) eob = 1;
    if (y[4]) eob = 2;
    if (y[8]) eob = 3;
    if (y[5]) eob = 4;
    if (y[2]) eob = 5;
    if (y[3]) eob = 6;
    if (y[6]) eob = 7;
    if (y[9]) eob = 8;
    if (y[12]) eob = 9;
    if (y[13]) eob = 10;
    if (y[10]) eob = 11;
    if (y[7]) eob = 12;
    if (y[11]) eob = 13;
    if (y[14]) eob = 14;
    if (y[15]) eob = 15;

    *d->eob = (char)(eob + 1);
#else
    int eob;
    int x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
    int y0, y1, y2, y3, y4, y5, y6, y7, y8, y9, y10, y11, y12, y13, y14, y15;
    short *coeff_ptr   = b->coeff;
    short *round_ptr   = b->round;
    short *quant_ptr   = b->quant_fast;
    short *qcoeff_ptr  = d->qcoeff;
    short *dqcoeff_ptr = d->dqcoeff;
    short *dequant_ptr = d->dequant;

    eob = -1;

    x0  = (coeff_ptr[0] ^ ((int)coeff_ptr[0] >> 31)) - ((int)coeff_ptr[0] >> 31);
    x1  = (coeff_ptr[1] ^ ((int)coeff_ptr[1] >> 31)) - ((int)coeff_ptr[1] >> 31);
    x2  = (coeff_ptr[2] ^ ((int)coeff_ptr[2] >> 31)) - ((int)coeff_ptr[2] >> 31);
    x3  = (coeff_ptr[3] ^ ((int)coeff_ptr[3] >> 31)) - ((int)coeff_ptr[3] >> 31);
    x4  = (coeff_ptr[4] ^ ((int)coeff_ptr[4] >> 31)) - ((int)coeff_ptr[4] >> 31);
    x5  = (coeff_ptr[5] ^ ((int)coeff_ptr[5] >> 31)) - ((int)coeff_ptr[5] >> 31);
    x6  = (coeff_ptr[6] ^ ((int)coeff_ptr[6] >> 31)) - ((int)coeff_ptr[6] >> 31);
    x7  = (coeff_ptr[7] ^ ((int)coeff_ptr[7] >> 31)) - ((int)coeff_ptr[7] >> 31);
    x8  = (coeff_ptr[8] ^ ((int)coeff_ptr[8] >> 31)) - ((int)coeff_ptr[8] >> 31);
    x9  = (coeff_ptr[9] ^ ((int)coeff_ptr[9] >> 31)) - ((int)coeff_ptr[9] >> 31);
    x10  = (coeff_ptr[10] ^ ((int)coeff_ptr[10] >> 31)) - ((int)coeff_ptr[10] >> 31);
    x11  = (coeff_ptr[11] ^ ((int)coeff_ptr[11] >> 31)) - ((int)coeff_ptr[11] >> 31);
    x12  = (coeff_ptr[12] ^ ((int)coeff_ptr[12] >> 31)) - ((int)coeff_ptr[12] >> 31);
    x13  = (coeff_ptr[13] ^ ((int)coeff_ptr[13] >> 31)) - ((int)coeff_ptr[13] >> 31);
    x14  = (coeff_ptr[14] ^ ((int)coeff_ptr[14] >> 31)) - ((int)coeff_ptr[14] >> 31);
    x15  = (coeff_ptr[15] ^ ((int)coeff_ptr[15] >> 31)) - ((int)coeff_ptr[15] >> 31);

    y0  = ((x0 + round_ptr[0]) * quant_ptr[0]) >> 16;
    y1  = ((x1 + round_ptr[1]) * quant_ptr[1]) >> 16;
    y2  = ((x2 + round_ptr[2]) * quant_ptr[2]) >> 16;
    y3  = ((x3 + round_ptr[3]) * quant_ptr[3]) >> 16;
    y4  = ((x4 + round_ptr[4]) * quant_ptr[4]) >> 16;
    y5  = ((x5 + round_ptr[5]) * quant_ptr[5]) >> 16;
    y6  = ((x6 + round_ptr[6]) * quant_ptr[6]) >> 16;
    y7  = ((x7 + round_ptr[7]) * quant_ptr[7]) >> 16;
    y8  = ((x8 + round_ptr[8]) * quant_ptr[8]) >> 16;
    y9  = ((x9 + round_ptr[9]) * quant_ptr[9]) >> 16;
    y10  = ((x10 + round_ptr[10]) * quant_ptr[10]) >> 16;
    y11  = ((x11 + round_ptr[11]) * quant_ptr[11]) >> 16;
    y12  = ((x12 + round_ptr[12]) * quant_ptr[12]) >> 16;
    y13  = ((x13 + round_ptr[13]) * quant_ptr[13]) >> 16;
    y14  = ((x14 + round_ptr[14]) * quant_ptr[14]) >> 16;
    y15  = ((x15 + round_ptr[15]) * quant_ptr[15]) >> 16;

    x0  = (y0 ^ ((int)coeff_ptr[0] >> 31)) - ((int)coeff_ptr[0] >> 31);
    x1  = (y1 ^ ((int)coeff_ptr[1] >> 31)) - ((int)coeff_ptr[1] >> 31);
    x2  = (y2 ^ ((int)coeff_ptr[2] >> 31)) - ((int)coeff_ptr[2] >> 31);
    x3  = (y3 ^ ((int)coeff_ptr[3] >> 31)) - ((int)coeff_ptr[3] >> 31);
    x4  = (y4 ^ ((int)coeff_ptr[4] >> 31)) - ((int)coeff_ptr[4] >> 31);
    x5  = (y5 ^ ((int)coeff_ptr[5] >> 31)) - ((int)coeff_ptr[5] >> 31);
    x6  = (y6 ^ ((int)coeff_ptr[6] >> 31)) - ((int)coeff_ptr[6] >> 31);
    x7  = (y7 ^ ((int)coeff_ptr[7] >> 31)) - ((int)coeff_ptr[7] >> 31);
    x8  = (y8 ^ ((int)coeff_ptr[8] >> 31)) - ((int)coeff_ptr[8] >> 31);
    x9  = (y9 ^ ((int)coeff_ptr[9] >> 31)) - ((int)coeff_ptr[9] >> 31);
    x10  = (y10 ^ ((int)coeff_ptr[10] >> 31)) - ((int)coeff_ptr[10] >> 31);
    x11  = (y11 ^ ((int)coeff_ptr[11] >> 31)) - ((int)coeff_ptr[11] >> 31);
    x12  = (y12 ^ ((int)coeff_ptr[12] >> 31)) - ((int)coeff_ptr[12] >> 31);
    x13  = (y13 ^ ((int)coeff_ptr[13] >> 31)) - ((int)coeff_ptr[13] >> 31);
    x14  = (y14 ^ ((int)coeff_ptr[14] >> 31)) - ((int)coeff_ptr[14] >> 31);
    x15  = (y15 ^ ((int)coeff_ptr[15] >> 31)) - ((int)coeff_ptr[15] >> 31);

    qcoeff_ptr[0] = x0;
    qcoeff_ptr[1] = x1;
    qcoeff_ptr[2] = x2;
    qcoeff_ptr[3] = x3;
    qcoeff_ptr[4] = x4;
    qcoeff_ptr[5] = x5;
    qcoeff_ptr[6] = x6;
    qcoeff_ptr[7] = x7;
    qcoeff_ptr[8] = x8;
    qcoeff_ptr[9] = x9;
    qcoeff_ptr[10] = x10;
    qcoeff_ptr[11] = x11;
    qcoeff_ptr[12] = x12;
    qcoeff_ptr[13] = x13;
    qcoeff_ptr[14] = x14;
    qcoeff_ptr[15] = x15;

    dqcoeff_ptr[0] = x0 * dequant_ptr[0];
    dqcoeff_ptr[1] = x1 * dequant_ptr[1];
    dqcoeff_ptr[2] = x2 * dequant_ptr[2];
    dqcoeff_ptr[3] = x3 * dequant_ptr[3];
    dqcoeff_ptr[4] = x4 * dequant_ptr[4];
    dqcoeff_ptr[5] = x5 * dequant_ptr[5];
    dqcoeff_ptr[6] = x6 * dequant_ptr[6];
    dqcoeff_ptr[7] = x7 * dequant_ptr[7];
    dqcoeff_ptr[8] = x8 * dequant_ptr[8];
    dqcoeff_ptr[9] = x9 * dequant_ptr[9];
    dqcoeff_ptr[10] = x10 * dequant_ptr[10];
    dqcoeff_ptr[11] = x11 * dequant_ptr[11];
    dqcoeff_ptr[12] = x12 * dequant_ptr[12];
    dqcoeff_ptr[13] = x13 * dequant_ptr[13];
    dqcoeff_ptr[14] = x14 * dequant_ptr[14];
    dqcoeff_ptr[15] = x15 * dequant_ptr[15];

    if (y0) eob = 0;
    if (y1) eob = 1;
    if (y4) eob = 2;
    if (y8) eob = 3;
    if (y5) eob = 4;
    if (y2) eob = 5;
    if (y3) eob = 6;
    if (y6) eob = 7;
    if (y9) eob = 8;
    if (y12) eob = 9;
    if (y13) eob = 10;
    if (y10) eob = 11;
    if (y7) eob = 12;
    if (y11) eob = 13;
    if (y14) eob = 14;
    if (y15) eob = 15;

    *d->eob = (char)(eob + 1);
#endif
}

void vp8_regular_quantize_b_mmi(BLOCK *b, BLOCKD *d)
{
    int i, rc, eob;
    int zbin;
    int x, y, z, sz;
    short *zbin_boost_ptr  = b->zrun_zbin_boost;
    short *coeff_ptr       = b->coeff;
    short *zbin_ptr        = b->zbin;
    short *round_ptr       = b->round;
    short *quant_ptr       = b->quant;
    short *quant_shift_ptr = b->quant_shift;
    short *qcoeff_ptr      = d->qcoeff;
    short *dqcoeff_ptr     = d->dqcoeff;
    short *dequant_ptr     = d->dequant;
    short zbin_oq_value    = b->zbin_extra;

    memset(qcoeff_ptr, 0, 32);
    memset(dqcoeff_ptr, 0, 32);

    eob = -1;

    for (i = 0; i < 16; i++) {
        rc   = vp8_default_zig_zag1d[i];
        z    = coeff_ptr[rc];

        zbin = zbin_ptr[rc] + *zbin_boost_ptr + zbin_oq_value;

        zbin_boost_ptr ++;
        sz = (z >> 31);                              /* sign of z */
        x  = (z ^ sz) - sz;                          /* x = abs(z) */

        if (x >= zbin) {
            x += round_ptr[rc];
            y  = ((((x * quant_ptr[rc]) >> 16) + x)
                 * quant_shift_ptr[rc]) >> 16;       /* quantize (x) */
            x  = (y ^ sz) - sz;                      /* get the sign back */
            qcoeff_ptr[rc]  = x;                     /* write to destination */
            dqcoeff_ptr[rc] = x * dequant_ptr[rc];   /* dequantized value */

            if (y) {
                eob = i;                             /* last nonzero coeffs */
                zbin_boost_ptr = b->zrun_zbin_boost; /* reset zero runlength */
            }
        }
    }

    *d->eob = (char)(eob + 1);
}
