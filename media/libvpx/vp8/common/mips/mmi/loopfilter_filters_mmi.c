/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdlib.h>
#include "vp8/common/loopfilter.h"
#include "vp8/common/onyxc_int.h"
#include "vpx_ports/asmdefs_mmi.h"

DECLARE_ALIGNED(8, static const uint64_t, ff_ph_01) = {0x0001000100010001ULL};
DECLARE_ALIGNED(8, static const uint64_t, ff_ph_003f) = {0x003f003f003f003fULL};
DECLARE_ALIGNED(8, static const uint64_t, ff_ph_0900) = {0x0900090009000900ULL};
DECLARE_ALIGNED(8, static const uint64_t, ff_ph_1200) = {0x1200120012001200ULL};
DECLARE_ALIGNED(8, static const uint64_t, ff_ph_1b00) = {0x1b001b001b001b00ULL};
DECLARE_ALIGNED(8, static const uint64_t, ff_pb_fe) = {0xfefefefefefefefeULL};
DECLARE_ALIGNED(8, static const uint64_t, ff_pb_80) = {0x8080808080808080ULL};
DECLARE_ALIGNED(8, static const uint64_t, ff_pb_04) = {0x0404040404040404ULL};
DECLARE_ALIGNED(8, static const uint64_t, ff_pb_03) = {0x0303030303030303ULL};

typedef unsigned char uc;

static signed char vp8_signed_char_clamp(int t)
{
    t = (t < -128 ? -128 : t);
    t = (t > 127 ? 127 : t);
    return (signed char) t;
}


/* should we apply any filter at all ( 11111111 yes, 00000000 no) */
static signed char vp8_filter_mask(uc limit, uc blimit,
                            uc p3, uc p2, uc p1, uc p0,
                            uc q0, uc q1, uc q2, uc q3)
{
    signed char mask = 0;
    mask |= (abs(p3 - p2) > limit);
    mask |= (abs(p2 - p1) > limit);
    mask |= (abs(p1 - p0) > limit);
    mask |= (abs(q1 - q0) > limit);
    mask |= (abs(q2 - q1) > limit);
    mask |= (abs(q3 - q2) > limit);
    mask |= (abs(p0 - q0) * 2 + abs(p1 - q1) / 2  > blimit);
    return mask - 1;
}

/* is there high variance internal edge ( 11111111 yes, 00000000 no) */
static signed char vp8_hevmask(uc thresh, uc p1, uc p0, uc q0, uc q1)
{
    signed char hev = 0;
    hev  |= (abs(p1 - p0) > thresh) * -1;
    hev  |= (abs(q1 - q0) > thresh) * -1;
    return hev;
}

static void vp8_filter(signed char mask, uc hev, uc *op1,
        uc *op0, uc *oq0, uc *oq1)

{
    signed char ps0, qs0;
    signed char ps1, qs1;
    signed char filter_value, Filter1, Filter2;
    signed char u;

    ps1 = (signed char) * op1 ^ 0x80;
    ps0 = (signed char) * op0 ^ 0x80;
    qs0 = (signed char) * oq0 ^ 0x80;
    qs1 = (signed char) * oq1 ^ 0x80;

    /* add outer taps if we have high edge variance */
    filter_value = vp8_signed_char_clamp(ps1 - qs1);
    filter_value &= hev;

    /* inner taps */
    filter_value = vp8_signed_char_clamp(filter_value + 3 * (qs0 - ps0));
    filter_value &= mask;

    /* save bottom 3 bits so that we round one side +4 and the other +3
     * if it equals 4 we'll set to adjust by -1 to account for the fact
     * we'd round 3 the other way
     */
    Filter1 = vp8_signed_char_clamp(filter_value + 4);
    Filter2 = vp8_signed_char_clamp(filter_value + 3);
    Filter1 >>= 3;
    Filter2 >>= 3;
    u = vp8_signed_char_clamp(qs0 - Filter1);
    *oq0 = u ^ 0x80;
    u = vp8_signed_char_clamp(ps0 + Filter2);
    *op0 = u ^ 0x80;
    filter_value = Filter1;

    /* outer tap adjustments */
    filter_value += 1;
    filter_value >>= 1;
    filter_value &= ~hev;

    u = vp8_signed_char_clamp(qs1 - filter_value);
    *oq1 = u ^ 0x80;
    u = vp8_signed_char_clamp(ps1 + filter_value);
    *op1 = u ^ 0x80;

}

static void vp8_mbfilter(signed char mask, uc hev,
                           uc *op2, uc *op1, uc *op0, uc *oq0, uc *oq1, uc *oq2)
{
    signed char s, u;
    signed char filter_value, Filter1, Filter2;
    signed char ps2 = (signed char) * op2 ^ 0x80;
    signed char ps1 = (signed char) * op1 ^ 0x80;
    signed char ps0 = (signed char) * op0 ^ 0x80;
    signed char qs0 = (signed char) * oq0 ^ 0x80;
    signed char qs1 = (signed char) * oq1 ^ 0x80;
    signed char qs2 = (signed char) * oq2 ^ 0x80;

    /* add outer taps if we have high edge variance */
    filter_value = vp8_signed_char_clamp(ps1 - qs1);
    filter_value = vp8_signed_char_clamp(filter_value + 3 * (qs0 - ps0));
    filter_value &= mask;

    Filter2 = filter_value;
    Filter2 &= hev;

    /* save bottom 3 bits so that we round one side +4 and the other +3 */
    Filter1 = vp8_signed_char_clamp(Filter2 + 4);
    Filter2 = vp8_signed_char_clamp(Filter2 + 3);
    Filter1 >>= 3;
    Filter2 >>= 3;
    qs0 = vp8_signed_char_clamp(qs0 - Filter1);
    ps0 = vp8_signed_char_clamp(ps0 + Filter2);


    /* only apply wider filter if not high edge variance */
    filter_value &= ~hev;
    Filter2 = filter_value;

    /* roughly 3/7th difference across boundary */
    u = vp8_signed_char_clamp((63 + Filter2 * 27) >> 7);
    s = vp8_signed_char_clamp(qs0 - u);
    *oq0 = s ^ 0x80;
    s = vp8_signed_char_clamp(ps0 + u);
    *op0 = s ^ 0x80;

    /* roughly 2/7th difference across boundary */
    u = vp8_signed_char_clamp((63 + Filter2 * 18) >> 7);
    s = vp8_signed_char_clamp(qs1 - u);
    *oq1 = s ^ 0x80;
    s = vp8_signed_char_clamp(ps1 + u);
    *op1 = s ^ 0x80;

    /* roughly 1/7th difference across boundary */
    u = vp8_signed_char_clamp((63 + Filter2 * 9) >> 7);
    s = vp8_signed_char_clamp(qs2 - u);
    *oq2 = s ^ 0x80;
    s = vp8_signed_char_clamp(ps2 + u);
    *op2 = s ^ 0x80;
}

