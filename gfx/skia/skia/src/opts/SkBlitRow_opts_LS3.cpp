/*
 * Copyright 2014 Lemote Co,.ltd.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow.h"
#include "SkColorPriv.h"
#include "SkUtils.h"

#ifdef _MIPS_ARCH_LOONGSON3A

#include "MMIHelpers.h"

static void S32_D565_Opaque_LS3(uint16_t* SK_RESTRICT dst,
                          const SkPMColor* SK_RESTRICT src, int count,
                          U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    if (count <= 0) {
        return;
    }

    if (count >= 8) {
        while (((size_t)dst & 0x0F) != 0) {
            SkPMColor c = *src++;
            SkPMColorAssert(c);

            *dst++ = SkPixel32ToPixel16_ToU16(c);
            count--;
        }

        uint64_t tmp;
        const void * s = reinterpret_cast<const void*>(src);
        void * d = reinterpret_cast<void *>(dst);
        double r16_mask, g16_mask, b16_mask;
        double srlr, srlg, srlb, sllr, sllg, sllb;
        __asm__ volatile (
            ".set push \n\t"
            ".set arch=loongson3a \n\t"
            // r16_mask
            "ori %[tmp], $0, %[sk_r16_mask] \n\t"
            "mtc1 %[tmp], %[r16_mask] \n\t"
            "punpcklwd %[r16_mask], %[r16_mask], %[r16_mask] \n\t"
            // g16_mask
            "ori %[tmp], $0, %[sk_g16_mask] \n\t"
            "mtc1 %[tmp], %[g16_mask] \n\t"
            "punpcklwd %[g16_mask], %[g16_mask], %[g16_mask] \n\t"
            // b16_mask
            "ori %[tmp], $0, %[sk_b16_mask] \n\t"
            "mtc1 %[tmp], %[b16_mask] \n\t"
            "punpcklwd %[b16_mask], %[b16_mask], %[b16_mask] \n\t"
            // srlr
            "ori %[tmp], $0, %[sk_srlr] \n\t"
            "mtc1 %[tmp], %[srlr] \n\t"
            // srlg
            "ori %[tmp], $0, %[sk_srlg] \n\t"
            "mtc1 %[tmp], %[srlg] \n\t"
            // srlb
            "ori %[tmp], $0, %[sk_srlb] \n\t"
            "mtc1 %[tmp], %[srlb] \n\t"
            // sllr
            "ori %[tmp], $0, %[sk_sllr] \n\t"
            "mtc1 %[tmp], %[sllr] \n\t"
            // sllg
            "ori %[tmp], $0, %[sk_sllg] \n\t"
            "mtc1 %[tmp], %[sllg] \n\t"
            // sllb
            "ori %[tmp], $0, %[sk_sllb] \n\t"
            "mtc1 %[tmp], %[sllb] \n\t"
            ".set pop \n\t"
            :[r16_mask]"=f"(r16_mask), [g16_mask]"=f"(g16_mask),
             [b16_mask]"=f"(b16_mask), [srlr]"=f"(srlr),
             [srlg]"=f"(srlg), [srlb]"=f"(srlb), [sllr]"=f"(sllr),
             [sllg]"=f"(sllg), [sllb]"=f"(sllb), [tmp]"=&r"(tmp)
            :[sk_r16_mask]"i"(SK_R16_MASK), [sk_g16_mask]"i"(SK_G16_MASK),
             [sk_b16_mask]"i"(SK_B16_MASK), [sk_sllr]"i"(SK_R16_SHIFT),
             [sk_sllg]"i"(SK_G16_SHIFT), [sk_sllb]"i"(SK_B16_SHIFT),
             [sk_srlr]"i"(SK_R32_SHIFT + (8 - SK_R16_BITS)),
             [sk_srlg]"i"(SK_G32_SHIFT + (8 - SK_G16_BITS)),
             [sk_srlb]"i"(SK_B32_SHIFT + (8 - SK_B16_BITS))
        );
        while (count >= 8) {
            double drh, drl, dgh, dgl, dbh, dbl;
            double sp1h, sp1l, sp2h, sp2l, dph, dpl, t;
            double rh, rl, gh, gl, bh, bl, t1h, t1l, t2h, t2l;

            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                // Load 8 pixels of src.
                "gsldlc1 %[sp1h], 0xf(%[s]) \n\t"
                "gsldrc1 %[sp1h], 0x8(%[s]) \n\t"
                "gsldlc1 %[sp1l], 0x7(%[s]) \n\t"
                "gsldrc1 %[sp1l], 0x0(%[s]) \n\t"
                "gsldlc1 %[sp2h], 0x1f(%[s]) \n\t"
                "gsldrc1 %[sp2h], 0x18(%[s]) \n\t"
                "gsldlc1 %[sp2l], 0x17(%[s]) \n\t"
                "gsldrc1 %[sp2l], 0x10(%[s]) \n\t"
                // Calculate result r.
                _mm_psrlw(t1, sp1, srlr)
                _mm_and(t1, t1, r16_mask)
                _mm_psrlw(t2, sp2, srlr)
                _mm_and(t2, t2, r16_mask)
                _mm_packsswh(r, t1, t2, t)
                :[sp1h]"=&f"(sp1h), [sp1l]"=&f"(sp1l),
                 [sp2h]"=&f"(sp2h), [sp2l]"=&f"(sp2l),
                 [t1h]"=&f"(t1h), [t1l]"=&f"(t1l),
                 [t2h]"=&f"(t2h), [t2l]"=&f"(t2l),
                 [rh]"=f"(rh), [rl]"=f"(rl), [t]"=&f"(t)
                :[srlr]"f"(srlr), [s]"r"(s),
                 [r16_maskh]"f"(r16_mask),
                 [r16_maskl]"f"(r16_mask)
            );

            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                // Calculate result g.
                _mm_psrlw(t1, sp1, srlg)
                _mm_and(t1, t1, g16_mask)
                _mm_psrlw(t2, sp2, srlg)
                _mm_and(t2, t2, g16_mask)
                _mm_packsswh(g, t1, t2, t)
                :[t1h]"=&f"(t1h), [t1l]"=&f"(t1l),
                 [t2h]"=&f"(t2h), [t2l]"=&f"(t2l),
                 [gh]"=f"(gh), [gl]"=f"(gl),
                 [t]"+f"(t)
                :[sp1h]"f"(sp1h), [sp1l]"f"(sp1l),
                 [sp2h]"f"(sp2h), [sp2l]"f"(sp2l),
                 [srlg]"f"(srlg), [srlb]"f"(srlb),
                 [g16_maskh]"f"(g16_mask),
                 [g16_maskl]"f"(g16_mask)
            );

            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                // Calculate result b.
                _mm_psrlw(t1, sp1, srlb)
                _mm_and(t1, t1, b16_mask)
                _mm_psrlw(t2, sp2, srlb)
                _mm_and(t2, t2, b16_mask)
                _mm_packsswh(b, t1, t2, t)
                :[t1h]"+f"(t1h), [t1l]"+f"(t1l),
                 [t2h]"+f"(t2h), [t2l]"+f"(t2l),
                 [bh]"=f"(bh), [bl]"=f"(bl),
                 [t]"+f"(t)
                :[sp1h]"f"(sp1h), [sp1l]"f"(sp1l),
                 [sp2h]"f"(sp2h), [sp2l]"f"(sp2l),
                 [srlg]"f"(srlg), [srlb]"f"(srlb),
                 [b16_maskh]"f"(b16_mask),
                 [b16_maskl]"f"(b16_mask)
            );

            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                // Store 8 16-bit colors in dst.
                _mm_psllh(dr, r, sllr)
                _mm_psllh(dg, g, sllg)
                _mm_psllh(db, b, sllb)
                ".set pop \n\t"
                :[drh]"=&f"(drh), [drl]"=&f"(drl),
                 [dgh]"=&f"(dgh), [dgl]"=&f"(dgl),
                 [dbh]"=&f"(dbh), [dbl]"=&f"(dbl)
                :[rh]"f"(rh), [rl]"f"(rl),
                 [gh]"f"(gh), [gl]"f"(gl),
                 [bh]"f"(bh), [bl]"f"(bl),
                 [sllr]"f"(sllr), [sllg]"f"(sllg),
                 [sllb]"f"(sllb)
            );

            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                _mm_or(dp, dr, dg)
                _mm_or(dp, dp, db)
                ".set pop \n\t"
                :[dph]"=&f"(dph), [dpl]"=&f"(dpl)
                :[dgh]"f"(dgh), [dgl]"f"(dgl),
                 [dbh]"f"(dbh), [dbl]"f"(dbl),
                 [drh]"f"(drh), [drl]"f"(drl)
            );

            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                "gssqc1 %[dph], %[dpl], (%[d]) \n\t"
                ".set pop \n\t"
                ::[dph]"f"(dph), [dpl]"f"(dpl),
                  [d]"r"(d)
                : "memory"
            );

            s += 32;
            d += 16;
            count -= 8;
        }
        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<uint16_t*>(d);
    }

    if (count > 0) {
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            *dst++ = SkPixel32ToPixel16_ToU16(c);
        } while (--count != 0);
    }
}

static SkBlitRow::Proc platform_16_procs[] = {
    S32_D565_Opaque_LS3,                // S32_D565_Opaque
    NULL,                               // S32_D565_Blend
    NULL,                               // S32A_D565_Opaque
    //S32A_D565_Opaque_LS3,             // S32A_D565_Opaque
    NULL,                               // S32A_D565_Blend
    //S32_D565_Opaque_Dither_LS3,       // S32_D565_Opaque_Dither
    NULL,                               // S32_D565_Opaque_Dither
    NULL,                               // S32_D565_Blend_Dither
    //S32A_D565_Opaque_Dither_LS3,      // S32A_D565_Opaque_Dither
    NULL,                               // S32A_D565_Opaque_Dither
    NULL,                               // S32A_D565_Blend_Dither
};

#endif /* _MIPS_ARCH_LOONGSON3A */

