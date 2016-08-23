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
#include "vp8/encoder/block.h"
#include "vp8/common/blockd.h"

void vp8_subtract_b_mmi(BLOCK *be, BLOCKD *bd, int pitch)
{
#if 1
    unsigned char *src_ptr = (*(be->base_src) + be->src);
    short *diff_ptr = be->src_diff;
    unsigned char *pred_ptr = bd->predictor;
    int src_stride = be->src_stride;
    double ftmp[12];
#if _MIPS_SIM == _ABIO32
    uint32_t tmp[1];
#endif

    __asm__ volatile (
        "xor        %[ftmp0],   %[ftmp0],           %[ftmp0]        \n\t"
#if _MIPS_SIM == _ABIO32
        "ulw        %[tmp0],    0x00(%[src_prt])                    \n\t"
        "mtc1       %[tmp0],    %[ftmp1]                            \n\t"
        "ulw        %[tmp0],    0x00(%[pred_prt])                   \n\t"
        "mtc1       %[tmp0],    %[ftmp2]                            \n\t"
#else
        "gslwlc1    %[ftmp1],   0x03(%[src_ptr])                    \n\t"
        "gslwrc1    %[ftmp1],   0x00(%[src_ptr])                    \n\t"
        "gslwlc1    %[ftmp2],   0x03(%[pred_ptr])                   \n\t"
        "gslwrc1    %[ftmp2],   0x00(%[pred_ptr])                   \n\t"
#endif
        PTR_ADDU   "%[src_ptr], %[src_ptr],         %[src_stride]   \n\t"
        PTR_ADDU   "%[pred_ptr],%[pred_ptr],        %[pitch]        \n\t"
#if _MIPS_SIM == _ABIO32
        "ulw        %[tmp0],    0x00(%[src_prt])                    \n\t"
        "mtc1       %[tmp0],    %[ftmp3]                            \n\t"
        "ulw        %[tmp0],    0x00(%[pred_prt])                   \n\t"
        "mtc1       %[tmp0],    %[ftmp4]                            \n\t"
#else
        "gslwlc1    %[ftmp3],   0x03(%[src_ptr])                    \n\t"
        "gslwrc1    %[ftmp3],   0x00(%[src_ptr])                    \n\t"
        "gslwlc1    %[ftmp4],   0x03(%[pred_ptr])                   \n\t"
        "gslwrc1    %[ftmp4],   0x00(%[pred_ptr])                   \n\t"
#endif
        PTR_ADDU   "%[src_ptr], %[src_ptr],         %[src_stride]   \n\t"
        PTR_ADDU   "%[pred_ptr],%[pred_ptr],        %[pitch]        \n\t"
#if _MIPS_SIM == _ABIO32
        "ulw        %[tmp0],    0x00(%[src_prt])                    \n\t"
        "mtc1       %[tmp0],    %[ftmp5]                            \n\t"
        "ulw        %[tmp0],    0x00(%[pred_prt])                   \n\t"
        "mtc1       %[tmp0],    %[ftmp6]                            \n\t"
#else
        "gslwlc1    %[ftmp5],   0x03(%[src_ptr])                    \n\t"
        "gslwrc1    %[ftmp5],   0x00(%[src_ptr])                    \n\t"
        "gslwlc1    %[ftmp6],   0x03(%[pred_ptr])                   \n\t"
        "gslwrc1    %[ftmp6],   0x00(%[pred_ptr])                   \n\t"
#endif
        PTR_ADDU   "%[src_ptr], %[src_ptr],         %[src_stride]   \n\t"
        PTR_ADDU   "%[pred_ptr],%[pred_ptr],        %[pitch]        \n\t"
#if _MIPS_SIM == _ABIO32
        "ulw        %[tmp0],    0x00(%[src_prt])                    \n\t"
        "mtc1       %[tmp0],    %[ftmp7]                            \n\t"
        "ulw        %[tmp0],    0x00(%[pred_prt])                   \n\t"
        "mtc1       %[tmp0],    %[ftmp8]                            \n\t"
#else
        "gslwlc1    %[ftmp7],   0x03(%[src_ptr])                    \n\t"
        "gslwrc1    %[ftmp7],   0x00(%[src_ptr])                    \n\t"
        "gslwlc1    %[ftmp8],   0x03(%[pred_ptr])                   \n\t"
        "gslwrc1    %[ftmp8],   0x00(%[pred_ptr])                   \n\t"
#endif

        "punpcklbh  %[ftmp9],   %[ftmp1],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp10],  %[ftmp2],           %[ftmp0]        \n\t"
        "psubh      %[ftmp11],  %[ftmp9],           %[ftmp10]       \n\t"
        "gssdlc1    %[ftmp11],  0x07(%[diff_ptr])                   \n\t"
        "gssdrc1    %[ftmp11],  0x00(%[diff_ptr])                   \n\t"
        PTR_ADDU   "%[diff_ptr],%[diff_ptr],        %[pitch_x2]     \n\t"
        "punpcklbh  %[ftmp9],   %[ftmp3],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp10],  %[ftmp4],           %[ftmp0]        \n\t"
        "psubh      %[ftmp11],  %[ftmp9],           %[ftmp10]       \n\t"
        "gssdlc1    %[ftmp11],  0x07(%[diff_ptr])                   \n\t"
        "gssdrc1    %[ftmp11],  0x00(%[diff_ptr])                   \n\t"
        PTR_ADDU   "%[diff_ptr],%[diff_ptr],        %[pitch_x2]     \n\t"
        "punpcklbh  %[ftmp9],   %[ftmp5],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp10],  %[ftmp6],           %[ftmp0]        \n\t"
        "psubh      %[ftmp11],  %[ftmp9],           %[ftmp10]       \n\t"
        "gssdlc1    %[ftmp11],  0x07(%[diff_ptr])                   \n\t"
        "gssdrc1    %[ftmp11],  0x00(%[diff_ptr])                   \n\t"
        PTR_ADDU   "%[diff_ptr],%[diff_ptr],        %[pitch_x2]     \n\t"
        "punpcklbh  %[ftmp9],   %[ftmp7],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp10],  %[ftmp8],           %[ftmp0]        \n\t"
        "psubh      %[ftmp11],  %[ftmp9],           %[ftmp10]       \n\t"
        "gssdlc1    %[ftmp11],  0x07(%[diff_ptr])                   \n\t"
        "gssdrc1    %[ftmp11],  0x00(%[diff_ptr])                   \n\t"
        : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
          [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
          [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
          [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
          [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
          [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
#if _MIPS_SIM == _ABIO32
          [tmp0]"=&r"(tmp[0]),
#endif
          [src_ptr]"+&r"(src_ptr),          [pred_ptr]"+&r"(pred_ptr),
          [diff_ptr]"+&r"(diff_ptr)
        : [src_stride]"r"((mips_reg)src_stride),
          [pitch]"r"((mips_reg)pitch),      [pitch_x2]"r"((mips_reg)(pitch*2))
        : "memory"
    );
#else
    unsigned char *src_ptr = (*(be->base_src) + be->src);
    short *diff_ptr = be->src_diff;
    unsigned char *pred_ptr = bd->predictor;
    int src_stride = be->src_stride;
    int r, c;

    for (r = 0; r < 4; r++)
    {
        for (c = 0; c < 4; c++)
        {
            diff_ptr[c] = src_ptr[c] - pred_ptr[c];
        }

        diff_ptr += pitch;
        pred_ptr += pitch;
        src_ptr  += src_stride;
    }
#endif
}

void vp8_subtract_mbuv_mmi(short *diff, unsigned char *usrc, unsigned char *vsrc,
                           int src_stride, unsigned char *upred,
                           unsigned char *vpred, int pred_stride)
{
    short *udiff = diff + 256;
    short *vdiff = diff + 320;

    int r, c;

    for (r = 0; r < 8; r++)
    {
        for (c = 0; c < 8; c++)
        {
            udiff[c] = usrc[c] - upred[c];
        }

        udiff += 8;
        upred += pred_stride;
        usrc  += src_stride;
    }

    for (r = 0; r < 8; r++)
    {
        for (c = 0; c < 8; c++)
        {
            vdiff[c] = vsrc[c] - vpred[c];
        }

        vdiff += 8;
        vpred += pred_stride;
        vsrc  += src_stride;
    }
}

void vp8_subtract_mby_mmi(short *diff, unsigned char *src, int src_stride,
                          unsigned char *pred, int pred_stride)
{
    int r, c;

    for (r = 0; r < 16; r++)
    {
        for (c = 0; c < 16; c++)
        {
            diff[c] = src[c] - pred[c];
        }

        diff += 16;
        pred += pred_stride;
        src  += src_stride;
    }
}