/* should we apply any filter at all ( 11111111 yes, 00000000 no) */
static signed char vp8_simple_filter_mask(uc blimit, uc p1, uc p0, uc q0, uc q1)
{
/* Why does this cause problems for win32?
 * error C2143: syntax error : missing ';' before 'type'
 *  (void) limit;
 */
    signed char mask = (abs(p0 - q0) * 2 + abs(p1 - q1) / 2  <= blimit) * -1;
    return mask;
}

static void vp8_simple_filter(signed char mask, uc *op1, uc *op0, uc *oq0, uc *oq1)
{
    signed char filter_value, Filter1, Filter2;
    signed char p1 = (signed char) * op1 ^ 0x80;
    signed char p0 = (signed char) * op0 ^ 0x80;
    signed char q0 = (signed char) * oq0 ^ 0x80;
    signed char q1 = (signed char) * oq1 ^ 0x80;
    signed char u;

    filter_value = vp8_signed_char_clamp(p1 - q1);
    filter_value = vp8_signed_char_clamp(filter_value + 3 * (q0 - p0));
    filter_value &= mask;

    /* save bottom 3 bits so that we round one side +4 and the other +3 */
    Filter1 = vp8_signed_char_clamp(filter_value + 4);
    Filter1 >>= 3;
    u = vp8_signed_char_clamp(q0 - Filter1);
    *oq0  = u ^ 0x80;

    Filter2 = vp8_signed_char_clamp(filter_value + 3);
    Filter2 >>= 3;
    u = vp8_signed_char_clamp(p0 + Filter2);
    *op0 = u ^ 0x80;
}