SkBlitRow::Proc SkBlitRow::PlatformProcs565(unsigned flags) {
#ifdef _MIPS_ARCH_LOONGSON3A
    return platform_16_procs[flags];
#else
    return NULL;
#endif
}

#ifdef _MIPS_ARCH_LOONGSON3A

static void S32_Blend_BlitRow32_LS3(SkPMColor* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src,
                              int count, U8CPU alpha) {
    SkASSERT(alpha <= 255);
    if (count <= 0) {
        return;
    }

    uint32_t src_scale = SkAlpha255To256(alpha);
    uint32_t dst_scale = 256 - src_scale;

    if (count >= 4) {
        SkASSERT(((size_t)dst & 0x03) == 0);
        while (((size_t)dst & 0x0F) != 0) {
            *dst = SkAlphaMulQ(*src, src_scale) + SkAlphaMulQ(*dst, dst_scale);
            src++;
            dst++;
            count--;
        }

        uint64_t tmp;
        const void *s = reinterpret_cast<const void*>(src);
        void *d = reinterpret_cast<void *>(dst);
        double rb_mask, ag_mask, src_scale_wide, dst_scale_wide;
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
            ".set push \n\t"
            ".set arch=loongson3a \n\t"
            "xor %[ftmp0], %[ftmp0], %[ftmp0] \n\t"
            // rb_mask
            "ori %[tmp], $0, 0xff \n\t"
            "mtc1 %[tmp], %[ftmp1] \n\t"
            "pshufh %[rb_mask], %[ftmp1], %[ftmp0] \n\t"
            // ag_mask
            "ori %[tmp], $0, 0xff00 \n\t"
            "mtc1 %[tmp], %[ftmp1] \n\t"
            "pshufh %[ag_mask], %[ftmp1], %[ftmp0] \n\t"
            // Move scale factors to upper byte of word
            // src_scale_wide
            "sll %[tmp], %[src_scale], 8 \n\t"
            "mtc1 %[tmp], %[ftmp1] \n\t"
            "pshufh %[src_scale_wide], %[ftmp1], %[ftmp0] \n\t"
            // dst_scale_wide
            "sll %[tmp], %[dst_scale], 8 \n\t"
            "mtc1 %[tmp], %[ftmp1] \n\t"
            "pshufh %[dst_scale_wide], %[ftmp1], %[ftmp0] \n\t"
            ".set pop \n\t"
            :[rb_mask]"=f"(rb_mask), [ag_mask]"=f"(ag_mask),
             [src_scale_wide]"=f"(src_scale_wide),
             [dst_scale_wide]"=f"(dst_scale_wide),
             [tmp]"=&r"(tmp),
             [ftmp0]"=&f"(ftmp0), [ftmp1]"=&f"(ftmp1)
            :[src_scale]"r"(src_scale), [dst_scale]"r"(dst_scale)
        );
        while (count >= 4) {
            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                // Load 4 pixels each of src and dest.
                // %[ftmp0]: src_pixel h
                // %[ftmp1]: src_pixel l
                "gsldlc1 %[ftmp0], 0xf(%[s]) \n\t"
                "gsldrc1 %[ftmp0], 0x8(%[s]) \n\t"
                "gsldlc1 %[ftmp1], 0x7(%[s]) \n\t"
                "gsldrc1 %[ftmp1], 0x0(%[s]) \n\t"
                // %[ftmp2]: dst_pixel h
                // %[ftmp3]: dst_pixel l
                "gslqc1 %[ftmp2], %[ftmp3], (%[d]) \n\t"
                // Interleave Atom port 0/1 operations based on the execution port
                // constraints that multiply can only be executed on port 0 (while
                // boolean operations can be executed on either port 0 or port 1)
                // because GCC currently doesn't do a good job scheduling
                // instructions based on these constraints.

                // Get red and blue pixels into lower byte of each word.
                // (0, r, 0, b, 0, r, 0, b, 0, r, 0, b, 0, r, 0, b)
                // %[ftmp4]: src_rb h
                // %[ftmp5]: src_rb l
                "and %[ftmp4], %[ftmp0], %[rb_mask] \n\t"
                "and %[ftmp5], %[ftmp1], %[rb_mask] \n\t"
                // Multiply by scale.
                // (4 x (0, rs.h, 0, bs.h))
                // where rs.h stands for the higher byte of r * scale, and
                // bs.h the higher byte of b * scale.
                "pmulhuh %[ftmp4], %[ftmp4], %[src_scale_wide] \n\t"
                "pmulhuh %[ftmp5], %[ftmp5], %[src_scale_wide] \n\t"
                // Get alpha and green pixels into higher byte of each word.
                // (a, 0, g, 0, a, 0, g, 0, a, 0, g, 0, a, 0, g, 0)
                // %[ftmp6]: src_ag h
                // %[ftmp7]: src_ag l
                "and %[ftmp6], %[ftmp0], %[ag_mask] \n\t"
                "and %[ftmp7], %[ftmp1], %[ag_mask] \n\t"
                // Multiply by scale.
                // (4 x (as.h, as.l, gs.h, gs.l))
                "pmulhuh %[ftmp6], %[ftmp6], %[src_scale_wide] \n\t"
                "pmulhuh %[ftmp7], %[ftmp7], %[src_scale_wide] \n\t"
                // Clear the lower byte of the a*scale and g*scale results
                // (4 x (as.h, 0, gs.h, 0))
                "and %[ftmp6], %[ftmp6], %[ag_mask] \n\t"
                "and %[ftmp7], %[ftmp7], %[ag_mask] \n\t"
                // Operations the destination pixels are the same as on the
                // source pixels. See the comments above.
                // %[ftmp8]: dst_rb h
                // %[ftmp9]: dst_rb l
                "and %[ftmp8], %[ftmp2], %[rb_mask] \n\t"
                "and %[ftmp9], %[ftmp3], %[rb_mask] \n\t"
                "pmulhuh %[ftmp8], %[ftmp8], %[dst_scale_wide] \n\t"
                "pmulhuh %[ftmp9], %[ftmp9], %[dst_scale_wide] \n\t"
                // %[ftmp10]: dst_ag h
                // %[ftmp11]: dst_ag l
                "and %[ftmp10], %[ftmp2], %[ag_mask] \n\t"
                "and %[ftmp11], %[ftmp3], %[ag_mask] \n\t"
                "pmulhuh %[ftmp10], %[ftmp10], %[dst_scale_wide] \n\t"
                "pmulhuh %[ftmp11], %[ftmp11], %[dst_scale_wide] \n\t"
                "and %[ftmp10], %[ftmp10], %[ag_mask] \n\t"
                "and %[ftmp11], %[ftmp11], %[ag_mask] \n\t"
                // Combine back into RGBA.
                // (4 x (as.h, rs.h, gs.h, bs.h))
                "or %[ftmp0], %[ftmp4], %[ftmp6] \n\t"
                "or %[ftmp1], %[ftmp5], %[ftmp7] \n\t"
                "or %[ftmp2], %[ftmp8], %[ftmp10] \n\t"
                "or %[ftmp3], %[ftmp9], %[ftmp11] \n\t"
                // Add result
                "paddb %[ftmp0], %[ftmp0], %[ftmp2] \n\t"
                "paddb %[ftmp1], %[ftmp1], %[ftmp3] \n\t"
                ".set pop \n\t"
                : [ftmp0]"+f"(ftmp0), [ftmp1]"+f"(ftmp1),
                  [ftmp2]"=&f"(ftmp2), [ftmp3]"=&f"(ftmp3),
                  [ftmp4]"=&f"(ftmp4), [ftmp5]"=&f"(ftmp5),
                  [ftmp6]"=&f"(ftmp6), [ftmp7]"=&f"(ftmp7),
                  [ftmp8]"=&f"(ftmp8), [ftmp9]"=&f"(ftmp9),
                  [ftmp10]"=&f"(ftmp10), [ftmp11]"=&f"(ftmp11)
                : [rb_mask]"f"(rb_mask), [ag_mask]"f"(ag_mask),
                  [src_scale_wide]"f"(src_scale_wide),
                  [dst_scale_wide]"f"(dst_scale_wide),
                  [s]"r"(s), [d]"r"(d)
            );
            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                "gssqc1 %[ftmp0], %[ftmp1], (%[d]) \n\t"
                ".set pop \n\t"
                ::[ftmp0]"f"(ftmp0), [ftmp1]"f"(ftmp1),
                  [d]"r"(d)
                : "memory"
            );

            s += 16;
            d += 16;
            count -= 4;
        }
        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<SkPMColor*>(d);
    }

    while (count > 0) {
        *dst = SkAlphaMulQ(*src, src_scale) + SkAlphaMulQ(*dst, dst_scale);
        src++;
        dst++;
        count--;
    }
}

