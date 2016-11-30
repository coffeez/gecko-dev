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

void vp8_copy_mem16x16_mmi(
    unsigned char *src,
    int src_stride,
    unsigned char *dst,
    int dst_stride)
{
#if 1
    double ftmp[2];
    uint64_t tmp[3];

    __asm__ volatile (
        "li         %[tmp2],    0x04                            \n\t"
        "1:                                                     \n\t"
        "gsldlc1    %[ftmp0],   0x07(%[src])                    \n\t"
        "gsldrc1    %[ftmp0],   0x00(%[src])                    \n\t"
        "ldl        %[tmp0],    0x0f(%[src])                    \n\t"
        "ldr        %[tmp0],    0x08(%[src])                    \n\t"
        PTR_ADDU   "%[src],     %[src],         %[src_stride]   \n\t"
        "gssdlc1    %[ftmp0],   0x07(%[dst])                    \n\t"
        "gssdrc1    %[ftmp0],   0x00(%[dst])                    \n\t"
        "sdl        %[tmp0],    0x0f(%[dst])                    \n\t"
        "sdr        %[tmp0],    0x08(%[dst])                    \n\t"
        PTR_ADDU   "%[dst],      %[dst],        %[dst_stride]   \n\t"
        "gsldlc1    %[ftmp1],   0x07(%[src])                    \n\t"
        "gsldrc1    %[ftmp1],   0x00(%[src])                    \n\t"
        "ldl        %[tmp1],    0x0f(%[src])                    \n\t"
        "ldr        %[tmp1],    0x08(%[src])                    \n\t"
        PTR_ADDU   "%[src],     %[src],         %[src_stride]   \n\t"
        "gssdlc1    %[ftmp1],   0x07(%[dst])                    \n\t"
        "gssdrc1    %[ftmp1],   0x00(%[dst])                    \n\t"
        "sdl        %[tmp1],    0x0f(%[dst])                    \n\t"
        "sdr        %[tmp1],    0x08(%[dst])                    \n\t"
        PTR_ADDU   "%[dst],     %[dst],         %[dst_stride]   \n\t"

        "gsldlc1    %[ftmp0],   0x07(%[src])                    \n\t"
        "gsldrc1    %[ftmp0],   0x00(%[src])                    \n\t"
        "ldl        %[tmp0],    0x0f(%[src])                    \n\t"
        "ldr        %[tmp0],    0x08(%[src])                    \n\t"
        PTR_ADDU   "%[src],     %[src],         %[src_stride]   \n\t"
        "gssdlc1    %[ftmp0],   0x07(%[dst])                    \n\t"
        "gssdrc1    %[ftmp0],   0x00(%[dst])                    \n\t"
        "sdl        %[tmp0],    0x0f(%[dst])                    \n\t"
        "sdr        %[tmp0],    0x08(%[dst])                    \n\t"
        PTR_ADDU   "%[dst],      %[dst],        %[dst_stride]   \n\t"
        "gsldlc1    %[ftmp1],   0x07(%[src])                    \n\t"
        "gsldrc1    %[ftmp1],   0x00(%[src])                    \n\t"
        "ldl        %[tmp1],    0x0f(%[src])                    \n\t"
        "ldr        %[tmp1],    0x08(%[src])                    \n\t"
        PTR_ADDU   "%[src],     %[src],         %[src_stride]   \n\t"
        "gssdlc1    %[ftmp1],   0x07(%[dst])                    \n\t"
        "gssdrc1    %[ftmp1],   0x00(%[dst])                    \n\t"
        "sdl        %[tmp1],    0x0f(%[dst])                    \n\t"
        "sdr        %[tmp1],    0x08(%[dst])                    \n\t"
        "daddi      %[tmp2],    %[tmp2],        -0x01           \n\t"
        PTR_ADDU   "%[dst],     %[dst],         %[dst_stride]   \n\t"
        "bnez       %[tmp2],    1b                              \n\t"
        : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
          [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
          [tmp2]"=&r"(tmp[2]),
          [dst]"+&r"(dst),                  [src]"+&r"(src)
        : [src_stride]"r"((mips_reg)src_stride),
          [dst_stride]"r"((mips_reg)dst_stride)
        : "memory"
    );
#else
    int r;

    for (r = 0; r < 16; r++)
    {
        memcpy(dst, src, 16);
        src += src_stride;
        dst += dst_stride;
    }
#endif
}

void vp8_copy_mem8x8_mmi(
    unsigned char *src,
    int src_stride,
    unsigned char *dst,
    int dst_stride)
{
    int r;

    for (r = 0; r < 8; r++)
    {
        memcpy(dst, src, 8);

        src += src_stride;
        dst += dst_stride;

    }

}

void vp8_copy_mem8x4_mmi(
    unsigned char *src,
    int src_stride,
    unsigned char *dst,
    int dst_stride)
{
    int r;

    for (r = 0; r < 4; r++)
    {
        memcpy(dst, src, 8);

        src += src_stride;
        dst += dst_stride;

    }

}