void vp8_loop_filter_horizontal_edge_mmi
(
#if 1
    unsigned char *src_ptr,
    int src_pixel_step,
#else
    unsigned char *s,
    int p, /* pitch */
#endif
    const unsigned char *blimit,
    const unsigned char *limit,
    const unsigned char *thresh,
    int count
)
{
#if 1
    uint32_t tmp[2];
    mips_reg addr[2];
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
        "li         %[tmp0],    0x01                                    \n\t"
        "li         %[tmp1],    0x0b                                    \n\t"
        "1:                                                             \n\t"
        "gsldlc1    %[ftmp7],   0x07(%[limit])                          \n\t"
        "gsldrc1    %[ftmp7],   0x00(%[limit])                          \n\t"
        PTR_ADDU   "%[addr0],   %[src_ptr],         %[src_pixel_step]   \n\t"

        PTR_ADDU   "%[addr1],   %[addr0],           %[src_pixel_step_x2]\n\t"
        "gsldlc1    %[ftmp2],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp2],   0x00(%[addr1])                          \n\t"
        PTR_ADDU   "%[addr1],   %[src_ptr],         %[src_pixel_step_x2]\n\t"
        "gsldlc1    %[ftmp1],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp1],   0x00(%[addr1])                          \n\t"
        "mov.d      %[ftmp6],   %[ftmp1]                                \n\t"
        "psubusb    %[ftmp1],   %[ftmp1],           %[ftmp2]            \n\t"
        "psubusb    %[ftmp2],   %[ftmp2],           %[ftmp6]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp2]            \n\t"
        "psubusb    %[ftmp1],   %[ftmp1],           %[ftmp7]            \n\t"

        PTR_ADDU   "%[addr1],   %[src_ptr],         %[src_pixel_step]   \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[addr1])                          \n\t"
        "mov.d      %[ftmp3],   %[ftmp4]                                \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp6]            \n\t"
        "psubusb    %[ftmp6],   %[ftmp6],           %[ftmp3]            \n\t"
        "or         %[ftmp4],   %[ftmp4],           %[ftmp6]            \n\t"

        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp7]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"

        "gsldlc1    %[ftmp4],   0x07(%[src_ptr])                        \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[src_ptr])                        \n\t"
        "mov.d      %[ftmp0],   %[ftmp4]                                \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp3]            \n\t"
        "psubusb    %[ftmp3],   %[ftmp3],           %[ftmp0]            \n\t"
        "or         %[ftmp4],   %[ftmp4],           %[ftmp3]            \n\t"
        "mov.d      %[ftmp9],   %[ftmp4]                                \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp7]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"

        PTR_SUBU   "%[addr1],   %[src_ptr],         %[src_pixel_step_x4]\n\t"
        "gsldlc1    %[ftmp2],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp2],   0x00(%[addr1])                          \n\t"
        PTR_SUBU   "%[addr1],   %[addr0],           %[src_pixel_step_x4]\n\t"
        "gsldlc1    %[ftmp4],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[addr1])                          \n\t"
        "mov.d      %[ftmp5],   %[ftmp4]                                \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp2]            \n\t"
        "psubusb    %[ftmp2],   %[ftmp2],           %[ftmp5]            \n\t"
        "or         %[ftmp4],   %[ftmp4],           %[ftmp2]            \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp7]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"

        PTR_SUBU   "%[addr1],   %[src_ptr],         %[src_pixel_step_x2]\n\t"
        "gsldlc1    %[ftmp4],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[addr1])                          \n\t"
        "mov.d      %[ftmp3],   %[ftmp4]                                \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp5]            \n\t"
        "psubusb    %[ftmp5],   %[ftmp5],           %[ftmp3]            \n\t"
        "or         %[ftmp4],   %[ftmp4],           %[ftmp5]            \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp7]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"

        "mov.d      %[ftmp2],   %[ftmp3]                                \n\t"

        PTR_SUBU   "%[addr1],   %[src_ptr],         %[src_pixel_step]   \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[addr1])                          \n\t"
        "mov.d      %[ftmp5],   %[ftmp4]                                \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp3]            \n\t"
        "psubusb    %[ftmp3],   %[ftmp3],           %[ftmp5]            \n\t"
        "or         %[ftmp4],   %[ftmp4],           %[ftmp3]            \n\t"
        "mov.d      %[ftmp10],  %[ftmp4]                                \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp7]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"

        "gsldlc1    %[ftmp3],   0x07(%[addr0])                          \n\t"
        "gsldrc1    %[ftmp3],   0x00(%[addr0])                          \n\t"
        "mov.d      %[ftmp4],   %[ftmp3]                                \n\t"
        "psubusb    %[ftmp3],   %[ftmp3],           %[ftmp2]            \n\t"
        "psubusb    %[ftmp2],   %[ftmp2],           %[ftmp4]            \n\t"
        "or         %[ftmp2],   %[ftmp2],           %[ftmp3]            \n\t"
        "and        %[ftmp2],   %[ftmp2],           %[ff_pb_fe]         \n\t"
        "mtc1       %[tmp0],    %[ftmp8]                                \n\t"
        "psrlh      %[ftmp2],   %[ftmp2],           %[ftmp8]            \n\t"

        "mov.d      %[ftmp6],   %[ftmp5]                                \n\t"
        "gsldlc1    %[ftmp3],   0x07(%[src_ptr])                        \n\t"
        "gsldrc1    %[ftmp3],   0x00(%[src_ptr])                        \n\t"
        "psubusb    %[ftmp5],   %[ftmp5],           %[ftmp3]            \n\t"
        "psubusb    %[ftmp3],   %[ftmp3],           %[ftmp6]            \n\t"
        "or         %[ftmp5],   %[ftmp5],           %[ftmp3]            \n\t"
        "paddusb    %[ftmp5],   %[ftmp5],           %[ftmp5]            \n\t"
        "paddusb    %[ftmp5],   %[ftmp5],           %[ftmp2]            \n\t"

        "gsldlc1    %[ftmp7],   0x07(%[blimit])                         \n\t"
        "gsldrc1    %[ftmp7],   0x00(%[blimit])                         \n\t"

        "psubusb    %[ftmp5],   %[ftmp5],           %[ftmp7]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp5]            \n\t"
        "xor        %[ftmp5],   %[ftmp5],           %[ftmp5]            \n\t"
        "pcmpeqb    %[ftmp1],   %[ftmp1],           %[ftmp5]            \n\t"

        "gsldlc1    %[ftmp7],   0x07(%[thresh])                         \n\t"
        "gsldrc1    %[ftmp7],   0x00(%[thresh])                         \n\t"
        "psubusb    %[ftmp4],   %[ftmp9],           %[ftmp7]            \n\t"
        "psubusb    %[ftmp3],   %[ftmp10],          %[ftmp7]            \n\t"
        "paddb      %[ftmp4],   %[ftmp4],           %[ftmp3]            \n\t"

        "pcmpeqb    %[ftmp4],   %[ftmp4],           %[ftmp5]            \n\t"

        "pcmpeqb    %[ftmp5],   %[ftmp5],           %[ftmp5]            \n\t"
        "xor        %[ftmp4],   %[ftmp4],           %[ftmp5]            \n\t"

        PTR_SUBU   "%[addr1],   %[src_ptr],         %[src_pixel_step_x2]\n\t"
        "gsldlc1    %[ftmp2],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp2],   0x00(%[addr1])                          \n\t"
        "gsldlc1    %[ftmp7],   0x07(%[addr0])                          \n\t"
        "gsldrc1    %[ftmp7],   0x00(%[addr0])                          \n\t"
        "xor        %[ftmp2],   %[ftmp2],           %[ff_pb_80]         \n\t"
        "xor        %[ftmp7],   %[ftmp7],           %[ff_pb_80]         \n\t"
        "psubsb     %[ftmp2],   %[ftmp2],           %[ftmp7]            \n\t"
        "and        %[ftmp2],   %[ftmp2],           %[ftmp4]            \n\t"
        "xor        %[ftmp6],   %[ftmp6],           %[ff_pb_80]         \n\t"
        "xor        %[ftmp0],   %[ftmp0],           %[ff_pb_80]         \n\t"
        "mov.d      %[ftmp3],   %[ftmp0]                                \n\t"
        "psubsb     %[ftmp0],   %[ftmp0],           %[ftmp6]            \n\t"
        "paddsb     %[ftmp2],   %[ftmp2],           %[ftmp0]            \n\t"
        "paddsb     %[ftmp2],   %[ftmp2],           %[ftmp0]            \n\t"
        "paddsb     %[ftmp2],   %[ftmp2],           %[ftmp0]            \n\t"
        "and        %[ftmp1],   %[ftmp1],           %[ftmp2]            \n\t"
        "mov.d      %[ftmp2],   %[ftmp1]                                \n\t"
        "paddsb     %[ftmp1],   %[ftmp1],           %[ff_pb_04]         \n\t"
        "paddsb     %[ftmp2],   %[ftmp2],           %[ff_pb_03]         \n\t"

        "xor        %[ftmp0],   %[ftmp0],           %[ftmp0]            \n\t"
        "xor        %[ftmp5],   %[ftmp5],           %[ftmp5]            \n\t"
        "punpcklbh  %[ftmp0],   %[ftmp0],           %[ftmp2]            \n\t"
        "punpckhbh  %[ftmp5],   %[ftmp5],           %[ftmp2]            \n\t"
        "mtc1       %[tmp1],    %[ftmp8]                                \n\t"
        "psrah      %[ftmp0],   %[ftmp0],           %[ftmp8]            \n\t"
        "psrah      %[ftmp5],   %[ftmp5],           %[ftmp8]            \n\t"
        "packsshb   %[ftmp0],   %[ftmp0],           %[ftmp5]            \n\t"
        "mov.d      %[ftmp2],   %[ftmp0]                                \n\t"

        "xor        %[ftmp0],   %[ftmp0],           %[ftmp0]            \n\t"
        "mov.d      %[ftmp5],   %[ftmp1]                                \n\t"
        "punpcklbh  %[ftmp0],   %[ftmp0],           %[ftmp1]            \n\t"
        "psrah      %[ftmp0],   %[ftmp0],           %[ftmp8]            \n\t"
        "xor        %[ftmp1],   %[ftmp1],           %[ftmp1]            \n\t"
        "punpckhbh  %[ftmp1],   %[ftmp1],           %[ftmp5]            \n\t"
        "psrah      %[ftmp1],   %[ftmp1],           %[ftmp8]            \n\t"
        "mov.d      %[ftmp5],   %[ftmp0]                                \n\t"

        "packsshb   %[ftmp0],   %[ftmp0],           %[ftmp1]            \n\t"
        "paddsh     %[ftmp5],   %[ftmp5],           %[ff_ph_01]         \n\t"
        "paddsh     %[ftmp1],   %[ftmp1],           %[ff_ph_01]         \n\t"
        "mtc1       %[tmp0],    %[ftmp8]                                \n\t"
        "psrah      %[ftmp5],   %[ftmp5],           %[ftmp8]            \n\t"
        "psrah      %[ftmp1],   %[ftmp1],           %[ftmp8]            \n\t"
        "packsshb   %[ftmp5],   %[ftmp5],           %[ftmp1]            \n\t"
        "pandn      %[ftmp4],   %[ftmp4],           %[ftmp5]            \n\t"

        "paddsb     %[ftmp6],   %[ftmp6],           %[ftmp2]            \n\t"
        "xor        %[ftmp6],   %[ftmp6],           %[ff_pb_80]         \n\t"
        PTR_SUBU   "%[addr1],   %[src_ptr],         %[src_pixel_step]   \n\t"
        "gssdlc1    %[ftmp6],   0x07(%[addr1])                          \n\t"
        "gssdrc1    %[ftmp6],   0x00(%[addr1])                          \n\t"

        PTR_SUBU   "%[addr1],   %[src_ptr],         %[src_pixel_step_x2]\n\t"
        "gsldlc1    %[ftmp6],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp6],   0x00(%[addr1])                          \n\t"
        "xor        %[ftmp6],   %[ftmp6],           %[ff_pb_80]         \n\t"
        "paddsb     %[ftmp6],   %[ftmp6],           %[ftmp4]            \n\t"
        "xor        %[ftmp6],   %[ftmp6],           %[ff_pb_80]         \n\t"
        "gssdlc1    %[ftmp6],   0x07(%[addr1])                          \n\t"
        "gssdrc1    %[ftmp6],   0x00(%[addr1])                          \n\t"

        "psubsb     %[ftmp3],   %[ftmp3],           %[ftmp0]            \n\t"
        "xor        %[ftmp3],   %[ftmp3],           %[ff_pb_80]         \n\t"
        "gssdlc1    %[ftmp3],   0x07(%[src_ptr])                        \n\t"
        "gssdrc1    %[ftmp3],   0x00(%[src_ptr])                        \n\t"

        "psubsb     %[ftmp7],   %[ftmp7],           %[ftmp4]            \n\t"
        "xor        %[ftmp7],   %[ftmp7],           %[ff_pb_80]         \n\t"
        "gssdlc1    %[ftmp7],   0x07(%[addr0])                          \n\t"
        "gssdrc1    %[ftmp7],   0x00(%[addr0])                          \n\t"

        "addiu      %[count],   %[count],           -0x01               \n\t"
        PTR_ADDIU  "%[src_ptr], %[src_ptr],         0x08                \n\t"
        "bnez       %[count],   1b                                      \n\t"
        : [ftmp0]"=&f"(ftmp0),              [ftmp1]"=&f"(ftmp1),
          [ftmp2]"=&f"(ftmp2),              [ftmp3]"=&f"(ftmp3),
          [ftmp4]"=&f"(ftmp4),              [ftmp5]"=&f"(ftmp5),
          [ftmp6]"=&f"(ftmp6),              [ftmp7]"=&f"(ftmp7),
          [ftmp8]"=&f"(ftmp8),              [ftmp9]"=&f"(ftmp9),
          [ftmp10]"=&f"(ftmp10),
          [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
          [addr0]"=&r"(addr[0]),            [addr1]"=&r"(addr[1]),
          [src_ptr]"+&r"(src_ptr),          [count]"+&r"(count)
        : [limit]"r"(limit),                [blimit]"r"(blimit),
          [thresh]"r"(thresh),
          [src_pixel_step]"r"((mips_reg)src_pixel_step),
          [src_pixel_step_x2]"r"((mips_reg)(src_pixel_step<<1)),
          [src_pixel_step_x4]"r"((mips_reg)(src_pixel_step<<2)),
          [ff_ph_01]"f"(ff_ph_01),          [ff_pb_fe]"f"(ff_pb_fe),
          [ff_pb_80]"f"(ff_pb_80),          [ff_pb_04]"f"(ff_pb_04),
          [ff_pb_03]"f"(ff_pb_03)
        : "memory"
    );
