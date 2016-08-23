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