static void S32A_Opaque_BlitRow32_LS3(SkPMColor* SK_RESTRICT dst,
                                const SkPMColor* SK_RESTRICT src,
                                int count, U8CPU alpha) {
    SkASSERT(alpha == 255);
    if (count <= 0) {
        return;
    }

    if (count >= 4) {
        SkASSERT(((size_t)dst & 0x03) == 0);
        while (((size_t)dst & 0x0F) != 0) {
            *dst = SkPMSrcOver(*src, *dst);
            src++;
            dst++;
            count--;
        }

        uint64_t tmp;
        const void *s = reinterpret_cast<const void*>(src);
        void *d = reinterpret_cast<void*>(dst);
        double rb_mask, c_256, srl, sfh;
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
#endif  // _MIPS_SIM == _ABIO32
        __asm__ volatile (
            ".set push \n\t"
            ".set arch=loongson3a \n\t"
            "xor %[ftmp0], %[ftmp0], %[ftmp0] \n\t"
            // rb_mask
            "ori %[tmp], $0, 0xff \n\t"
            "mtc1 %[tmp], %[ftmp1] \n\t"
            "pshufh %[rb_mask], %[ftmp1], %[ftmp0] \n\t"
            // c_256, 8 copies of 256 (16-bit)
            "ori %[tmp], $0, 0x100 \n\t"
            "mtc1 %[tmp], %[ftmp1] \n\t"
            "pshufh %[c_256], %[ftmp1], %[ftmp0] \n\t"
            // srl = 8
            "ori %[tmp], $0, 0x8 \n\t"
            "mtc1 %[tmp], %[srl] \n\t"
            // sfh = 0xf5
            "ori %[tmp], $0, 0xf5 \n\t"
            "mtc1 %[tmp], %[sfh] \n\t"
            ".set pop \n\t"
            :[rb_mask]"=f"(rb_mask), [c_256]"=f"(c_256),
             [srl]"=f"(srl), [sfh]"=f"(sfh), [tmp]"=&r"(tmp),
             [ftmp0]"=&f"(ftmp0), [ftmp1]"=&f"(ftmp1)
        );
        while (count >= 4) {
            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                // Load 4 pixels
                // %[ftmp0]: src_pixel h
                // %[ftmp1]: src_pixel l
                "gsldlc1 %[ftmp0], 0xf(%[s]) \n\t"
                "gsldrc1 %[ftmp0], 0x8(%[s]) \n\t"
                "gsldlc1 %[ftmp1], 0x7(%[s]) \n\t"
                "gsldrc1 %[ftmp1], 0x0(%[s]) \n\t"
                // %[ftmp2]: dst_pixel h
                // %[ftmp3]: dst_pixel l
                "gslqc1 %[ftmp2], %[ftmp3], (%[d]) \n\t"
                // %[ftmp4]: dst_rb h
                // %[ftmp5]: dst_rb l
                "and %[ftmp4], %[ftmp2], %[rb_mask] \n\t"
                "and %[ftmp5], %[ftmp3], %[rb_mask] \n\t"
                // %[ftmp6]: dst_ag h
                // %[ftmp7]: dst_ag l
                "psrlh %[ftmp6], %[ftmp2], %[srl] \n\t"
                "psrlh %[ftmp7], %[ftmp3], %[srl] \n\t"
                // (a0, g0, a1, g1, a2, g2, a3, g3)  (low byte of each word)
                // %[ftmp8]: alpha h
                // %[ftmp9]: alpha l
                "psrlh %[ftmp8], %[ftmp0], %[srl] \n\t"
                "psrlh %[ftmp9], %[ftmp1], %[srl] \n\t"
                // (a0, a0, a1, a1, a2, g2, a3, g3)
                "pshufh %[ftmp8], %[ftmp8], %[sfh] \n\t"
                // (a0, a0, a1, a1, a2, a2, a3, a3)
                "pshufh %[ftmp9], %[ftmp9], %[sfh] \n\t"
                // Subtract alphas from 256, to get 1..256
                "psubh %[ftmp8], %[c_256], %[ftmp8] \n\t"
                "psubh %[ftmp9], %[c_256], %[ftmp9] \n\t"
                // Multiply by red and blue by src alpha.
                "pmullh %[ftmp4], %[ftmp4], %[ftmp8] \n\t"
                "pmullh %[ftmp5], %[ftmp5], %[ftmp9] \n\t"
                // Multiply by alpha and green by src alpha.
                "pmullh %[ftmp6], %[ftmp6], %[ftmp8] \n\t"
                "pmullh %[ftmp7], %[ftmp7], %[ftmp9] \n\t"
                // Divide by 256.
                "psrlh %[ftmp4], %[ftmp4], %[srl] \n\t"
                "psrlh %[ftmp5], %[ftmp5], %[srl] \n\t"
                // Mask out high bits (already in the right place)
                "pandn %[ftmp6], %[rb_mask], %[ftmp6] \n\t"
                "pandn %[ftmp7], %[rb_mask], %[ftmp7] \n\t"
                // Combine back into RGBA.
                "or %[ftmp2], %[ftmp4], %[ftmp6] \n\t"
                "or %[ftmp3], %[ftmp5], %[ftmp7] \n\t"
                // Add result
                "paddb %[ftmp0], %[ftmp0], %[ftmp2] \n\t"
                "paddb %[ftmp1], %[ftmp1], %[ftmp3] \n\t"
                ".set pop \n\t"
                : [ftmp0]"+f"(ftmp0), [ftmp1]"+f"(ftmp1),
                  [ftmp2]"=&f"(ftmp2), [ftmp3]"=&f"(ftmp3),
                  [ftmp4]"=&f"(ftmp4), [ftmp5]"=&f"(ftmp5),
                  [ftmp6]"=&f"(ftmp6), [ftmp7]"=&f"(ftmp7),
                  [ftmp8]"=&f"(ftmp8), [ftmp9]"=&f"(ftmp9)
                : [rb_mask]"f"(rb_mask), [c_256]"f"(c_256),
                  [srl]"f"(srl), [sfh]"f"(sfh), [s]"r"(s), [d]"r"(d)
            );
            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                "gssqc1 %[ftmp0], %[ftmp1], (%[d]) \n\t"
                ".set pop \n\t"
                ::[ftmp0]"f"(ftmp0), [ftmp1]"f"(ftmp1),
                  [d]"r"(d)
                : "memory"
            );
            s += 16;
            d += 16;
            count -= 4;
        }
        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<SkPMColor*>(d);
    }

    while (count > 0) {
        *dst = SkPMSrcOver(*src, *dst);
        src++;
        dst++;
        count--;
    }
}