#else
    int  hev = 0; /* high edge variance */
    signed char mask = 0;
    int i = 0;

    /* loop filter designed to work using chars so that we can make maximum use
     * of 8 bit simd instructions.
     */
    do
    {
        mask = vp8_filter_mask(limit[0], blimit[0],
                               s[-4*p], s[-3*p], s[-2*p], s[-1*p],
                               s[0*p], s[1*p], s[2*p], s[3*p]);

        hev = vp8_hevmask(thresh[0], s[-2*p], s[-1*p], s[0*p], s[1*p]);

        vp8_filter(mask, hev, s - 2 * p, s - 1 * p, s, s + 1 * p);

        ++s;
    }
    while (++i < count * 8);
#endif
}

void vp8_loop_filter_vertical_edge_mmi
(
    unsigned char *s,
    int p,
    const unsigned char *blimit,
    const unsigned char *limit,
    const unsigned char *thresh,
    int count
)
{
    int  hev = 0; /* high edge variance */
    signed char mask = 0;
    int i = 0;

    /* loop filter designed to work using chars so that we can make maximum use
     * of 8 bit simd instructions.
     */
    do
    {
        mask = vp8_filter_mask(limit[0], blimit[0],
                               s[-4], s[-3], s[-2], s[-1], s[0], s[1], s[2], s[3]);

        hev = vp8_hevmask(thresh[0], s[-2], s[-1], s[0], s[1]);

        vp8_filter(mask, hev, s - 2, s - 1, s, s + 1);

        s += p;
    }
    while (++i < count * 8);
}

