/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp8/common/filter.h"
#include "vpx_ports/asmdefs_mmi.h"

DECLARE_ALIGNED(8, static const short, vp8_six_tap_mmi[8][6*8]) = {
   {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},

   {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa,
    0x007b, 0x007b, 0x007b, 0x007b, 0x007b, 0x007b, 0x007b, 0x007b,
    0x000c, 0x000c, 0x000c, 0x000c, 0x000c, 0x000c, 0x000c, 0x000c,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},

   {0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0xfff5, 0xfff5, 0xfff5, 0xfff5, 0xfff5, 0xfff5, 0xfff5, 0xfff5,
    0x006c, 0x006c, 0x006c, 0x006c, 0x006c, 0x006c, 0x006c, 0x006c,
    0x0024, 0x0024, 0x0024, 0x0024, 0x0024, 0x0024, 0x0024, 0x0024,
    0xfff8, 0xfff8, 0xfff8, 0xfff8, 0xfff8, 0xfff8, 0xfff8, 0xfff8,
    0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001},

   {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xfff7, 0xfff7, 0xfff7, 0xfff7, 0xfff7, 0xfff7, 0xfff7, 0xfff7,
    0x005d, 0x005d, 0x005d, 0x005d, 0x005d, 0x005d, 0x005d, 0x005d,
    0x0032, 0x0032, 0x0032, 0x0032, 0x0032, 0x0032, 0x0032, 0x0032,
    0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},

   {0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003,
    0xfff0, 0xfff0, 0xfff0, 0xfff0, 0xfff0, 0xfff0, 0xfff0, 0xfff0,
    0x004d, 0x004d, 0x004d, 0x004d, 0x004d, 0x004d, 0x004d, 0x004d,
    0x004d, 0x004d, 0x004d, 0x004d, 0x004d, 0x004d, 0x004d, 0x004d,
    0xfff0, 0xfff0, 0xfff0, 0xfff0, 0xfff0, 0xfff0, 0xfff0, 0xfff0,
    0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003},

   {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa,
    0x0032, 0x0032, 0x0032, 0x0032, 0x0032, 0x0032, 0x0032, 0x0032,
    0x005d, 0x005d, 0x005d, 0x005d, 0x005d, 0x005d, 0x005d, 0x005d,
    0xfff7, 0xfff7, 0xfff7, 0xfff7, 0xfff7, 0xfff7, 0xfff7, 0xfff7,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},

   {0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
    0xfff8, 0xfff8, 0xfff8, 0xfff8, 0xfff8, 0xfff8, 0xfff8, 0xfff8,
    0x0024, 0x0024, 0x0024, 0x0024, 0x0024, 0x0024, 0x0024, 0x0024,
    0x006c, 0x006c, 0x006c, 0x006c, 0x006c, 0x006c, 0x006c, 0x006c,
    0xfff5, 0xfff5, 0xfff5, 0xfff5, 0xfff5, 0xfff5, 0xfff5, 0xfff5,
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002},

   {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0x000c, 0x000c, 0x000c, 0x000c, 0x000c, 0x000c, 0x000c, 0x000c,
    0x007b, 0x007b, 0x007b, 0x007b, 0x007b, 0x007b, 0x007b, 0x007b,
    0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa, 0xfffa,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}
};

