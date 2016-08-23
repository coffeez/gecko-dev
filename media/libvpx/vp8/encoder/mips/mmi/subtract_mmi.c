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
#if 1
    short *udiff = diff + 256;
    short *vdiff = diff + 320;
    double ftmp[13];
    uint32_t tmp[1];

    __asm__ volatile (
        "xor        %[ftmp0],   %[ftmp0],           %[ftmp0]        \n\t"

        // u
        "li         %[tmp0],    0x02                                \n\t"
        "1:                                                         \n\t"
        "gsldlc1    %[ftmp1],   0x07(%[usrc])                       \n\t"
        "gsldrc1    %[ftmp1],   0x00(%[usrc])                       \n\t"
        "gsldlc1    %[ftmp2],   0x07(%[upred])                      \n\t"
        "gsldrc1    %[ftmp2],   0x00(%[upred])                      \n\t"
        PTR_ADDU   "%[usrc],    %[usrc],            %[src_stride]   \n\t"
        PTR_ADDU   "%[upred],   %[upred],           %[pred_stride]  \n\t"
        "gsldlc1    %[ftmp3],   0x07(%[usrc])                       \n\t"
        "gsldrc1    %[ftmp3],   0x00(%[usrc])                       \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[upred])                      \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[upred])                      \n\t"
        PTR_ADDU   "%[usrc],    %[usrc],            %[src_stride]   \n\t"
        PTR_ADDU   "%[upred],   %[upred],           %[pred_stride]  \n\t"
        "gsldlc1    %[ftmp5],   0x07(%[usrc])                       \n\t"
        "gsldrc1    %[ftmp5],   0x00(%[usrc])                       \n\t"
        "gsldlc1    %[ftmp6],   0x07(%[upred])                      \n\t"
        "gsldrc1    %[ftmp6],   0x00(%[upred])                      \n\t"
        PTR_ADDU   "%[usrc],    %[usrc],            %[src_stride]   \n\t"
        PTR_ADDU   "%[upred],   %[upred],           %[pred_stride]  \n\t"
        "gsldlc1    %[ftmp7],   0x07(%[usrc])                       \n\t"
        "gsldrc1    %[ftmp7],   0x00(%[usrc])                       \n\t"
        "gsldlc1    %[ftmp8],   0x07(%[upred])                      \n\t"
        "gsldrc1    %[ftmp8],   0x00(%[upred])                      \n\t"
        PTR_ADDU   "%[usrc],    %[usrc],            %[src_stride]   \n\t"
        PTR_ADDU   "%[upred],   %[upred],           %[pred_stride]  \n\t"

        "punpcklbh  %[ftmp9],   %[ftmp1],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp10],  %[ftmp1],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp11],  %[ftmp2],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp12],  %[ftmp2],           %[ftmp0]        \n\t"
        "psubh      %[ftmp9],   %[ftmp9],           %[ftmp11]       \n\t"
        "psubh      %[ftmp10],  %[ftmp10],          %[ftmp12]       \n\t"
        "gssdlc1    %[ftmp9],   0x07(%[udiff])                      \n\t"
        "gssdrc1    %[ftmp9],   0x00(%[udiff])                      \n\t"
        "gssdlc1    %[ftmp10],  0x0f(%[udiff])                      \n\t"
        "gssdrc1    %[ftmp10],  0x08(%[udiff])                      \n\t"

        "punpcklbh  %[ftmp9],   %[ftmp3],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp10],  %[ftmp3],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp11],  %[ftmp4],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp12],  %[ftmp4],           %[ftmp0]        \n\t"
        "psubh      %[ftmp9],   %[ftmp9],           %[ftmp11]       \n\t"
        "psubh      %[ftmp10],  %[ftmp10],          %[ftmp12]       \n\t"
        "gssdlc1    %[ftmp9],   0x17(%[udiff])                      \n\t"
        "gssdrc1    %[ftmp9],   0x10(%[udiff])                      \n\t"
        "gssdlc1    %[ftmp10],  0x1f(%[udiff])                      \n\t"
        "gssdrc1    %[ftmp10],  0x18(%[udiff])                      \n\t"

        "punpcklbh  %[ftmp9],   %[ftmp5],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp10],  %[ftmp5],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp11],  %[ftmp6],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp12],  %[ftmp6],           %[ftmp0]        \n\t"
        "psubh      %[ftmp9],   %[ftmp9],           %[ftmp11]       \n\t"
        "psubh      %[ftmp10],  %[ftmp10],          %[ftmp12]       \n\t"
        "gssdlc1    %[ftmp9],   0x27(%[udiff])                      \n\t"
        "gssdrc1    %[ftmp9],   0x20(%[udiff])                      \n\t"
        "gssdlc1    %[ftmp10],  0x2f(%[udiff])                      \n\t"
        "gssdrc1    %[ftmp10],  0x28(%[udiff])                      \n\t"

        "punpcklbh  %[ftmp9],   %[ftmp7],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp10],  %[ftmp7],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp11],  %[ftmp8],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp12],  %[ftmp8],           %[ftmp0]        \n\t"
        "psubh      %[ftmp9],   %[ftmp9],           %[ftmp11]       \n\t"
        "psubh      %[ftmp10],  %[ftmp10],          %[ftmp12]       \n\t"
        "gssdlc1    %[ftmp9],   0x37(%[udiff])                      \n\t"
        "gssdrc1    %[ftmp9],   0x30(%[udiff])                      \n\t"
        "gssdlc1    %[ftmp10],  0x3f(%[udiff])                      \n\t"
        "gssdrc1    %[ftmp10],  0x38(%[udiff])                      \n\t"

        "addiu      %[tmp0],    %[tmp0],            -0x01           \n\t"
        PTR_ADDIU  "%[udiff],   %[udiff],           0x40            \n\t"
        "bnez       %[tmp0],    1b                                  \n\t"

        // v
        "li         %[tmp0],    0x02                                \n\t"
        "2:                                                         \n\t"
        "gsldlc1    %[ftmp1],   0x07(%[vsrc])                       \n\t"
        "gsldrc1    %[ftmp1],   0x00(%[vsrc])                       \n\t"
        "gsldlc1    %[ftmp2],   0x07(%[vpred])                      \n\t"
        "gsldrc1    %[ftmp2],   0x00(%[vpred])                      \n\t"
        PTR_ADDU   "%[vsrc],    %[vsrc],            %[src_stride]   \n\t"
        PTR_ADDU   "%[vpred],   %[vpred],           %[pred_stride]  \n\t"
        "gsldlc1    %[ftmp3],   0x07(%[vsrc])                       \n\t"
        "gsldrc1    %[ftmp3],   0x00(%[vsrc])                       \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[vpred])                      \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[vpred])                      \n\t"
        PTR_ADDU   "%[vsrc],    %[vsrc],            %[src_stride]   \n\t"
        PTR_ADDU   "%[vpred],   %[vpred],           %[pred_stride]  \n\t"
        "gsldlc1    %[ftmp5],   0x07(%[vsrc])                       \n\t"
        "gsldrc1    %[ftmp5],   0x00(%[vsrc])                       \n\t"
        "gsldlc1    %[ftmp6],   0x07(%[vpred])                      \n\t"
        "gsldrc1    %[ftmp6],   0x00(%[vpred])                      \n\t"
        PTR_ADDU   "%[vsrc],    %[vsrc],            %[src_stride]   \n\t"
        PTR_ADDU   "%[vpred],   %[vpred],           %[pred_stride]  \n\t"
        "gsldlc1    %[ftmp7],   0x07(%[vsrc])                       \n\t"
        "gsldrc1    %[ftmp7],   0x00(%[vsrc])                       \n\t"
        "gsldlc1    %[ftmp8],   0x07(%[vpred])                      \n\t"
        "gsldrc1    %[ftmp8],   0x00(%[vpred])                      \n\t"
        PTR_ADDU   "%[vsrc],    %[vsrc],            %[src_stride]   \n\t"
        PTR_ADDU   "%[vpred],   %[vpred],           %[pred_stride]  \n\t"

        "punpcklbh  %[ftmp9],   %[ftmp1],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp10],  %[ftmp1],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp11],  %[ftmp2],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp12],  %[ftmp2],           %[ftmp0]        \n\t"
        "psubh      %[ftmp9],   %[ftmp9],           %[ftmp11]       \n\t"
        "psubh      %[ftmp10],  %[ftmp10],          %[ftmp12]       \n\t"
        "gssdlc1    %[ftmp9],   0x07(%[vdiff])                      \n\t"
        "gssdrc1    %[ftmp9],   0x00(%[vdiff])                      \n\t"
        "gssdlc1    %[ftmp10],  0x0f(%[vdiff])                      \n\t"
        "gssdrc1    %[ftmp10],  0x08(%[vdiff])                      \n\t"

        "punpcklbh  %[ftmp9],   %[ftmp3],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp10],  %[ftmp3],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp11],  %[ftmp4],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp12],  %[ftmp4],           %[ftmp0]        \n\t"
        "psubh      %[ftmp9],   %[ftmp9],           %[ftmp11]       \n\t"
        "psubh      %[ftmp10],  %[ftmp10],          %[ftmp12]       \n\t"
        "gssdlc1    %[ftmp9],   0x17(%[vdiff])                      \n\t"
        "gssdrc1    %[ftmp9],   0x10(%[vdiff])                      \n\t"
        "gssdlc1    %[ftmp10],  0x1f(%[vdiff])                      \n\t"
        "gssdrc1    %[ftmp10],  0x18(%[vdiff])                      \n\t"

        "punpcklbh  %[ftmp9],   %[ftmp5],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp10],  %[ftmp5],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp11],  %[ftmp6],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp12],  %[ftmp6],           %[ftmp0]        \n\t"
        "psubh      %[ftmp9],   %[ftmp9],           %[ftmp11]       \n\t"
        "psubh      %[ftmp10],  %[ftmp10],          %[ftmp12]       \n\t"
        "gssdlc1    %[ftmp9],   0x27(%[vdiff])                      \n\t"
        "gssdrc1    %[ftmp9],   0x20(%[vdiff])                      \n\t"
        "gssdlc1    %[ftmp10],  0x2f(%[vdiff])                      \n\t"
        "gssdrc1    %[ftmp10],  0x28(%[vdiff])                      \n\t"

        "punpcklbh  %[ftmp9],   %[ftmp7],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp10],  %[ftmp7],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp11],  %[ftmp8],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp12],  %[ftmp8],           %[ftmp0]        \n\t"
        "psubh      %[ftmp9],   %[ftmp9],           %[ftmp11]       \n\t"
        "psubh      %[ftmp10],  %[ftmp10],          %[ftmp12]       \n\t"
        "gssdlc1    %[ftmp9],   0x37(%[vdiff])                      \n\t"
        "gssdrc1    %[ftmp9],   0x30(%[vdiff])                      \n\t"
        "gssdlc1    %[ftmp10],  0x3f(%[vdiff])                      \n\t"
        "gssdrc1    %[ftmp10],  0x38(%[vdiff])                      \n\t"

        "addiu      %[tmp0],    %[tmp0],            -0x01           \n\t"
        PTR_ADDIU  "%[vdiff],   %[vdiff],           0x40            \n\t"
        "bnez       %[tmp0],    2b                                  \n\t"
        : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
          [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
          [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
          [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
          [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
          [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
          [ftmp12]"=&f"(ftmp[12]),
          [tmp0]"=&r"(tmp[0]),
          [usrc]"+&r"(usrc),                [upred]"+&r"(upred),
          [vsrc]"+&r"(vsrc),                [vpred]"+&r"(vpred),
          [udiff]"+&r"(udiff),              [vdiff]"+&r"(vdiff)
        : [pred_stride]"r"((mips_reg)pred_stride),
          [src_stride]"r"((mips_reg)src_stride)
        : "memory"
    );
#else
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
#endif
}

void vp8_subtract_mby_mmi(short *diff, unsigned char *src, int src_stride,
                          unsigned char *pred, int pred_stride)
{
#if 1
    double ftmp[13];
    uint32_t tmp[1];

    __asm__ volatile (
        "xor        %[ftmp0],   %[ftmp0],           %[ftmp0]        \n\t"
        "li         %[tmp0],    0x08                                \n\t"

        "1:                                                         \n\t"
        "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t"
        "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t"
        "gsldlc1    %[ftmp2],   0x07(%[pred])                       \n\t"
        "gsldrc1    %[ftmp2],   0x00(%[pred])                       \n\t"
        "gsldlc1    %[ftmp3],   0x0f(%[src])                        \n\t"
        "gsldrc1    %[ftmp3],   0x08(%[src])                        \n\t"
        "gsldlc1    %[ftmp4],   0x0f(%[pred])                       \n\t"
        "gsldrc1    %[ftmp4],   0x08(%[pred])                       \n\t"
        PTR_ADDU   "%[src],     %[src],             %[src_stride]   \n\t"
        PTR_ADDU   "%[pred],    %[pred],            %[pred_stride]  \n\t"
        "gsldlc1    %[ftmp5],   0x07(%[src])                        \n\t"
        "gsldrc1    %[ftmp5],   0x00(%[src])                        \n\t"
        "gsldlc1    %[ftmp6],   0x07(%[pred])                       \n\t"
        "gsldrc1    %[ftmp6],   0x00(%[pred])                       \n\t"
        "gsldlc1    %[ftmp7],   0x0f(%[src])                        \n\t"
        "gsldrc1    %[ftmp7],   0x08(%[src])                        \n\t"
        "gsldlc1    %[ftmp8],   0x0f(%[pred])                       \n\t"
        "gsldrc1    %[ftmp8],   0x08(%[pred])                       \n\t"
        PTR_ADDU   "%[src],     %[src],             %[src_stride]   \n\t"
        PTR_ADDU   "%[pred],    %[pred],            %[pred_stride]  \n\t"

        "punpcklbh  %[ftmp9],   %[ftmp1],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp10],  %[ftmp1],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp11],  %[ftmp2],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp12],  %[ftmp2],           %[ftmp0]        \n\t"
        "psubh      %[ftmp9],   %[ftmp9],           %[ftmp11]       \n\t"
        "psubh      %[ftmp10],  %[ftmp10],          %[ftmp12]       \n\t"
        "gssdlc1    %[ftmp9],   0x07(%[diff])                       \n\t"
        "gssdrc1    %[ftmp9],   0x00(%[diff])                       \n\t"
        "gssdlc1    %[ftmp10],  0x0f(%[diff])                       \n\t"
        "gssdrc1    %[ftmp10],  0x08(%[diff])                       \n\t"
        "punpcklbh  %[ftmp9],   %[ftmp3],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp10],  %[ftmp3],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp11],  %[ftmp4],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp12],  %[ftmp4],           %[ftmp0]        \n\t"
        "psubh      %[ftmp9],   %[ftmp9],           %[ftmp11]       \n\t"
        "psubh      %[ftmp10],  %[ftmp10],          %[ftmp12]       \n\t"
        "gssdlc1    %[ftmp9],   0x17(%[diff])                       \n\t"
        "gssdrc1    %[ftmp9],   0x10(%[diff])                       \n\t"
        "gssdlc1    %[ftmp10],  0x1f(%[diff])                       \n\t"
        "gssdrc1    %[ftmp10],  0x18(%[diff])                       \n\t"

        "punpcklbh  %[ftmp9],   %[ftmp5],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp10],  %[ftmp5],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp11],  %[ftmp6],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp12],  %[ftmp6],           %[ftmp0]        \n\t"
        "psubh      %[ftmp9],   %[ftmp9],           %[ftmp11]       \n\t"
        "psubh      %[ftmp10],  %[ftmp10],          %[ftmp12]       \n\t"
        "gssdlc1    %[ftmp9],   0x27(%[diff])                       \n\t"
        "gssdrc1    %[ftmp9],   0x20(%[diff])                       \n\t"
        "gssdlc1    %[ftmp10],  0x2f(%[diff])                       \n\t"
        "gssdrc1    %[ftmp10],  0x28(%[diff])                       \n\t"
        "punpcklbh  %[ftmp9],   %[ftmp7],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp10],  %[ftmp7],           %[ftmp0]        \n\t"
        "punpcklbh  %[ftmp11],  %[ftmp8],           %[ftmp0]        \n\t"
        "punpckhbh  %[ftmp12],  %[ftmp8],           %[ftmp0]        \n\t"
        "psubh      %[ftmp9],   %[ftmp9],           %[ftmp11]       \n\t"
        "psubh      %[ftmp10],  %[ftmp10],          %[ftmp12]       \n\t"
        "gssdlc1    %[ftmp9],   0x37(%[diff])                       \n\t"
        "gssdrc1    %[ftmp9],   0x30(%[diff])                       \n\t"
        "gssdlc1    %[ftmp10],  0x3f(%[diff])                       \n\t"
        "gssdrc1    %[ftmp10],  0x38(%[diff])                       \n\t"

        "addiu      %[tmp0],    %[tmp0],            -0x01           \n\t"
        PTR_ADDIU  "%[diff],    %[diff],            0x40            \n\t"
        "bnez       %[tmp0],    1b                                  \n\t"
        : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
          [ftmp2]"=&f"(ftmp[2]),            [ftmp3]"=&f"(ftmp[3]),
          [ftmp4]"=&f"(ftmp[4]),            [ftmp5]"=&f"(ftmp[5]),
          [ftmp6]"=&f"(ftmp[6]),            [ftmp7]"=&f"(ftmp[7]),
          [ftmp8]"=&f"(ftmp[8]),            [ftmp9]"=&f"(ftmp[9]),
          [ftmp10]"=&f"(ftmp[10]),          [ftmp11]"=&f"(ftmp[11]),
          [ftmp12]"=&f"(ftmp[12]),
          [tmp0]"=&r"(tmp[0]),
          [src]"+&r"(src),                  [pred]"+&r"(pred),
          [diff]"+&r"(diff)
        : [pred_stride]"r"((mips_reg)pred_stride),
          [src_stride]"r"((mips_reg)src_stride)
        : "memory"
    );
#else
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
#endif
}