static void S32A_Blend_BlitRow32_LS3(SkPMColor* SK_RESTRICT dst,
                               const SkPMColor* SK_RESTRICT src,
                               int count, U8CPU alpha) {
    SkASSERT(alpha <= 255);
    if (count <= 0) {
        return;
    }

    if (count >= 4) {
        while (((size_t)dst & 0x0F) != 0) {
            *dst = SkBlendARGB32(*src, *dst, alpha);
            src++;
            dst++;
            count--;
        }

        uint32_t src_scale = SkAlpha255To256(alpha);

        uint64_t tmp;
        const void *s = reinterpret_cast<const void*>(src);
        void *d = reinterpret_cast<void*>(dst);
        double src_scale_wide, rb_mask, c_256, srl, sfh;
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
            ".set push \n\t"
            ".set arch=loongson3a \n\t"
            "xor %[ftmp0], %[ftmp0], %[ftmp0] \n\t"
            // src_scale_wide
            "mtc1 %[src_scale], %[ftmp1] \n\t"
            "pshufh %[src_scale_wide], %[ftmp1], %[ftmp0] \n\t"
            // rb_mask
            "ori %[tmp], $0, 0xff \n\t"
            "mtc1 %[tmp], %[ftmp1] \n\t"
            "pshufh %[rb_mask], %[ftmp1], %[ftmp0] \n\t"
            // c_256, 8 copies of 256 (16-bit)
            "ori %[tmp], $0, 0x100 \n\t"
            "mtc1 %[tmp], %[ftmp1] \n\t"
            "pshufh %[c_256], %[ftmp1], %[ftmp0] \n\t"
            // srl
            "ori %[tmp], $0, 0x8 \n\t"
            "mtc1 %[tmp], %[srl] \n\t"
            // sfh
            "ori %[tmp], $0, 0xf5 \n\t"
            "mtc1 %[tmp], %[sfh] \n\t"
            ".set pop \n\t"
            :[src_scale_wide]"=f"(src_scale_wide),
             [rb_mask]"=f"(rb_mask), [c_256]"=f"(c_256),
             [srl]"=f"(srl), [sfh]"=f"(sfh), [tmp]"=&r"(tmp),
             [ftmp0]"=&f"(ftmp0), [ftmp1]"=&f"(ftmp1)
            :[src_scale]"r"(src_scale << 8)
        );
        while (count >= 4) {
            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                // Load 4 pixels each of src and dest.
                // %[ftmp0]: src_pixel h
                // %[ftmp1]: src_pixel l
                "gsldlc1 %[ftmp0], 0xf(%[s]) \n\t"
                "gsldrc1 %[ftmp0], 0x8(%[s]) \n\t"
                "gsldlc1 %[ftmp1], 0x7(%[s]) \n\t"
                "gsldrc1 %[ftmp1], 0x0(%[s]) \n\t"
                // %[ftmp2]: dst_pixel h
                // %[ftmp3]: dst_pixel l
                "gslqc1 %[ftmp2], %[ftmp3], (%[d]) \n\t"
                // Get red and blue pixels into lower byte of each word.
                // %[ftmp4]: dst_rb h
                // %[ftmp5]: dst_rb l
                "and %[ftmp4], %[ftmp2], %[rb_mask] \n\t"
                "and %[ftmp5], %[ftmp3], %[rb_mask] \n\t"
                // %[ftmp6]: src_rb h
                // %[ftmp7]: src_rb l
                "and %[ftmp6], %[ftmp0], %[rb_mask] \n\t"
                "and %[ftmp7], %[ftmp1], %[rb_mask] \n\t"
                // Get alpha and green into lower byte of each word.
                // %[ftmp8]: dst_ag h
                // %[ftmp9]: dst_ag l
                "psrlh %[ftmp8], %[ftmp2], %[srl] \n\t"
                "psrlh %[ftmp9], %[ftmp3], %[srl] \n\t"
                // %[ftmp10]: src_ag h
                // %[ftmp11]: src_ag l
                "psrlh %[ftmp10], %[ftmp0], %[srl] \n\t"
                "psrlh %[ftmp11], %[ftmp1], %[srl] \n\t"
                : [ftmp0]"+f"(ftmp0), [ftmp1]"+f"(ftmp1),
                  [ftmp2]"=&f"(ftmp2), [ftmp3]"=&f"(ftmp3),
                  [ftmp4]"=&f"(ftmp4), [ftmp5]"=&f"(ftmp5),
                  [ftmp6]"=&f"(ftmp6), [ftmp7]"=&f"(ftmp7),
                  [ftmp8]"=&f"(ftmp8), [ftmp9]"=&f"(ftmp9),
                  [ftmp10]"=&f"(ftmp10), [ftmp11]"=&f"(ftmp11)
                : [rb_mask]"f"(rb_mask),
                  [srl]"f"(srl), [s]"r"(s), [d]"r"(d)
            );

            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                // Put per-pixel alpha in low byte of each word.
                // After the following two statements, the dst_alpha looks like
                // (0, a0, 0, a0, 0, a1, 0, a1, 0, a2, 0, a2, 0, a3, 0, a3)
                // %[ftmp0]: dst_alpha h
                // %[ftmp1]: dst_alpha l
                "pshufh %[ftmp0], %[ftmp10], %[sfh] \n\t"
                "pshufh %[ftmp1], %[ftmp11], %[sfh] \n\t"
                // dst_alpha = dst_alpha * src_scale
                // Because src_scales are in the higher byte of each word and
                // we use mulhi here, the resulting alpha values are already
                // in the right place and don't need to be divided by 256.
                // (0, sa0, 0, sa0, 0, sa1, 0, sa1, 0, sa2, 0, sa2, 0, sa3, 0, sa3)
                "pmulhuh %[ftmp0], %[ftmp0], %[src_scale_wide] \n\t"
                "pmulhuh %[ftmp1], %[ftmp1], %[src_scale_wide] \n\t"
                // Subtract alphas from 256, to get 1..256
                "psubh %[ftmp0], %[c_256], %[ftmp0] \n\t"
                "psubh %[ftmp1], %[c_256], %[ftmp1] \n\t"
                // Multiply red and blue by dst pixel alpha.
                "pmullh %[ftmp4], %[ftmp4], %[ftmp0] \n\t"
                "pmullh %[ftmp5], %[ftmp5], %[ftmp1] \n\t"
                // Multiply alpha and green by dst pixel alpha.
                "pmullh %[ftmp8], %[ftmp8], %[ftmp0] \n\t"
                "pmullh %[ftmp9], %[ftmp9], %[ftmp1] \n\t"
                // Multiply red and blue by global alpha.
                // (4 x (0, rs.h, 0, bs.h))
                // where rs.h stands for the higher byte of r * src_scale,
                // and bs.h the higher byte of b * src_scale.
                // Again, because we use mulhi, the resuling red and blue
                // values are already in the right place and don't need to
                // be divided by 256.
                "pmulhuh %[ftmp6], %[ftmp6], %[src_scale_wide] \n\t"
                "pmulhuh %[ftmp7], %[ftmp7], %[src_scale_wide] \n\t"
                // Multiply alpha and green by global alpha.
                // (4 x (0, as.h, 0, gs.h))
                "pmulhuh %[ftmp10], %[ftmp10], %[src_scale_wide] \n\t"
                "pmulhuh %[ftmp11], %[ftmp11], %[src_scale_wide] \n\t"
                // Divide by 256.
                "psrlh %[ftmp4], %[ftmp4], %[srl] \n\t"
                "psrlh %[ftmp5], %[ftmp5], %[srl] \n\t"
                // Mask out low bits (goodies already in the right place; no need to divide)
                "pandn %[ftmp8], %[rb_mask], %[ftmp8] \n\t"
                "pandn %[ftmp9], %[rb_mask], %[ftmp9] \n\t"
                // Shift alpha and green to higher byte of each word.
                // (4 x (as.h, 0, gs.h, 0))
                "psllh %[ftmp10], %[ftmp10], %[srl] \n\t"
                "psllh %[ftmp11], %[ftmp11], %[srl] \n\t"
                ".set pop \n\t"
                : [ftmp0]"=&f"(ftmp0), [ftmp1]"=&f"(ftmp1),
                  [ftmp4]"+f"(ftmp4), [ftmp5]"+f"(ftmp5),
                  [ftmp6]"+f"(ftmp6), [ftmp7]"+f"(ftmp7),
                  [ftmp8]"+f"(ftmp8), [ftmp9]"+f"(ftmp9),
                  [ftmp10]"+f"(ftmp10), [ftmp11]"+f"(ftmp11)
                : [src_scale_wide]"f"(src_scale_wide), [rb_mask]"f"(rb_mask),
                  [c_256]"f"(c_256), [srl]"f"(srl), [sfh]"f"(sfh)
            );

            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                // Combine back into RGBA.
                "or %[ftmp2], %[ftmp4], %[ftmp8] \n\t"
                "or %[ftmp3], %[ftmp5], %[ftmp9] \n\t"
                "or %[ftmp0], %[ftmp6], %[ftmp10] \n\t"
                "or %[ftmp1], %[ftmp7], %[ftmp11] \n\t"
                // Add two pixels into result.
                "paddb %[ftmp0], %[ftmp0], %[ftmp2] \n\t"
                "paddb %[ftmp1], %[ftmp1], %[ftmp3] \n\t"
                ".set pop \n\t"
                : [ftmp0]"=&f"(ftmp0), [ftmp1]"=&f"(ftmp1),
                  [ftmp2]"=&f"(ftmp2), [ftmp3]"=&f"(ftmp3)
                : [ftmp4]"f"(ftmp4), [ftmp5]"f"(ftmp5),
                  [ftmp6]"f"(ftmp6), [ftmp7]"f"(ftmp7),
                  [ftmp8]"f"(ftmp8), [ftmp9]"f"(ftmp9),
                  [ftmp10]"f"(ftmp10), [ftmp11]"f"(ftmp11)
            );

            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                "gssqc1 %[ftmp0], %[ftmp1], (%[d]) \n\t"
                ".set pop \n\t"
                ::[ftmp0]"f"(ftmp0), [ftmp1]"f"(ftmp1),
                  [d]"r"(d)
                : "memory"
            );
            s += 16;
            d += 16;
            count -= 4;
        }
        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<SkPMColor*>(d);
    }

    while (count > 0) {
        *dst = SkBlendARGB32(*src, *dst, alpha);
        src++;
        dst++;
        count--;
    }
}
static SkBlitRow::Proc32 platform_32_procs_LS3[] = {
    NULL,                               // S32_Opaque,
    S32_Blend_BlitRow32_LS3,            // S32_Blend,
    S32A_Opaque_BlitRow32_LS3,          // S32A_Opaque
    S32A_Blend_BlitRow32_LS3,           // S32A_Blend,
};