static INLINE void vp8_filter_block1d_h6_mmi
(
    unsigned char   *src_ptr,
    unsigned short  *output_ptr,
    unsigned int    src_pixels_per_line,
    unsigned int    pixel_step,
    unsigned int    output_height,
    unsigned int    output_width,
    const short     *vp8_filter
) {
    uint32_t tmp[1];
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_40) = {0x0040004000400040ULL};
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
#endif  // _MIPS_SIM == _ABIO32

    __asm__ volatile (
        "ldc1       %[ftmp9],   0x00(%[vp8_filter])                     \n\t"
        "ldc1       %[ftmp1],   0x10(%[vp8_filter])                     \n\t"
        "ldc1       %[ftmp2],   0x20(%[vp8_filter])                     \n\t"
        "ldc1       %[ftmp6],   0x30(%[vp8_filter])                     \n\t"
        "ldc1       %[ftmp7],   0x40(%[vp8_filter])                     \n\t"
        "xor        %[ftmp0],   %[ftmp0],           %[ftmp0]            \n\t"
        "ldc1       %[ftmp8],   0x50(%[vp8_filter])                     \n\t"

        "1:                                                             \n\t"
        "gsldlc1    %[ftmp3],   0x05(%[src_ptr])                        \n\t"
        "gsldrc1    %[ftmp3],  -0x02(%[src_ptr])                        \n\t"

        "li         %[tmp0],    0x08                                    \n\t"
        "mtc1       %[tmp0],    %[ftmp10]                               \n\t"
        "mov.d      %[ftmp4],   %[ftmp3]                                \n\t"
        "dsrl       %[ftmp3],   %[ftmp3],           %[ftmp10]           \n\t"
        "punpcklbh  %[ftmp3],   %[ftmp3],           %[ftmp0]            \n\t"
        "pmullh     %[ftmp3],   %[ftmp3],           %[ftmp1]            \n\t"

        "mov.d      %[ftmp5],   %[ftmp4]                                \n\t"
        "punpckhbh  %[ftmp4],   %[ftmp4],           %[ftmp0]            \n\t"
        "pmullh     %[ftmp4],   %[ftmp4],           %[ftmp7]            \n\t"
        "paddsh     %[ftmp3],   %[ftmp3],           %[ftmp4]            \n\t"

        "li         %[tmp0],    0x10                                    \n\t"
        "dmtc1      %[tmp0],    %[ftmp10]                               \n\t"
        "mov.d      %[ftmp4],   %[ftmp5]                                \n\t"
        "dsrl       %[ftmp5],   %[ftmp5],           %[ftmp10]           \n\t"
        "punpcklbh  %[ftmp5],   %[ftmp5],           %[ftmp0]            \n\t"
        "pmullh     %[ftmp5],   %[ftmp5],           %[ftmp2]            \n\t"
        "paddsh     %[ftmp3],   %[ftmp3],           %[ftmp5]            \n\t"

        "li         %[tmp0],    0x18                                    \n\t"
        "mtc1       %[tmp0],    %[ftmp10]                               \n\t"
        "mov.d      %[ftmp5],   %[ftmp4]                                \n\t"
        "dsrl       %[ftmp4],   %[ftmp4],           %[ftmp10]           \n\t"
        "punpcklbh  %[ftmp4],   %[ftmp4],           %[ftmp0]            \n\t"
        "pmullh     %[ftmp4],   %[ftmp4],           %[ftmp6]            \n\t"
        "paddsh     %[ftmp3],   %[ftmp3],           %[ftmp4]            \n\t"

#if _MIPS_SIM == _ABIO32
        "ulw        %[tmp0],    0x03(%[src_ptr])                        \n\t"
        "mtc1       %[tmp0],    %[ftmp4]                                \n\t"
#else
        "gslwlc1    %[ftmp4],   0x06(%[src_ptr])                        \n\t"
        "gslwrc1    %[ftmp4],   0x03(%[src_ptr])                        \n\t"
#endif

        "li         %[tmp0],    0x07                                    \n\t"
        "mtc1       %[tmp0],    %[ftmp10]                               \n\t"
        "punpcklbh  %[ftmp4],   %[ftmp4],           %[ftmp0]            \n\t"
        "pmullh     %[ftmp4],   %[ftmp4],           %[ftmp8]            \n\t"
        "paddsh     %[ftmp3],   %[ftmp3],           %[ftmp4]            \n\t"
        "punpcklbh  %[ftmp5],   %[ftmp5],           %[ftmp0]            \n\t"
        "pmullh     %[ftmp5],   %[ftmp5],           %[ftmp9]            \n\t"
        "paddsh     %[ftmp3],   %[ftmp3],           %[ftmp5]            \n\t"
        "paddsh     %[ftmp3],   %[ftmp3],           %[ff_ph_40]         \n\t"
        "psrah      %[ftmp3],   %[ftmp3],           %[ftmp10]           \n\t"
        "packushb   %[ftmp3],   %[ftmp3],           %[ftmp0]            \n\t"
        "punpcklbh  %[ftmp3],   %[ftmp3],           %[ftmp0]            \n\t"

        "gssdlc1    %[ftmp3],   0x07(%[output_ptr])                     \n\t"
        "gssdrc1    %[ftmp3],   0x00(%[output_ptr])                     \n\t"

        "addiu      %[output_height], %[output_height], -0x01           \n\t"
        PTR_ADDU   "%[output_ptr], %[output_ptr], %[output_width]       \n\t"
        PTR_ADDU   "%[src_ptr], %[src_ptr], %[src_pixels_per_line]      \n\t"
        "bnez       %[output_height],               1b                  \n\t"
        : [ftmp0]"=&f"(ftmp0),              [ftmp1]"=&f"(ftmp1),
          [ftmp2]"=&f"(ftmp2),              [ftmp3]"=&f"(ftmp3),
          [ftmp4]"=&f"(ftmp4),              [ftmp5]"=&f"(ftmp5),
          [ftmp6]"=&f"(ftmp6),              [ftmp7]"=&f"(ftmp7),
          [ftmp8]"=&f"(ftmp8),              [ftmp9]"=&f"(ftmp9),
          [ftmp10]"=&f"(ftmp10),
          [tmp0]"=&r"(tmp[0]),
          [src_ptr]"+&r"(src_ptr),          [output_ptr]"+&r"(output_ptr),
          [output_height]"+&r"(output_height)
        : [src_pixels_per_line]"r"((mips_reg)src_pixels_per_line),
          [vp8_filter]"r"(vp8_filter),      [output_width]"r"(output_width),
          [ff_ph_40]"f"(ff_ph_40)
        : "memory"
    );
};

