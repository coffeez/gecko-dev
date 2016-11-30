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

#include "vp8/encoder/onyx_int.h"
#include "vp8/encoder/quantize.h"
#include "vp8/common/quant_common.h"

void vp8_fast_quantize_b_mmi(BLOCK *b, BLOCKD *d)
{
    int i, rc, eob;
    int x, y, z, sz;
    short *coeff_ptr   = b->coeff;
    short *round_ptr   = b->round;
    short *quant_ptr   = b->quant_fast;
    short *qcoeff_ptr  = d->qcoeff;
    short *dqcoeff_ptr = d->dqcoeff;
    short *dequant_ptr = d->dequant;

    eob = -1;

    for (i = 0; i < 16; i++) {
        rc   = vp8_default_zig_zag1d[i];
        z    = coeff_ptr[rc];

        sz = (z >> 31);                              /* sign of z */
        x  = (z ^ sz) - sz;                          /* x = abs(z) */

        y  = ((x + round_ptr[rc]) * quant_ptr[rc]) >> 16; /* quantize (x) */
        x  = (y ^ sz) - sz;                          /* get the sign back */
        qcoeff_ptr[rc] = x;                          /* write to destination */
        dqcoeff_ptr[rc] = x * dequant_ptr[rc];       /* dequantized value */

        if (y) {
            eob = i;                                 /* last nonzero coeffs */
        }
    }

    *d->eob = (char)(eob + 1);
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
