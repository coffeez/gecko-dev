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

#include "vpx_ports/asmdefs_mmi.h"
#include "vpx/vpx_integer.h"
#include "vpx_ports/mem.h"

#define SAD_SRC_REF_ABS_SUB_64                                        \
    "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x0f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x08(%[src])                        \n\t" \
    "gsldlc1    %[ftmp3],   0x07(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x00(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x0f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x08(%[ref])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    "gsldlc1    %[ftmp1],   0x17(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x10(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x1f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x18(%[src])                        \n\t" \
    "gsldlc1    %[ftmp3],   0x17(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x10(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x1f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x18(%[ref])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    "gsldlc1    %[ftmp1],   0x27(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x20(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x2f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x28(%[src])                        \n\t" \
    "gsldlc1    %[ftmp3],   0x27(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x20(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x2f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x28(%[ref])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    "gsldlc1    %[ftmp1],   0x37(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x30(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x3f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x38(%[src])                        \n\t" \
    "gsldlc1    %[ftmp3],   0x37(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x30(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x3f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x38(%[ref])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t" \
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"

#define SAD_SRC_REF_ABS_SUB_32                                        \
    "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x0f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x08(%[src])                        \n\t" \
    "gsldlc1    %[ftmp3],   0x07(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x00(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x0f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x08(%[ref])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    "gsldlc1    %[ftmp1],   0x17(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x10(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x1f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x18(%[src])                        \n\t" \
    "gsldlc1    %[ftmp3],   0x17(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x10(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x1f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x18(%[ref])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t" \
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"

#define SAD_SRC_REF_ABS_SUB_16                                        \
    "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x0f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x08(%[src])                        \n\t" \
    "gsldlc1    %[ftmp3],   0x07(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x00(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x0f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x08(%[ref])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t" \
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"

#define SAD_SRC_REF_ABS_SUB_8                                         \
    "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x07(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x00(%[ref])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp2]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "paddw      %[ftmp3],   %[ftmp3],       %[ftmp1]            \n\t" \
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t" \
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"

#if _MIPS_SIM == _ABIO32
#define SAD_SRC_REF_ABS_SUB_4                                         \
    "ulw        %[tmp0],    0x00(%[src])                        \n\t" \
    "mtc1       %[tmp0],    %[ftmp1]                            \n\t" \
    "ulw        %[tmp0],    0x00(%[ref])                        \n\t" \
    "mtc1       %[tmp0],    %[ftmp2]                            \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp2]            \n\t" \
    "mthc1      $0,         %[ftmp1]                            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "paddw      %[ftmp3],   %[ftmp3],       %[ftmp1]            \n\t" \
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t" \
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"
#else  /* _MIPS_SIM == _ABI64 || _MIPS_SIM == _ABIN32 */
#define SAD_SRC_REF_ABS_SUB_4                                         \
    "gslwlc1    %[ftmp1],   0x03(%[src])                        \n\t" \
    "gslwrc1    %[ftmp1],   0x00(%[src])                        \n\t" \
    "gslwlc1    %[ftmp2],   0x03(%[ref])                        \n\t" \
    "gslwrc1    %[ftmp2],   0x00(%[ref])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp2]            \n\t" \
    "mthc1      $0,         %[ftmp1]                            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "paddw      %[ftmp3],   %[ftmp3],       %[ftmp1]            \n\t" \
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t" \
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"
#endif  /* _MIPS_SIM == _ABIO32 */


#define SAD_SRC_AVGREF_ABS_SUB_64                                     \
    "gsldlc1    %[ftmp1],   0x07(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp2],   0x0f(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp2],   0x08(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp3],   0x07(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x00(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x0f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x08(%[ref])                        \n\t" \
    "pavgb      %[ftmp3],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pavgb      %[ftmp4],   %[ftmp2],       %[ftmp4]            \n\t" \
    "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x0f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x08(%[src])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    "gsldlc1    %[ftmp1],   0x17(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp1],   0x10(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp2],   0x1f(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp2],   0x18(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp3],   0x17(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x10(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x1f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x18(%[ref])                        \n\t" \
    "pavgb      %[ftmp3],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pavgb      %[ftmp4],   %[ftmp2],       %[ftmp4]            \n\t" \
    "gsldlc1    %[ftmp1],   0x17(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x10(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x1f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x18(%[src])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    "gsldlc1    %[ftmp1],   0x27(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp1],   0x20(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp2],   0x2f(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp2],   0x28(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp3],   0x27(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x20(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x2f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x28(%[ref])                        \n\t" \
    "pavgb      %[ftmp3],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pavgb      %[ftmp4],   %[ftmp2],       %[ftmp4]            \n\t" \
    "gsldlc1    %[ftmp1],   0x27(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x20(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x2f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x28(%[src])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    "gsldlc1    %[ftmp1],   0x37(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp1],   0x30(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp2],   0x3f(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp2],   0x38(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp3],   0x37(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x30(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x3f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x38(%[ref])                        \n\t" \
    "pavgb      %[ftmp3],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pavgb      %[ftmp4],   %[ftmp2],       %[ftmp4]            \n\t" \
    "gsldlc1    %[ftmp1],   0x37(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x30(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x3f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x38(%[src])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    PTR_ADDIU  "%[second_pred], %[second_pred], 0x40            \n\t" \
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t" \
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"

#define SAD_SRC_AVGREF_ABS_SUB_32                                     \
    "gsldlc1    %[ftmp1],   0x07(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp2],   0x0f(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp2],   0x08(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp3],   0x07(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x00(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x0f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x08(%[ref])                        \n\t" \
    "pavgb      %[ftmp3],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pavgb      %[ftmp4],   %[ftmp2],       %[ftmp4]            \n\t" \
    "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x0f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x08(%[src])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    "gsldlc1    %[ftmp1],   0x17(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp1],   0x10(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp2],   0x1f(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp2],   0x18(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp3],   0x17(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x10(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x1f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x18(%[ref])                        \n\t" \
    "pavgb      %[ftmp3],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pavgb      %[ftmp4],   %[ftmp2],       %[ftmp4]            \n\t" \
    "gsldlc1    %[ftmp1],   0x17(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x10(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x1f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x18(%[src])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    PTR_ADDIU  "%[second_pred], %[second_pred], 0x20            \n\t" \
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t" \
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"

#define SAD_SRC_AVGREF_ABS_SUB_16                                     \
    "gsldlc1    %[ftmp1],   0x07(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp2],   0x0f(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp2],   0x08(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp3],   0x07(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp3],   0x00(%[ref])                        \n\t" \
    "gsldlc1    %[ftmp4],   0x0f(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp4],   0x08(%[ref])                        \n\t" \
    "pavgb      %[ftmp3],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pavgb      %[ftmp4],   %[ftmp2],       %[ftmp4]            \n\t" \
    "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t" \
    "gsldlc1    %[ftmp2],   0x0f(%[src])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x08(%[src])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp3]            \n\t" \
    "pasubub    %[ftmp2],   %[ftmp2],       %[ftmp4]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "biadd      %[ftmp2],   %[ftmp2]                            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp1]            \n\t" \
    "paddw      %[ftmp5],   %[ftmp5],       %[ftmp2]            \n\t" \
    PTR_ADDIU  "%[second_pred], %[second_pred], 0x10            \n\t" \
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t" \
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"

#define SAD_SRC_AVGREF_ABS_SUB_8                                      \
    "gsldlc1    %[ftmp1],   0x07(%[second_pred])                \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[second_pred])                \n\t" \
    "gsldlc1    %[ftmp2],   0x07(%[ref])                        \n\t" \
    "gsldrc1    %[ftmp2],   0x00(%[ref])                        \n\t" \
    "pavgb      %[ftmp2],   %[ftmp1],       %[ftmp2]            \n\t" \
    "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp2]            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "paddw      %[ftmp3],   %[ftmp3],       %[ftmp1]            \n\t" \
    PTR_ADDIU  "%[second_pred], %[second_pred], 0x08            \n\t" \
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t" \
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"

#if _MIPS_SIM == _ABIO32
#define SAD_SRC_AVGREF_ABS_SUB_4                                      \
    "ulw        %[tmp0],    0x00(%[second_pred])                \n\t" \
    "mtc1       %[tmp0],    %[ftmp1]                            \n\t" \
    "ulw        %[tmp0],    0x00(%[ref])                        \n\t" \
    "mtc1       %[tmp0],    %[ftmp2]                            \n\t" \
    "pavgb      %[ftmp2],   %[ftmp1],       %[ftmp2]            \n\t" \
    "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp2]            \n\t" \
    "mthc1      $0,         %[ftmp1]                            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "paddw      %[ftmp3],   %[ftmp3],       %[ftmp1]            \n\t" \
    PTR_ADDIU  "%[second_pred], %[second_pred], 0x04            \n\t" \
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t" \
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"
#else  /* _MIPS_SIM == _ABI64 || _MIPS_SIM == _ABIN32 */
#define SAD_SRC_AVGREF_ABS_SUB_4                                      \
    "gslwlc1    %[ftmp1],   0x03(%[second_pred])                \n\t" \
    "gslwrc1    %[ftmp1],   0x00(%[second_pred])                \n\t" \
    "gslwlc1    %[ftmp2],   0x03(%[ref])                        \n\t" \
    "gslwrc1    %[ftmp2],   0x00(%[ref])                        \n\t" \
    "pavgb      %[ftmp2],   %[ftmp1],       %[ftmp2]            \n\t" \
    "gsldlc1    %[ftmp1],   0x07(%[src])                        \n\t" \
    "gsldrc1    %[ftmp1],   0x00(%[src])                        \n\t" \
    "pasubub    %[ftmp1],   %[ftmp1],       %[ftmp2]            \n\t" \
    "mthc1      $0,         %[ftmp1]                            \n\t" \
    "biadd      %[ftmp1],   %[ftmp1]                            \n\t" \
    "paddw      %[ftmp3],   %[ftmp3],       %[ftmp1]            \n\t" \
    PTR_ADDIU  "%[second_pred], %[second_pred], 0x04            \n\t" \
    PTR_ADDU   "%[src],     %[src],         %[src_stride]       \n\t" \
    PTR_ADDU   "%[ref],     %[ref],         %[ref_stride]       \n\t"
#endif  /* _MIPS_SIM == _ABIO32 */


// 64x64
//sadMxN(64, 64)
unsigned int vpx_sad64x64_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x40                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_REF_ABS_SUB_64
    SAD_SRC_REF_ABS_SUB_64
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad64x64_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x40                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_AVGREF_ABS_SUB_64
    SAD_SRC_AVGREF_ABS_SUB_64
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNxK(64, 64, 3)
void vpx_sad64x64x3_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *ref_array, int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad64x64_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(64, 64, 8)
void vpx_sad64x64x8_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *ref_array, int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad64x64_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(64, 64)
void vpx_sad64x64x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad64x64_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 64x32
//sadMxN(64, 32)
unsigned int vpx_sad64x32_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_REF_ABS_SUB_64
    SAD_SRC_REF_ABS_SUB_64
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad64x32_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_AVGREF_ABS_SUB_64
    SAD_SRC_AVGREF_ABS_SUB_64
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNx4D(64, 32)
void vpx_sad64x32x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad64x32_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 32x64
//sadMxN(32, 64)
unsigned int vpx_sad32x64_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x40                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_REF_ABS_SUB_32
    SAD_SRC_REF_ABS_SUB_32
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad32x64_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x40                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_AVGREF_ABS_SUB_32
    SAD_SRC_AVGREF_ABS_SUB_32
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNx4D(32, 64)
void vpx_sad32x64x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad32x64_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 32x32
//sadMxN(32, 32)
unsigned int vpx_sad32x32_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_REF_ABS_SUB_32
    SAD_SRC_REF_ABS_SUB_32
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad32x32_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_AVGREF_ABS_SUB_32
    SAD_SRC_AVGREF_ABS_SUB_32
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNxK(32, 32, 3)
void vpx_sad32x32x3_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *ref_array, int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad32x32_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(32, 32, 8)
void vpx_sad32x32x8_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *ref_array, int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad32x32_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(32, 32)
void vpx_sad32x32x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad32x32_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 32x16
//sadMxN(32, 16)
unsigned int vpx_sad32x16_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x10                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_REF_ABS_SUB_32
    SAD_SRC_REF_ABS_SUB_32
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad32x16_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x10                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_AVGREF_ABS_SUB_32
    SAD_SRC_AVGREF_ABS_SUB_32
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNx4D(32, 16)
void vpx_sad32x16x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad32x16_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 16x32
//sadMxN(16, 32)
unsigned int vpx_sad16x32_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_REF_ABS_SUB_16
    SAD_SRC_REF_ABS_SUB_16
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad16x32_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x20                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_AVGREF_ABS_SUB_16
    SAD_SRC_AVGREF_ABS_SUB_16
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNx4D(16, 32)
void vpx_sad16x32x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad16x32_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 16x16
//sadMxN(16, 16)
unsigned int vpx_sad16x16_mmi(const uint8_t *src, int src_stride,
                              const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x10                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_REF_ABS_SUB_16
    SAD_SRC_REF_ABS_SUB_16
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad16x16_avg_mmi(const uint8_t *src, int src_stride,
                                  const uint8_t *ref, int ref_stride,
                                  const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x10                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_AVGREF_ABS_SUB_16
    SAD_SRC_AVGREF_ABS_SUB_16
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNxK(16, 16, 3)
void vpx_sad16x16x3_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *ref_array, int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad16x16_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(16, 16, 8)
void vpx_sad16x16x8_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *ref_array, int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad16x16_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(16, 16)
void vpx_sad16x16x4d_mmi(const uint8_t *src, int src_stride,
                         const uint8_t *const ref_array[], int ref_stride,
                         uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad16x16_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 16x8
//sadMxN(16, 8)
unsigned int vpx_sad16x8_mmi(const uint8_t *src, int src_stride,
                             const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x08                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_REF_ABS_SUB_16
    SAD_SRC_REF_ABS_SUB_16
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad16x8_avg_mmi(const uint8_t *src, int src_stride,
                                 const uint8_t *ref, int ref_stride,
                                 const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3, ftmp4, ftmp5;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x08                                \n\t"
    "xor        %[ftmp5],   %[ftmp5],       %[ftmp5]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_AVGREF_ABS_SUB_16
    SAD_SRC_AVGREF_ABS_SUB_16
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp5]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),              [ftmp4]"=&f"(ftmp4),
      [ftmp5]"=&f"(ftmp5),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNxK(16, 8, 3)
void vpx_sad16x8x3_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *ref_array, int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad16x8_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(16, 8, 8)
void vpx_sad16x8x8_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *ref_array, int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad16x8_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(16, 8)
void vpx_sad16x8x4d_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *const ref_array[], int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad16x8_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 8x16
//sadMxN(8, 16)
unsigned int vpx_sad8x16_mmi(const uint8_t *src, int src_stride,
                             const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x10                                \n\t"
    "xor        %[ftmp3],   %[ftmp3],       %[ftmp3]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    "addiu      %[tmp0],    %[tmp0],        -0x04               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp3]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad8x16_avg_mmi(const uint8_t *src, int src_stride,
                                 const uint8_t *ref, int ref_stride,
                                 const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x10                                \n\t"
    "xor        %[ftmp3],   %[ftmp3],       %[ftmp3]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_AVGREF_ABS_SUB_8
    SAD_SRC_AVGREF_ABS_SUB_8
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp3]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNxK(8, 16, 3)
void vpx_sad8x16x3_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *ref_array, int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad8x16_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(8, 16, 8)
void vpx_sad8x16x8_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *ref_array, int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad8x16_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(8, 16)
void vpx_sad8x16x4d_mmi(const uint8_t *src, int src_stride,
                        const uint8_t *const ref_array[], int ref_stride,
                        uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad8x16_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 8x8
//sadMxN(8, 8)
unsigned int vpx_sad8x8_mmi(const uint8_t *src, int src_stride,
                            const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3;

  __asm__ volatile (
    "xor        %[ftmp3],   %[ftmp3],       %[ftmp3]            \n\t"
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    "mfc1       %[sad],     %[ftmp3]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad8x8_avg_mmi(const uint8_t *src, int src_stride,
                                const uint8_t *ref, int ref_stride,
                                const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3;
  uint32_t tmp0;

  __asm__ volatile (
    "li         %[tmp0],    0x08                                \n\t"
    "xor        %[ftmp3],   %[ftmp3],       %[ftmp3]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_AVGREF_ABS_SUB_8
    SAD_SRC_AVGREF_ABS_SUB_8
    "addiu      %[tmp0],    %[tmp0],        -0x02               \n\t"
    "bnez       %[tmp0],    1b                                  \n\t"
    "mfc1       %[sad],     %[ftmp3]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNxK(8, 8, 3)
void vpx_sad8x8x3_mmi(const uint8_t *src, int src_stride,
                      const uint8_t *ref_array, int ref_stride,
                      uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad8x8_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(8, 8, 8)
void vpx_sad8x8x8_mmi(const uint8_t *src, int src_stride,
                      const uint8_t *ref_array, int ref_stride,
                      uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad8x8_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(8, 8)
void vpx_sad8x8x4d_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *const ref_array[], int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad8x8_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 8x4
//sadMxN(8, 4)
unsigned int vpx_sad8x4_mmi(const uint8_t *src, int src_stride,
                            const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3;

  __asm__ volatile (
    "xor        %[ftmp3],   %[ftmp3],       %[ftmp3]            \n\t"
    "1:                                                         \n\t"
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    SAD_SRC_REF_ABS_SUB_8
    "mfc1       %[sad],     %[ftmp3]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad8x4_avg_mmi(const uint8_t *src, int src_stride,
                                const uint8_t *ref, int ref_stride,
                                const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3;

  __asm__ volatile (
    "xor        %[ftmp3],   %[ftmp3],       %[ftmp3]            \n\t"
    SAD_SRC_AVGREF_ABS_SUB_8
    SAD_SRC_AVGREF_ABS_SUB_8
    SAD_SRC_AVGREF_ABS_SUB_8
    SAD_SRC_AVGREF_ABS_SUB_8
    "mfc1       %[sad],     %[ftmp3]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNxK(8, 4, 8)
void vpx_sad8x4x8_mmi(const uint8_t *src, int src_stride,
                      const uint8_t *ref_array, int ref_stride,
                      uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad8x4_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(8, 4)
void vpx_sad8x4x4d_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *const ref_array[], int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad8x4_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 4x8
//sadMxN(4, 8)
unsigned int vpx_sad4x8_mmi(const uint8_t *src, int src_stride,
                            const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3;
  uint32_t tmp0;

  __asm__ volatile (
    "xor        %[ftmp3],   %[ftmp3],       %[ftmp3]            \n\t"
    SAD_SRC_REF_ABS_SUB_4
    SAD_SRC_REF_ABS_SUB_4
    SAD_SRC_REF_ABS_SUB_4
    SAD_SRC_REF_ABS_SUB_4
    SAD_SRC_REF_ABS_SUB_4
    SAD_SRC_REF_ABS_SUB_4
    SAD_SRC_REF_ABS_SUB_4
    SAD_SRC_REF_ABS_SUB_4
    "mfc1       %[sad],     %[ftmp3]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad4x8_avg_mmi(const uint8_t *src, int src_stride,
                                const uint8_t *ref, int ref_stride,
                                const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3;
  uint32_t tmp0;

  __asm__ volatile (
    "xor        %[ftmp3],   %[ftmp3],       %[ftmp3]            \n\t"
    SAD_SRC_AVGREF_ABS_SUB_4
    SAD_SRC_AVGREF_ABS_SUB_4
    SAD_SRC_AVGREF_ABS_SUB_4
    SAD_SRC_AVGREF_ABS_SUB_4
    SAD_SRC_AVGREF_ABS_SUB_4
    SAD_SRC_AVGREF_ABS_SUB_4
    SAD_SRC_AVGREF_ABS_SUB_4
    SAD_SRC_AVGREF_ABS_SUB_4
    "mfc1       %[sad],     %[ftmp3]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNxK(4, 8, 8)
void vpx_sad4x8x8_mmi(const uint8_t *src, int src_stride,
                      const uint8_t *ref_array, int ref_stride,
                      uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad4x8_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(4, 8)
void vpx_sad4x8x4d_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *const ref_array[], int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad4x8_mmi(src, src_stride, ref_array[i], ref_stride);
}


// 4x4
//sadMxN(4, 4)
unsigned int vpx_sad4x4_mmi(const uint8_t *src, int src_stride,
                            const uint8_t *ref, int ref_stride) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3;
  uint32_t tmp0;

  __asm__ volatile (
    "xor        %[ftmp3],   %[ftmp3],       %[ftmp3]            \n\t"
    SAD_SRC_REF_ABS_SUB_4
    SAD_SRC_REF_ABS_SUB_4
    SAD_SRC_REF_ABS_SUB_4
    SAD_SRC_REF_ABS_SUB_4
    "mfc1       %[sad],     %[ftmp3]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

unsigned int vpx_sad4x4_avg_mmi(const uint8_t *src, int src_stride,
                                const uint8_t *ref, int ref_stride,
                                const uint8_t *second_pred) {
  unsigned int sad;
  double ftmp1, ftmp2, ftmp3;
  uint32_t tmp0;

  __asm__ volatile (
    "xor        %[ftmp3],   %[ftmp3],       %[ftmp3]            \n\t"
    SAD_SRC_AVGREF_ABS_SUB_4
    SAD_SRC_AVGREF_ABS_SUB_4
    SAD_SRC_AVGREF_ABS_SUB_4
    SAD_SRC_AVGREF_ABS_SUB_4
    "mfc1       %[sad],     %[ftmp3]                            \n\t"
    : [ftmp1]"=&f"(ftmp1),              [ftmp2]"=&f"(ftmp2),
      [ftmp3]"=&f"(ftmp3),
      [tmp0]"=&r"(tmp0),
      [src]"+&r"(src),                  [ref]"+&r"(ref),
      [second_pred]"+&r"((mips_reg)second_pred),
      [sad]"=&r"(sad)
    : [src_stride]"r"((mips_reg)src_stride),
      [ref_stride]"r"((mips_reg)ref_stride)
  );
  return sad;
}

//sadMxNxK(4, 4, 3)
void vpx_sad4x4x3_mmi(const uint8_t *src, int src_stride,
                      const uint8_t *ref_array, int ref_stride,
                      uint32_t *sad_array) {
  int i;
  for (i = 0; i < 3; ++i)
    sad_array[i] = vpx_sad4x4_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNxK(4, 4, 8)
void vpx_sad4x4x8_mmi(const uint8_t *src, int src_stride,
                      const uint8_t *ref_array, int ref_stride,
                      uint32_t *sad_array) {
  int i;
  for (i = 0; i < 8; ++i)
    sad_array[i] = vpx_sad4x4_mmi(src, src_stride, &ref_array[i], ref_stride);
}

//sadMxNx4D(4, 4)
void vpx_sad4x4x4d_mmi(const uint8_t *src, int src_stride,
                       const uint8_t *const ref_array[], int ref_stride,
                       uint32_t *sad_array) {
  int i;
  for (i = 0; i < 4; ++i)
    sad_array[i] = vpx_sad4x4_mmi(src, src_stride, ref_array[i], ref_stride);
}