static INLINE void vp8_filter_block1dc_v6_mmi
(
   unsigned short *src_ptr,
   unsigned char *output_ptr,
   int output_pitch,
   unsigned int pixels_per_line,
   unsigned int pixel_step,
   unsigned int output_height,
   unsigned int output_width,
   const short *vp8_filter
) {
    DECLARE_ALIGNED(8, const uint64_t, ff_ph_40) = {0x0040004000400040ULL};
    uint32_t tmp[1];
    mips_reg addr[1];
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
#endif  // _MIPS_SIM == _ABIO32

    __asm__ volatile (
        "li         %[tmp0],    0x07                                    \n\t"
        "mtc1       %[tmp0],    %[ftmp7]                                \n\t"
        "ldc1       %[ftmp1],   0x10(%[vp8_filter])                     \n\t"
        PTR_SUBU   "%[src_ptr], %[src_ptr],         %[pixels_per_line]  \n\t"
        "ldc1       %[ftmp2],   0x20(%[vp8_filter])                     \n\t"
        PTR_SUBU   "%[src_ptr], %[src_ptr],         %[pixels_per_line]  \n\t"
        "ldc1       %[ftmp5],   0x30(%[vp8_filter])                     \n\t"
        "xor        %[ftmp0],   %[ftmp0],           %[ftmp0]            \n\t"
        "ldc1       %[ftmp6],   0x40(%[vp8_filter])                     \n\t"

        "1:                                                             \n\t"
        PTR_ADDU   "%[addr0],   %[src_ptr],         %[pixels_per_line]  \n\t"
        "gsldlc1    %[ftmp3],   0x07(%[addr0])                          \n\t"
        "gsldrc1    %[ftmp3],   0x00(%[addr0])                          \n\t"
        "pmullh     %[ftmp3],   %[ftmp3],           %[ftmp1]            \n\t"

        PTR_ADDU   "%[addr0],   %[src_ptr],     %[pixels_per_line_x4]   \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[addr0])                          \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[addr0])                          \n\t"
        "pmullh     %[ftmp4],   %[ftmp4],           %[ftmp6]            \n\t"
        "paddsh     %[ftmp3],   %[ftmp3],           %[ftmp4]            \n\t"

        PTR_ADDU   "%[addr0],   %[src_ptr],     %[pixels_per_line_x2]   \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[addr0])                          \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[addr0])                          \n\t"
        "pmullh     %[ftmp4],   %[ftmp4],           %[ftmp2]            \n\t"
        "paddsh     %[ftmp3],   %[ftmp3],           %[ftmp4]            \n\t"

        "gsldlc1    %[ftmp4],   0x07(%[src_ptr])                        \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[src_ptr])                        \n\t"
        "ldc1       %[ftmp8],   0x00(%[vp8_filter])                     \n\t"
        "pmullh     %[ftmp4],   %[ftmp4],           %[ftmp8]            \n\t"
        "paddsh     %[ftmp3],   %[ftmp3],           %[ftmp4]            \n\t"

        PTR_ADDU   "%[src_ptr], %[src_ptr],         %[pixels_per_line]  \n\t"

        PTR_ADDU   "%[addr0],   %[src_ptr],     %[pixels_per_line_x2]   \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[addr0])                          \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[addr0])                          \n\t"
        "pmullh     %[ftmp4],   %[ftmp4],           %[ftmp5]            \n\t"
        "paddsh     %[ftmp3],   %[ftmp3],           %[ftmp4]            \n\t"

        PTR_ADDU   "%[addr0],   %[src_ptr],     %[pixels_per_line_x4]   \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[addr0])                          \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[addr0])                          \n\t"
        "ldc1       %[ftmp8],   0x50(%[vp8_filter])                     \n\t"
        "pmullh     %[ftmp4],   %[ftmp4],           %[ftmp8]            \n\t"
        "paddsh     %[ftmp3],   %[ftmp3],           %[ftmp4]            \n\t"
        "paddsh     %[ftmp3],   %[ftmp3],           %[ff_ph_40]         \n\t"
        "psrah      %[ftmp3],   %[ftmp3],           %[ftmp7]            \n\t"
        "packushb   %[ftmp3],   %[ftmp3],           %[ftmp0]            \n\t"

        "gsswlc1    %[ftmp3],   0x03(%[output_ptr])                     \n\t"
        "gsswrc1    %[ftmp3],   0x00(%[output_ptr])                     \n\t"

        PTR_ADDIU  "%[output_height], %[output_height], -0x01           \n\t"
        PTR_ADDU   "%[output_ptr],    %[output_ptr],    %[output_pitch] \n\t"
        "bnez       %[output_height],   1b                              \n\t"
        : [ftmp0]"=&f"(ftmp0),              [ftmp1]"=&f"(ftmp1),
          [ftmp2]"=&f"(ftmp2),              [ftmp3]"=&f"(ftmp3),
          [ftmp4]"=&f"(ftmp4),              [ftmp5]"=&f"(ftmp5),
          [ftmp6]"=&f"(ftmp6),              [ftmp7]"=&f"(ftmp7),
          [ftmp8]"=&f"(ftmp8),
          [tmp0]"=&r"(tmp[0]),              [addr0]"=&r"(addr[0]),
          [src_ptr]"+&r"(src_ptr),
          [output_ptr]"+&r"(output_ptr),
          [output_height]"+&r"(output_height)
        : [pixels_per_line]"r"((mips_reg)pixels_per_line),
          [pixels_per_line_x2]"r"((mips_reg)(pixels_per_line<<1)),
          [pixels_per_line_x4]"r"((mips_reg)(pixels_per_line<<2)),
          [vp8_filter]"r"(vp8_filter),
          [output_pitch]"r"((mips_reg)output_pitch),
          [ff_ph_40]"f"(ff_ph_40)
        : "memory"
    );
}