#endif /* _MIPS_ARCH_LOONGSON3A */

SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
#ifdef _MIPS_ARCH_LOONGSON3A
    return platform_32_procs_LS3[flags];
#else
    return NULL;
#endif
}

#ifdef _MIPS_ARCH_LOONGSON3A

static void Color32_LS3(SkPMColor dst[], const SkPMColor src[], int count,
                  SkPMColor color) {
    if (count <= 0) {
        return;
    }

    if (0 == color) {
        if (src != dst) {
            sk_memcpy32(dst, src, count * sizeof(SkPMColor));
        }
        return;
    }

    unsigned colorA = SkGetPackedA32(color);
    if (255 == colorA) {
        sk_memset32(dst, color, count);
    } else {
        unsigned scale = 256 - SkAlpha255To256(colorA);

        if (count >= 4) {
            SkASSERT(((size_t)dst & 0x03) == 0);
            while (((size_t)dst & 0x0F) != 0) {
                *dst = color + SkAlphaMulQ(*src, scale);
                src++;
                dst++;
                count--;
            }

            uint64_t tmp;
            const void *s = reinterpret_cast<const void*>(src);
            void *d = reinterpret_cast<void*>(dst);
            double rb_mask, src_scale_wide, color_wide, srl;
#if _MIPS_SIM == _ABIO32
            register double ftmp0 asm ("$f0");
            register double ftmp1 asm ("$f2");
            register double ftmp2 asm ("$f4");
            register double ftmp3 asm ("$f6");
            register double ftmp4 asm ("$f8");
            register double ftmp5 asm ("$f10");
#else
            register double ftmp0 asm ("$f0");
            register double ftmp1 asm ("$f1");
            register double ftmp2 asm ("$f2");
            register double ftmp3 asm ("$f3");
            register double ftmp4 asm ("$f4");
            register double ftmp5 asm ("$f5");
#endif  // _MIPS_SIM == _ABIO32
            __asm__ volatile (
                ".set push \n\t"
                ".set arch=loongson3a \n\t"
                "xor %[ftmp0], %[ftmp0], %[ftmp0] \n\t"
                // rb_mask
                "ori %[tmp], $0, 0xff \n\t"
                "mtc1 %[tmp], %[ftmp1] \n\t"
                "pshufh %[rb_mask], %[ftmp1], %[ftmp0] \n\t"
                // src_scale_wide
                "mtc1 %[scale], %[ftmp1] \n\t"
                "pshufh %[src_scale_wide], %[ftmp1], %[ftmp0] \n\t"
                // color_wide
                "mtc1 %[color], %[color_wide] \n\t"
                "punpcklwd %[color_wide], %[color_wide], %[color_wide] \n\t"
                // srl = 8
                "ori %[tmp], $0, 0x8 \n\t"
                "mtc1 %[tmp], %[srl] \n\t"
                ".set pop \n\t"
                :[rb_mask]"=f"(rb_mask), [src_scale_wide]"=f"(src_scale_wide),
                 [color_wide]"=f"(color_wide), [srl]"=f"(srl), [tmp]"=&r"(tmp),
                 [ftmp0]"=&f"(ftmp0), [ftmp1]"=&f"(ftmp1)
                :[scale]"r"(scale), [color]"r"(color)
            );
            while (count >= 4) {
                __asm__ volatile (
                    ".set push \n\t"
                    ".set arch=loongson3a \n\t"
                    // Load 4 pixels each of src and dest.
                    // %[ftmp0]: src_pixel h
                    // %[ftmp1]: src_pixel l
                    "gsldlc1 %[ftmp0], 0xf(%[s]) \n\t"
                    "gsldrc1 %[ftmp0], 0x8(%[s]) \n\t"
                    "gsldlc1 %[ftmp1], 0x7(%[s]) \n\t"
                    "gsldrc1 %[ftmp1], 0x0(%[s]) \n\t"
                    // Get red and blue pixels into lower byte of each word.
                    // %[ftmp2]: src_rb h
                    // %[ftmp3]: src_rb l
                    "and %[ftmp2], %[ftmp0], %[rb_mask] \n\t"
                    "and %[ftmp3], %[ftmp1], %[rb_mask] \n\t"
                    // Get alpha and green into lower byte of each word.
                    // %[ftmp4]: src_ag h
                    // %[ftmp5]: src_ag l
                    "psrlh %[ftmp4], %[ftmp0], %[srl] \n\t"
                    "psrlh %[ftmp5], %[ftmp1], %[srl] \n\t"
                    // Multiply by scale.
                    "pmullh %[ftmp2], %[ftmp2], %[src_scale_wide] \n\t"
                    "pmullh %[ftmp3], %[ftmp3], %[src_scale_wide] \n\t"
                    "pmullh %[ftmp4], %[ftmp4], %[src_scale_wide] \n\t"
                    "pmullh %[ftmp5], %[ftmp5], %[src_scale_wide] \n\t"
                    // Divide by 256.
                    "psrlh %[ftmp2], %[ftmp2], %[srl] \n\t"
                    "psrlh %[ftmp3], %[ftmp3], %[srl] \n\t"
                    "pandn %[ftmp4], %[rb_mask], %[ftmp4] \n\t"
                    "pandn %[ftmp5], %[rb_mask], %[ftmp5] \n\t"
                    // Combine back into RGBA.
                    "or %[ftmp0], %[ftmp2], %[ftmp4] \n\t"
                    "or %[ftmp1], %[ftmp3], %[ftmp5] \n\t"
                    // Add color to result.
                    "paddb %[ftmp0], %[ftmp0], %[color_wide] \n\t"
                    "paddb %[ftmp1], %[ftmp1], %[color_wide] \n\t"
                    ".set pop \n\t"
                    : [ftmp0]"+f"(ftmp0), [ftmp1]"+f"(ftmp1),
                      [ftmp2]"=&f"(ftmp2), [ftmp3]"=&f"(ftmp3),
                      [ftmp4]"=&f"(ftmp4), [ftmp5]"=&f"(ftmp5)
                    : [rb_mask]"f"(rb_mask), [src_scale_wide]"f"(src_scale_wide),
                      [color_wide]"f"(color_wide), [s]"r"(s), [srl]"f"(srl)
                );
                __asm__ volatile (
                    ".set push \n\t"
                    ".set arch=loongson3a \n\t"
                    // Store result.
                    "gssqc1 %[ftmp0], %[ftmp1], (%[d]) \n\t"
                    ".set pop \n\t"
                    ::[ftmp0]"f"(ftmp0), [ftmp1]"f"(ftmp1),
                      [d]"r"(d)
                    : "memory"
                );

                s += 16;
                d += 16;
                count -= 4;
            }
            src = reinterpret_cast<const SkPMColor*>(s);
            dst = reinterpret_cast<SkPMColor*>(d);
        }

        while (count > 0) {
            *dst = color + SkAlphaMulQ(*src, scale);
            src += 1;
            dst += 1;
            count--;
        }
    }
}

#endif /* _MIPS_ARCH_LOONGSON3A */

SkBlitRow::ColorProc SkBlitRow::PlatformColorProc() {
#ifdef _MIPS_ARCH_LOONGSON3A
    return Color32_LS3;
#else
    return NULL;
#endif
}

SkBlitRow::ColorRectProc PlatformColorRectProcFactory() {
    return NULL;
}
