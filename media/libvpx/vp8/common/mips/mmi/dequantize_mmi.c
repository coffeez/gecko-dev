/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vpx_config.h"
#include "vp8_rtcd.h"
#include "vp8/common/blockd.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/asmdefs_mmi.h"

void vp8_dequantize_b_mmi(BLOCKD *d, short *DQC)
{
#if 1
    double ftmp[8];

    __asm__ volatile (
        "gsldlc1    %[ftmp0],   0x07(%[qcoeff])                 \n\t"
        "gsldrc1    %[ftmp0],   0x00(%[qcoeff])                 \n\t"
        "gsldlc1    %[ftmp1],   0x0f(%[qcoeff])                 \n\t"
        "gsldrc1    %[ftmp1],   0x08(%[qcoeff])                 \n\t"
        "gsldlc1    %[ftmp2],   0x17(%[qcoeff])                 \n\t"
        "gsldrc1    %[ftmp2],   0x10(%[qcoeff])                 \n\t"
        "gsldlc1    %[ftmp3],   0x1f(%[qcoeff])                 \n\t"
        "gsldrc1    %[ftmp3],   0x18(%[qcoeff])                 \n\t"

        "gsldlc1    %[ftmp4],   0x07(%[DQC])                    \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[DQC])                    \n\t"
        "gsldlc1    %[ftmp5],   0x0f(%[DQC])                    \n\t"
        "gsldrc1    %[ftmp5],   0x08(%[DQC])                    \n\t"
        "gsldlc1    %[ftmp6],   0x17(%[DQC])                    \n\t"
        "gsldrc1    %[ftmp6],   0x10(%[DQC])                    \n\t"
        "gsldlc1    %[ftmp7],   0x1f(%[DQC])                    \n\t"
        "gsldrc1    %[ftmp7],   0x18(%[DQC])                    \n\t"

        "pmullh     %[ftmp0],   %[ftmp0],       %[ftmp4]        \n\t"
        "pmullh     %[ftmp1],   %[ftmp1],       %[ftmp5]        \n\t"
        "pmullh     %[ftmp2],   %[ftmp2],       %[ftmp6]        \n\t"
        "pmullh     %[ftmp3],   %[ftmp3],       %[ftmp7]        \n\t"

        "gssdlc1    %[ftmp0],   0x07(%[dqcoeff])                \n\t"
        "gssdrc1    %[ftmp0],   0x00(%[dqcoeff])                \n\t"
        "gssdlc1    %[ftmp1],   0x0f(%[dqcoeff])                \n\t"
        "gssdrc1    %[ftmp1],   0x08(%[dqcoeff])                \n\t"
        "gssdlc1    %[ftmp2],   0x17(%[dqcoeff])                \n\t"
        "gssdrc1    %[ftmp2],   0x10(%[dqcoeff])                \n\t"
        "gssdlc1    %[ftmp3],   0x1f(%[dqcoeff])                \n\t"
        "gssdrc1    %[ftmp3],   0x18(%[dqcoeff])                \n\t"
        : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
          [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
          [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
          [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7])
        : [dqcoeff]"r"(d->dqcoeff),         [qcoeff]"r"(d->qcoeff),
          [DQC]"r"(DQC)
        : "memory"
    );
#else
    int i;
    short *DQ  = d->dqcoeff;
    short *Q   = d->qcoeff;

    for (i = 0; i < 16; i++)
    {
        DQ[i] = Q[i] * DQC[i];
    }
#endif
}

void vp8_dequant_idct_add_mmi(short *input, short *dq,
                              unsigned char *dest, int stride)
{
#if 1
    double ftmp[8];

    __asm__ volatile (
        "gsldlc1    %[ftmp0],   0x07(%[dq])                     \n\t"
        "gsldrc1    %[ftmp0],   0x00(%[dq])                     \n\t"
        "gsldlc1    %[ftmp1],   0x0f(%[dq])                     \n\t"
        "gsldrc1    %[ftmp1],   0x08(%[dq])                     \n\t"
        "gsldlc1    %[ftmp2],   0x17(%[dq])                     \n\t"
        "gsldrc1    %[ftmp2],   0x10(%[dq])                     \n\t"
        "gsldlc1    %[ftmp3],   0x1f(%[dq])                     \n\t"
        "gsldrc1    %[ftmp3],   0x18(%[dq])                     \n\t"

        "gsldlc1    %[ftmp4],   0x07(%[input])                  \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[input])                  \n\t"
        "gsldlc1    %[ftmp5],   0x0f(%[input])                  \n\t"
        "gsldrc1    %[ftmp5],   0x08(%[input])                  \n\t"
        "gsldlc1    %[ftmp6],   0x17(%[input])                  \n\t"
        "gsldrc1    %[ftmp6],   0x10(%[input])                  \n\t"
        "gsldlc1    %[ftmp7],   0x1f(%[input])                  \n\t"
        "gsldrc1    %[ftmp7],   0x18(%[input])                  \n\t"

        "pmullh     %[ftmp0],   %[ftmp0],       %[ftmp4]        \n\t"
        "pmullh     %[ftmp1],   %[ftmp1],       %[ftmp5]        \n\t"
        "pmullh     %[ftmp2],   %[ftmp2],       %[ftmp6]        \n\t"
        "pmullh     %[ftmp3],   %[ftmp3],       %[ftmp7]        \n\t"

        "gssdlc1    %[ftmp0],   0x07(%[input])                  \n\t"
        "gssdrc1    %[ftmp0],   0x00(%[input])                  \n\t"
        "gssdlc1    %[ftmp1],   0x0f(%[input])                  \n\t"
        "gssdrc1    %[ftmp1],   0x08(%[input])                  \n\t"
        "gssdlc1    %[ftmp2],   0x17(%[input])                  \n\t"
        "gssdrc1    %[ftmp2],   0x10(%[input])                  \n\t"
        "gssdlc1    %[ftmp3],   0x1f(%[input])                  \n\t"
        "gssdrc1    %[ftmp3],   0x18(%[input])                  \n\t"
        : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
          [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
          [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
          [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
          [input]"+&r"(input)
        : [dq]"r"(dq)
        : "memory"
    );

    vp8_short_idct4x4llm_mmi(input, dest, stride, dest, stride);

    __asm__ volatile (
        "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]        \n\t"
        "gssdlc1    %[ftmp0],   0x07(%[input])                  \n\t"
        "gssdrc1    %[ftmp0],   0x00(%[input])                  \n\t"
        "sdl        $0,         0x0f(%[input])                  \n\t"
        "sdr        $0,         0x08(%[input])                  \n\t"
        "gssdlc1    %[ftmp0],   0x17(%[input])                  \n\t"
        "gssdrc1    %[ftmp0],   0x10(%[input])                  \n\t"
        "sdl        $0,         0x1f(%[input])                  \n\t"
        "sdr        $0,         0x18(%[input])                  \n\t"
        : [ftmp0]"=&f"(ftmp[0]),            [input]"+&r"(input)
        ::"memory"
    );
#else
    int i;

    for (i = 0; i < 16; i++)
    {
        input[i] = dq[i] * input[i];
    }

    vp8_short_idct4x4llm_mmi(input, dest, stride, dest, stride);

    memset(input, 0, 32);
#endif
}