void vp8_sixtap_predict4x4_mmi
(
    unsigned char  *src_ptr,
    int   src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pitch
)
{
  DECLARE_ALIGNED(16, unsigned short, FData2[16*16]);  /* Temp data bufffer used in filtering */

  const short *HFilter, *VFilter;

  HFilter = vp8_six_tap_mmi[xoffset];

  vp8_filter_block1d_h6_mmi(src_ptr - (2 * src_pixels_per_line), FData2, src_pixels_per_line, 1, 9, 8, HFilter);

  VFilter = vp8_six_tap_mmi[yoffset];

  vp8_filter_block1dc_v6_mmi(FData2 + 8, dst_ptr, dst_pitch, 8, 4 , 4, 4, VFilter);
}

void vp8_sixtap_predict8x8_mmi
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int  dst_pitch
)
{
    DECLARE_ALIGNED(16, unsigned short, FData2[256]);    /* Temp data bufffer used in filtering */

    const short *HFilter, *VFilter;

    HFilter = vp8_six_tap_mmi[xoffset];

    vp8_filter_block1d_h6_mmi(src_ptr - (2 * src_pixels_per_line)    ,  FData2    , src_pixels_per_line, 1, 13, 16, HFilter);
    vp8_filter_block1d_h6_mmi(src_ptr - (2 * src_pixels_per_line) + 4,  FData2 + 4, src_pixels_per_line, 1, 13, 16, HFilter);

    VFilter = vp8_six_tap_mmi[yoffset];

    vp8_filter_block1dc_v6_mmi(FData2 + 16, dst_ptr    , dst_pitch, 16, 8 , 8, 8, VFilter);
    vp8_filter_block1dc_v6_mmi(FData2 + 20, dst_ptr + 4, dst_pitch, 16, 8 , 8, 8, VFilter);
}

