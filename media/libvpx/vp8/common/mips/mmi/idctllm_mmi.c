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
#include "vpx_ports/asmdefs_mmi.h"

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
static const int cospi8sqrt2minus1 = 20091;
static const int sinpi8sqrt2      = 35468;

void vp8_short_idct4x4llm_mmi(short *input, unsigned char *pred_ptr,
                              int pred_stride, unsigned char *dst_ptr,
                              int dst_stride)
{
    int i;
    int r, c;
    int a1, b1, c1, d1;
    short output[16];
    short *ip = input;
    short *op = output;
    int temp1, temp2;
    int shortpitch = 4;

    for (i = 0; i < 4; i++)
    {
        a1 = ip[0] + ip[8];
        b1 = ip[0] - ip[8];

        temp1 = (ip[4] * sinpi8sqrt2) >> 16;
        temp2 = ip[12] + ((ip[12] * cospi8sqrt2minus1) >> 16);
        c1 = temp1 - temp2;

        temp1 = ip[4] + ((ip[4] * cospi8sqrt2minus1) >> 16);
        temp2 = (ip[12] * sinpi8sqrt2) >> 16;
        d1 = temp1 + temp2;

        op[shortpitch*0] = a1 + d1;
        op[shortpitch*3] = a1 - d1;

        op[shortpitch*1] = b1 + c1;
        op[shortpitch*2] = b1 - c1;

        ip++;
        op++;
    }

    ip = output;
    op = output;

    for (i = 0; i < 4; i++)
    {
        a1 = ip[0] + ip[2];
        b1 = ip[0] - ip[2];

        temp1 = (ip[1] * sinpi8sqrt2) >> 16;
        temp2 = ip[3] + ((ip[3] * cospi8sqrt2minus1) >> 16);
        c1 = temp1 - temp2;

        temp1 = ip[1] + ((ip[1] * cospi8sqrt2minus1) >> 16);
        temp2 = (ip[3] * sinpi8sqrt2) >> 16;
        d1 = temp1 + temp2;


        op[0] = (a1 + d1 + 4) >> 3;
        op[3] = (a1 - d1 + 4) >> 3;

        op[1] = (b1 + c1 + 4) >> 3;
        op[2] = (b1 - c1 + 4) >> 3;

        ip += shortpitch;
        op += shortpitch;
    }

    ip = output;
    for (r = 0; r < 4; r++)
    {
        for (c = 0; c < 4; c++)
        {
            int a = ip[c] + pred_ptr[c] ;

            if (a < 0)
                a = 0;

            if (a > 255)
                a = 255;

            dst_ptr[c] = (unsigned char) a ;
        }
        ip += 4;
        dst_ptr += dst_stride;
        pred_ptr += pred_stride;
    }
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