void vp8_mbloop_filter_horizontal_edge_mmi
(
#if 1
    unsigned char *src_ptr,
    int  src_pixel_step,
#else
    unsigned char *s,
    int p,
#endif
    const unsigned char *blimit,
    const unsigned char *limit,
    const unsigned char *thresh,
    int count
)
{
#if 1
    uint32_t tmp[1];
    mips_reg addr[2];
    DECLARE_ALIGNED(8, const uint64_t, t[2]);
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
        "1:                                                             \n\t"
        "gsldlc1    %[ftmp7],   0x07(%[limit])                          \n\t"
        "gsldrc1    %[ftmp7],   0x00(%[limit])                          \n\t"
        PTR_ADDU   "%[addr0],   %[src_ptr],         %[src_pixel_step]   \n\t"

        PTR_SLL    "%[tmp0],    %[src_pixel_step],  0x01                \n\t"
        PTR_ADDU   "%[addr1],   %[addr0],           %[tmp0]             \n\t"
        "gsldlc1    %[ftmp2],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp2],   0x00(%[addr1])                          \n\t"

        PTR_ADDU   "%[addr1],   %[src_ptr],         %[tmp0]             \n\t"
        "gsldlc1    %[ftmp1],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp1],   0x00(%[addr1])                          \n\t"
        "mov.d      %[ftmp6],   %[ftmp1]                                \n\t"
        "psubusb    %[ftmp1],   %[ftmp1],           %[ftmp2]            \n\t"
        "psubusb    %[ftmp2],   %[ftmp2],           %[ftmp6]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp2]            \n\t"
        "psubusb    %[ftmp1],   %[ftmp1],           %[ftmp7]            \n\t"

        PTR_ADDU   "%[addr1],   %[src_ptr],         %[src_pixel_step]   \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[addr1])                          \n\t"
        "mov.d      %[ftmp3],   %[ftmp4]                                \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp6]            \n\t"
        "psubusb    %[ftmp6],   %[ftmp6],           %[ftmp3]            \n\t"
        "or         %[ftmp4],   %[ftmp4],           %[ftmp6]            \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp7]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"

        "gsldlc1    %[ftmp4],   0x07(%[src_ptr])                        \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[src_ptr])                        \n\t"
        "mov.d      %[ftmp0],   %[ftmp4]                                \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp3]            \n\t"
        "psubusb    %[ftmp3],   %[ftmp3],           %[ftmp0]            \n\t"
        "or         %[ftmp4],   %[ftmp4],           %[ftmp3]            \n\t"
        "sdc1       %[ftmp4],   0x00(%[t])                              \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp7]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"

        PTR_SLL    "%[tmp0],    %[src_pixel_step],  0x02                \n\t"
        PTR_SUBU   "%[addr1],   %[src_ptr],         %[tmp0]             \n\t"
        "gsldlc1    %[ftmp2],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp2],   0x00(%[addr1])                          \n\t"
        PTR_SUBU   "%[addr1],   %[addr0],           %[tmp0]             \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[addr1])                          \n\t"
        "mov.d      %[ftmp5],   %[ftmp4]                                \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp2]            \n\t"
        "psubusb    %[ftmp2],   %[ftmp2],           %[ftmp5]            \n\t"
        "or         %[ftmp4],   %[ftmp4],           %[ftmp2]            \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp7]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"

        PTR_SLL    "%[tmp0],    %[src_pixel_step],  0x01                \n\t"
        PTR_SUBU   "%[addr1],   %[src_ptr],         %[tmp0]             \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[addr1])                          \n\t"
        "mov.d      %[ftmp3],   %[ftmp4]                                \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp5]            \n\t"
        "psubusb    %[ftmp5],   %[ftmp5],           %[ftmp3]            \n\t"
        "or         %[ftmp4],   %[ftmp4],           %[ftmp5]            \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp7]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"

        "mov.d      %[ftmp2],   %[ftmp3]                                \n\t"

        PTR_SUBU   "%[addr1],   %[src_ptr],         %[src_pixel_step]   \n\t"
        "gsldlc1    %[ftmp4],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp4],   0x00(%[addr1])                          \n\t"
        "mov.d      %[ftmp5],   %[ftmp4]                                \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp3]            \n\t"
        "psubusb    %[ftmp3],   %[ftmp3],           %[ftmp5]            \n\t"
        "or         %[ftmp4],   %[ftmp4],           %[ftmp3]            \n\t"
        "sdc1       %[ftmp4],   0x08(%[t])                              \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp7]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"

        "gsldlc1    %[ftmp3],   0x07(%[addr0])                          \n\t"
        "gsldrc1    %[ftmp3],   0x00(%[addr0])                          \n\t"
        "mov.d      %[ftmp4],   %[ftmp3]                                \n\t"
        "psubusb    %[ftmp3],   %[ftmp3],           %[ftmp2]            \n\t"
        "psubusb    %[ftmp2],   %[ftmp2],           %[ftmp4]            \n\t"
        "or         %[ftmp2],   %[ftmp2],           %[ftmp3]            \n\t"
        "li         %[tmp0],    0x01                                    \n\t"
        "and        %[ftmp2],   %[ftmp2],           %[ff_pb_fe]         \n\t"
        "mtc1       %[tmp0],    %[ftmp8]                                \n\t"
        "psrlh      %[ftmp2],   %[ftmp2],           %[ftmp8]            \n\t"

        "mov.d      %[ftmp6],   %[ftmp5]                                \n\t"
        "mov.d      %[ftmp3],   %[ftmp0]                                \n\t"
        "psubusb    %[ftmp5],   %[ftmp5],           %[ftmp3]            \n\t"
        "psubusb    %[ftmp3],   %[ftmp3],           %[ftmp6]            \n\t"
        "or         %[ftmp5],   %[ftmp5],           %[ftmp3]            \n\t"
        "paddusb    %[ftmp5],   %[ftmp5],           %[ftmp5]            \n\t"
        "paddusb    %[ftmp5],   %[ftmp5],           %[ftmp2]            \n\t"

        "gsldlc1    %[ftmp7],   0x07(%[blimit])                         \n\t"
        "gsldrc1    %[ftmp7],   0x00(%[blimit])                         \n\t"

        "psubusb    %[ftmp5],   %[ftmp5],           %[ftmp7]            \n\t"
        "or         %[ftmp1],   %[ftmp1],           %[ftmp5]            \n\t"
        "xor        %[ftmp5],   %[ftmp5],           %[ftmp5]            \n\t"
        "pcmpeqb    %[ftmp1],   %[ftmp1],           %[ftmp5]            \n\t"

        "gsldlc1    %[ftmp7],   0x07(%[thresh])                         \n\t"
        "gsldrc1    %[ftmp7],   0x00(%[thresh])                         \n\t"
        "ldc1       %[ftmp4],   0x00(%[t])                              \n\t"
        "psubusb    %[ftmp4],   %[ftmp4],           %[ftmp7]            \n\t"
        "ldc1       %[ftmp3],   0x08(%[t])                              \n\t"
        "psubusb    %[ftmp3],   %[ftmp3],           %[ftmp7]            \n\t"
        "paddb      %[ftmp4],   %[ftmp4],           %[ftmp3]            \n\t"

        "pcmpeqb    %[ftmp4],   %[ftmp4],           %[ftmp5]            \n\t"

        "pcmpeqb    %[ftmp5],   %[ftmp5],           %[ftmp5]            \n\t"
        "xor        %[ftmp4],   %[ftmp4],           %[ftmp5]            \n\t"

        PTR_SLL    "%[tmp0],    %[src_pixel_step],  0x01                \n\t"
        PTR_SUBU   "%[addr1],   %[src_ptr],         %[tmp0]             \n\t"
        "gsldlc1    %[ftmp2],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp2],   0x00(%[addr1])                          \n\t"
        "gsldlc1    %[ftmp7],   0x07(%[addr0])                          \n\t"
        "gsldrc1    %[ftmp7],   0x00(%[addr0])                          \n\t"
        "xor        %[ftmp2],   %[ftmp2],           %[ff_pb_80]         \n\t"
        "xor        %[ftmp7],   %[ftmp7],           %[ff_pb_80]         \n\t"
        "psubsb     %[ftmp2],   %[ftmp2],           %[ftmp7]            \n\t"

        "xor        %[ftmp6],   %[ftmp6],           %[ff_pb_80]         \n\t"
        "xor        %[ftmp0],   %[ftmp0],           %[ff_pb_80]         \n\t"
        "mov.d      %[ftmp3],   %[ftmp0]                                \n\t"
        "psubsb     %[ftmp0],   %[ftmp0],           %[ftmp6]            \n\t"
        "paddsb     %[ftmp2],   %[ftmp2],           %[ftmp0]            \n\t"
        "paddsb     %[ftmp2],   %[ftmp2],           %[ftmp0]            \n\t"
        "paddsb     %[ftmp2],   %[ftmp2],           %[ftmp0]            \n\t"
        "and        %[ftmp1],   %[ftmp1],           %[ftmp2]            \n\t"

        "mov.d      %[ftmp2],   %[ftmp1]                                \n\t"
        "and        %[ftmp2],   %[ftmp2],           %[ftmp4]            \n\t"

        "mov.d      %[ftmp5],   %[ftmp2]                                \n\t"
        "paddsb     %[ftmp5],   %[ftmp5],           %[ff_pb_03]         \n\t"

        "xor        %[ftmp0],   %[ftmp0],           %[ftmp0]            \n\t"
        "xor        %[ftmp7],   %[ftmp7],           %[ftmp7]            \n\t"

        "li         %[tmp0],    0x0b                                    \n\t"
        "mtc1       %[tmp0],    %[ftmp8]                                \n\t"
        "punpcklbh  %[ftmp0],   %[ftmp0],           %[ftmp5]            \n\t"
        "psrah      %[ftmp0],   %[ftmp0],           %[ftmp8]            \n\t"
        "punpckhbh  %[ftmp7],   %[ftmp7],           %[ftmp5]            \n\t"
        "psrah      %[ftmp7],   %[ftmp7],           %[ftmp8]            \n\t"
        "packsshb   %[ftmp0],   %[ftmp0],           %[ftmp7]            \n\t"

        "mov.d      %[ftmp5],   %[ftmp0]                                \n\t"

        "paddsb     %[ftmp2],   %[ftmp2],           %[ff_pb_04]         \n\t"
        "xor        %[ftmp0],   %[ftmp0],           %[ftmp0]            \n\t"
        "xor        %[ftmp7],   %[ftmp7],           %[ftmp7]            \n\t"

        "punpcklbh  %[ftmp0],   %[ftmp0],           %[ftmp2]            \n\t"
        "psrah      %[ftmp0],   %[ftmp0],           %[ftmp8]            \n\t"
        "punpckhbh  %[ftmp7],   %[ftmp7],           %[ftmp2]            \n\t"
        "psrah      %[ftmp7],   %[ftmp7],           %[ftmp8]            \n\t"
        "packsshb   %[ftmp0],   %[ftmp0],           %[ftmp7]            \n\t"

        "psubsb     %[ftmp3],   %[ftmp3],           %[ftmp0]            \n\t"
        "paddsb     %[ftmp6],   %[ftmp6],           %[ftmp5]            \n\t"

        "pandn      %[ftmp4],   %[ftmp4],           %[ftmp1]            \n\t"

        "xor        %[ftmp0],   %[ftmp0],           %[ftmp0]            \n\t"

        "li         %[tmp0],    0x07                                    \n\t"
        "mtc1       %[tmp0],    %[ftmp8]                                \n\t"
        "xor        %[ftmp1],   %[ftmp1],           %[ftmp1]            \n\t"
        "xor        %[ftmp2],   %[ftmp2],           %[ftmp2]            \n\t"
        "punpcklbh  %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"
        "punpckhbh  %[ftmp2],   %[ftmp2],           %[ftmp4]            \n\t"
        "pmulhh     %[ftmp1],   %[ftmp1],           %[ff_ph_1b00]       \n\t"
        "pmulhh     %[ftmp2],   %[ftmp2],           %[ff_ph_1b00]       \n\t"
        "paddh      %[ftmp1],   %[ftmp1],           %[ff_ph_003f]       \n\t"
        "paddh      %[ftmp2],   %[ftmp2],           %[ff_ph_003f]       \n\t"
        "psrah      %[ftmp1],   %[ftmp1],           %[ftmp8]            \n\t"
        "psrah      %[ftmp2],   %[ftmp2],           %[ftmp8]            \n\t"
        "packsshb   %[ftmp1],   %[ftmp1],           %[ftmp2]            \n\t"

        "psubsb     %[ftmp3],   %[ftmp3],           %[ftmp1]            \n\t"
        "paddsb     %[ftmp6],   %[ftmp6],           %[ftmp1]            \n\t"

        "xor        %[ftmp3],   %[ftmp3],           %[ff_pb_80]         \n\t"
        "xor        %[ftmp6],   %[ftmp6],           %[ff_pb_80]         \n\t"
        PTR_SUBU   "%[addr1],   %[src_ptr],         %[src_pixel_step]   \n\t"
        "gssdlc1    %[ftmp6],   0x07(%[addr1])                          \n\t"
        "gssdrc1    %[ftmp6],   0x00(%[addr1])                          \n\t"
        "gssdlc1    %[ftmp3],   0x07(%[src_ptr])                        \n\t"
        "gssdrc1    %[ftmp3],   0x00(%[src_ptr])                        \n\t"

        "xor        %[ftmp1],   %[ftmp1],           %[ftmp1]            \n\t"
        "xor        %[ftmp2],   %[ftmp2],           %[ftmp2]            \n\t"
        "punpcklbh  %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"
        "punpckhbh  %[ftmp2],   %[ftmp2],           %[ftmp4]            \n\t"
        "pmulhh     %[ftmp1],   %[ftmp1],           %[ff_ph_1200]       \n\t"
        "pmulhh     %[ftmp2],   %[ftmp2],           %[ff_ph_1200]       \n\t"
        "paddh      %[ftmp1],   %[ftmp1],           %[ff_ph_003f]       \n\t"
        "paddh      %[ftmp2],   %[ftmp2],           %[ff_ph_003f]       \n\t"
        "psrah      %[ftmp1],   %[ftmp1],           %[ftmp8]            \n\t"
        "psrah      %[ftmp2],   %[ftmp2],           %[ftmp8]            \n\t"
        "packsshb   %[ftmp1],   %[ftmp1],           %[ftmp2]            \n\t"

        "gsldlc1    %[ftmp3],   0x07(%[addr0])                          \n\t"
        "gsldrc1    %[ftmp3],   0x00(%[addr0])                          \n\t"
        PTR_SLL    "%[tmp0],    %[src_pixel_step],  0x01                \n\t"
        PTR_SUBU   "%[addr1],   %[src_ptr],         %[tmp0]             \n\t"
        "gsldlc1    %[ftmp6],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp6],   0x00(%[addr1])                          \n\t"

        "xor        %[ftmp3],   %[ftmp3],           %[ff_pb_80]         \n\t"
        "xor        %[ftmp6],   %[ftmp6],           %[ff_pb_80]         \n\t"

        "paddsb     %[ftmp6],   %[ftmp6],           %[ftmp1]            \n\t"
        "psubsb     %[ftmp3],   %[ftmp3],           %[ftmp1]            \n\t"

        "xor        %[ftmp6],   %[ftmp6],           %[ff_pb_80]         \n\t"
        "xor        %[ftmp3],   %[ftmp3],           %[ff_pb_80]         \n\t"
        "gssdlc1    %[ftmp3],   0x07(%[addr0])                          \n\t"
        "gssdrc1    %[ftmp3],   0x00(%[addr0])                          \n\t"
        PTR_SUBU   "%[addr1],   %[src_ptr],         %[tmp0]             \n\t"
        "gssdlc1    %[ftmp6],   0x07(%[addr1])                          \n\t"
        "gssdrc1    %[ftmp6],   0x00(%[addr1])                          \n\t"

        "xor        %[ftmp1],   %[ftmp1],           %[ftmp1]            \n\t"
        "xor        %[ftmp2],   %[ftmp2],           %[ftmp2]            \n\t"
        "punpcklbh  %[ftmp1],   %[ftmp1],           %[ftmp4]            \n\t"
        "punpckhbh  %[ftmp2],   %[ftmp2],           %[ftmp4]            \n\t"
        "pmulhh     %[ftmp1],   %[ftmp1],           %[ff_ph_0900]       \n\t"
        "pmulhh     %[ftmp2],   %[ftmp2],           %[ff_ph_0900]       \n\t"
        "paddh      %[ftmp1],   %[ftmp1],           %[ff_ph_003f]       \n\t"
        "paddh      %[ftmp2],   %[ftmp2],           %[ff_ph_003f]       \n\t"
        "psrah      %[ftmp1],   %[ftmp1],           %[ftmp8]            \n\t"
        "psrah      %[ftmp2],   %[ftmp2],           %[ftmp8]            \n\t"
        "packsshb   %[ftmp1],   %[ftmp1],           %[ftmp2]            \n\t"

        PTR_SLL    "%[tmp0],    %[src_pixel_step],  0x02                \n\t"
        PTR_SUBU   "%[addr1],   %[addr0],           %[tmp0]             \n\t"
        "gsldlc1    %[ftmp6],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp6],   0x00(%[addr1])                          \n\t"
        PTR_ADDU   "%[addr1],   %[addr0],           %[src_pixel_step]   \n\t"
        "gsldlc1    %[ftmp3],   0x07(%[addr1])                          \n\t"
        "gsldrc1    %[ftmp3],   0x00(%[addr1])                          \n\t"

        "xor        %[ftmp6],   %[ftmp6],           %[ff_pb_80]         \n\t"
        "xor        %[ftmp3],   %[ftmp3],           %[ff_pb_80]         \n\t"

        "paddsb     %[ftmp6],   %[ftmp6],           %[ftmp1]            \n\t"
        "psubsb     %[ftmp3],   %[ftmp3],           %[ftmp1]            \n\t"

        "xor        %[ftmp6],   %[ftmp6],           %[ff_pb_80]         \n\t"
        "xor        %[ftmp3],   %[ftmp3],           %[ff_pb_80]         \n\t"
        PTR_ADDU   "%[addr1],   %[addr0],           %[src_pixel_step]   \n\t"
        "gssdlc1    %[ftmp3],   0x07(%[addr1])                          \n\t"
        "gssdrc1    %[ftmp3],   0x00(%[addr1])                          \n\t"
        PTR_SUBU   "%[addr1],   %[addr0],           %[tmp0]             \n\t"
        "gssdlc1    %[ftmp6],   0x07(%[addr1])                          \n\t"
        "gssdrc1    %[ftmp6],   0x00(%[addr1])                          \n\t"

        "addiu      %[count],   %[count],           -0x01               \n\t"
        PTR_ADDIU  "%[src_ptr], %[src_ptr],         0x08                \n\t"
        "bnez       %[count],   1b                                      \n\t"
        : [ftmp0]"=&f"(ftmp0),              [ftmp1]"=&f"(ftmp1),
          [ftmp2]"=&f"(ftmp2),              [ftmp3]"=&f"(ftmp3),
          [ftmp4]"=&f"(ftmp4),              [ftmp5]"=&f"(ftmp5),
          [ftmp6]"=&f"(ftmp6),              [ftmp7]"=&f"(ftmp7),
          [ftmp8]"=&f"(ftmp8),
          [tmp0]"=&r"(tmp[0]),
          [addr0]"=&r"(addr[0]),            [addr1]"=&r"(addr[1]),
          [src_ptr]"+&r"(src_ptr),          [count]"+&r"(count)
        : [limit]"r"(limit),                [blimit]"r"(blimit),
          [t]"r"(t),                        [thresh]"r"(thresh),
          [src_pixel_step]"r"((mips_reg)src_pixel_step),
          [ff_pb_fe]"f"(ff_pb_fe),          [ff_pb_80]"f"(ff_pb_80),
          [ff_pb_04]"f"(ff_pb_04),          [ff_pb_03]"f"(ff_pb_03),
          [ff_ph_0900]"f"(ff_ph_0900),      [ff_ph_1b00]"f"(ff_ph_1b00),
          [ff_ph_1200]"f"(ff_ph_1200),      [ff_ph_003f]"f"(ff_ph_003f)
        : "memory"
    );