void vp8_sixtap_predict8x4_mmi
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int  dst_pitch
)
{
    DECLARE_ALIGNED(16, unsigned short, FData2[256]);    /* Temp data bufffer used in filtering */

    const short *HFilter, *VFilter;

    HFilter = vp8_six_tap_mmi[xoffset];

    vp8_filter_block1d_h6_mmi(src_ptr - (2 * src_pixels_per_line)    ,  FData2    , src_pixels_per_line, 1, 9, 16, HFilter);
    vp8_filter_block1d_h6_mmi(src_ptr - (2 * src_pixels_per_line) + 4,  FData2 + 4, src_pixels_per_line, 1, 9, 16, HFilter);

    VFilter = vp8_six_tap_mmi[yoffset];

    vp8_filter_block1dc_v6_mmi(FData2 + 16, dst_ptr    , dst_pitch, 16, 8 , 4, 8, VFilter);
    vp8_filter_block1dc_v6_mmi(FData2 + 20, dst_ptr + 4, dst_pitch, 16, 8 , 4, 8, VFilter);
}

void vp8_sixtap_predict16x16_mmi
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int  dst_pitch
)
{
    DECLARE_ALIGNED(16, unsigned short, FData2[24*24]);  /* Temp data bufffer used in filtering */

    const short *HFilter, *VFilter;

    HFilter = vp8_six_tap_mmi[xoffset];

    vp8_filter_block1d_h6_mmi(src_ptr - (2 * src_pixels_per_line)     ,  FData2    , src_pixels_per_line, 1, 21, 32, HFilter);
    vp8_filter_block1d_h6_mmi(src_ptr - (2 * src_pixels_per_line) + 4 ,  FData2 + 4, src_pixels_per_line, 1, 21, 32, HFilter);
    vp8_filter_block1d_h6_mmi(src_ptr - (2 * src_pixels_per_line) + 8 ,  FData2 + 8, src_pixels_per_line, 1, 21, 32, HFilter);
    vp8_filter_block1d_h6_mmi(src_ptr - (2 * src_pixels_per_line) + 12, FData2 + 12, src_pixels_per_line, 1, 21, 32, HFilter);

    VFilter = vp8_six_tap_mmi[yoffset];

    vp8_filter_block1dc_v6_mmi(FData2 + 32, dst_ptr     , dst_pitch, 32, 16 , 16, 16, VFilter);
    vp8_filter_block1dc_v6_mmi(FData2 + 36, dst_ptr + 4 , dst_pitch, 32, 16 , 16, 16, VFilter);
    vp8_filter_block1dc_v6_mmi(FData2 + 40, dst_ptr + 8 , dst_pitch, 32, 16 , 16, 16, VFilter);
    vp8_filter_block1dc_v6_mmi(FData2 + 44, dst_ptr + 12, dst_pitch, 32, 16 , 16, 16, VFilter);
}