#else
    signed char hev = 0; /* high edge variance */
    signed char mask = 0;
    int i = 0;

    /* loop filter designed to work using chars so that we can make maximum use
     * of 8 bit simd instructions.
     */
    do
    {

        mask = vp8_filter_mask(limit[0], blimit[0],
                               s[-4*p], s[-3*p], s[-2*p], s[-1*p],
                               s[0*p], s[1*p], s[2*p], s[3*p]);

        hev = vp8_hevmask(thresh[0], s[-2*p], s[-1*p], s[0*p], s[1*p]);

        vp8_mbfilter(mask, hev, s - 3 * p, s - 2 * p, s - 1 * p, s, s + 1 * p, s + 2 * p);

        ++s;
    }
    while (++i < count * 8);
#endif
}

void vp8_mbloop_filter_vertical_edge_mmi
(
    unsigned char *s,
    int p,
    const unsigned char *blimit,
    const unsigned char *limit,
    const unsigned char *thresh,
    int count
)
{
    signed char hev = 0; /* high edge variance */
    signed char mask = 0;
    int i = 0;

    do
    {

        mask = vp8_filter_mask(limit[0], blimit[0],
                               s[-4], s[-3], s[-2], s[-1], s[0], s[1], s[2], s[3]);

        hev = vp8_hevmask(thresh[0], s[-2], s[-1], s[0], s[1]);

        vp8_mbfilter(mask, hev, s - 3, s - 2, s - 1, s, s + 1, s + 2);

        s += p;
    }
    while (++i < count * 8);

}

void vp8_loop_filter_simple_horizontal_edge_mmi
(
    unsigned char *s,
    int p,
    const unsigned char *blimit
)
{
    signed char mask = 0;
    int i = 0;

    do
    {
        mask = vp8_simple_filter_mask(blimit[0], s[-2*p], s[-1*p], s[0*p], s[1*p]);
        vp8_simple_filter(mask, s - 2 * p, s - 1 * p, s, s + 1 * p);
        ++s;
    }
    while (++i < 16);
}

void vp8_loop_filter_simple_vertical_edge_mmi
(
    unsigned char *s,
    int p,
    const unsigned char *blimit
)
{
    signed char mask = 0;
    int i = 0;

    do
    {
        mask = vp8_simple_filter_mask(blimit[0], s[-2], s[-1], s[0], s[1]);
        vp8_simple_filter(mask, s - 2, s - 1, s, s + 1);
        s += p;
    }
    while (++i < 16);

}

/* Horizontal MB filtering */
void vp8_loop_filter_mbh_mmi(unsigned char *y_ptr, unsigned char *u_ptr,
                             unsigned char *v_ptr, int y_stride, int uv_stride,
                             loop_filter_info *lfi)
{
    vp8_mbloop_filter_horizontal_edge_mmi(y_ptr, y_stride, lfi->mblim, lfi->lim, lfi->hev_thr, 2);

    if (u_ptr)
        vp8_mbloop_filter_horizontal_edge_mmi(u_ptr, uv_stride, lfi->mblim, lfi->lim, lfi->hev_thr, 1);

    if (v_ptr)
        vp8_mbloop_filter_horizontal_edge_mmi(v_ptr, uv_stride, lfi->mblim, lfi->lim, lfi->hev_thr, 1);
}

/* Vertical MB Filtering */
void vp8_loop_filter_mbv_mmi(unsigned char *y_ptr, unsigned char *u_ptr,
                             unsigned char *v_ptr, int y_stride, int uv_stride,
                             loop_filter_info *lfi)
{
    vp8_mbloop_filter_vertical_edge_mmi(y_ptr, y_stride, lfi->mblim, lfi->lim, lfi->hev_thr, 2);

    if (u_ptr)
        vp8_mbloop_filter_vertical_edge_mmi(u_ptr, uv_stride, lfi->mblim, lfi->lim, lfi->hev_thr, 1);

    if (v_ptr)
        vp8_mbloop_filter_vertical_edge_mmi(v_ptr, uv_stride, lfi->mblim, lfi->lim, lfi->hev_thr, 1);
}

/* Horizontal B Filtering */
void vp8_loop_filter_bh_mmi(unsigned char *y_ptr, unsigned char *u_ptr,
                            unsigned char *v_ptr, int y_stride, int uv_stride,
                            loop_filter_info *lfi)
{
    vp8_loop_filter_horizontal_edge_mmi(y_ptr + 4 * y_stride, y_stride, lfi->blim, lfi->lim, lfi->hev_thr, 2);
    vp8_loop_filter_horizontal_edge_mmi(y_ptr + 8 * y_stride, y_stride, lfi->blim, lfi->lim, lfi->hev_thr, 2);
    vp8_loop_filter_horizontal_edge_mmi(y_ptr + 12 * y_stride, y_stride, lfi->blim, lfi->lim, lfi->hev_thr, 2);

    if (u_ptr)
        vp8_loop_filter_horizontal_edge_mmi(u_ptr + 4 * uv_stride, uv_stride, lfi->blim, lfi->lim, lfi->hev_thr, 1);

    if (v_ptr)
        vp8_loop_filter_horizontal_edge_mmi(v_ptr + 4 * uv_stride, uv_stride, lfi->blim, lfi->lim, lfi->hev_thr, 1);
}

void vp8_loop_filter_bhs_mmi(unsigned char *y_ptr, int y_stride,
                             const unsigned char *blimit)
{
    vp8_loop_filter_simple_horizontal_edge_mmi(y_ptr + 4 * y_stride, y_stride, blimit);
    vp8_loop_filter_simple_horizontal_edge_mmi(y_ptr + 8 * y_stride, y_stride, blimit);
    vp8_loop_filter_simple_horizontal_edge_mmi(y_ptr + 12 * y_stride, y_stride, blimit);
}

/* Vertical B Filtering */
void vp8_loop_filter_bv_mmi(unsigned char *y_ptr, unsigned char *u_ptr,
                            unsigned char *v_ptr, int y_stride, int uv_stride,
                            loop_filter_info *lfi)
{
    vp8_loop_filter_vertical_edge_mmi(y_ptr + 4, y_stride, lfi->blim, lfi->lim, lfi->hev_thr, 2);
    vp8_loop_filter_vertical_edge_mmi(y_ptr + 8, y_stride, lfi->blim, lfi->lim, lfi->hev_thr, 2);
    vp8_loop_filter_vertical_edge_mmi(y_ptr + 12, y_stride, lfi->blim, lfi->lim, lfi->hev_thr, 2);

    if (u_ptr)
        vp8_loop_filter_vertical_edge_mmi(u_ptr + 4, uv_stride, lfi->blim, lfi->lim, lfi->hev_thr, 1);

    if (v_ptr)
        vp8_loop_filter_vertical_edge_mmi(v_ptr + 4, uv_stride, lfi->blim, lfi->lim, lfi->hev_thr, 1);
}

void vp8_loop_filter_bvs_mmi(unsigned char *y_ptr, int y_stride,
                             const unsigned char *blimit)
{
    vp8_loop_filter_simple_vertical_edge_mmi(y_ptr + 4, y_stride, blimit);
    vp8_loop_filter_simple_vertical_edge_mmi(y_ptr + 8, y_stride, blimit);
    vp8_loop_filter_simple_vertical_edge_mmi(y_ptr + 12, y_stride, blimit);
}
