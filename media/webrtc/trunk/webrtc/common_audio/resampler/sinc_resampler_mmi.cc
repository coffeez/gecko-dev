/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/common_audio/resampler/sinc_resampler.h"

namespace webrtc {

float SincResampler::Convolve_MMI(const float* input_ptr, const float* k1,
                                  const float* k2,
                                  double kernel_interpolation_factor) {
#if 0
  float sum1 = 0;
  float sum2 = 0;

  // Generate a single output sample.  Unrolling this loop hurt performance in
  // local testing.
  int n = kKernelSize;
  while (n--) {
    sum1 += *input_ptr * *k1++;
    sum2 += *input_ptr++ * *k2++;
  }

  // Linearly interpolate the two "convolutions".
  return static_cast<float>((1.0 - kernel_interpolation_factor) * sum1 +
      kernel_interpolation_factor * sum2);
#else
    float result;
        __asm__ volatile (
            "li         $9, 0x01                                        \n\t"
            "and        $8, %[input_ptr], 0x0f                          \n\t"
            "xor        $f4, $f4, $f4                                   \n\t"
            "mtc1       $9, $f16                                        \n\t"
            "xor        $f6, $f6, $f6                                   \n\t"
            "xor        $f8, $f8, $f8                                   \n\t"
            "cvt.d.l    $f16, $f16                                      \n\t"
            "xor        $f10, $f10, $f10                                \n\t"
            "sub.d      $f16, $f16, %[kif]                              \n\t"
            "xor        $9, $9, $9                                      \n\t"
            "beqz       $8, 2f                                          \n\t"

            "dsubu      $8, $9, %[kKernelSize]                          \n\t"
            "bgez       $8, 4f                                          \n\t"
            "1:                                                         \n\t"
            "gsldlc1    $f0, 0x07(%[input_ptr])                         \n\t"
            "gsldrc1    $f0, 0x00(%[input_ptr])                         \n\t"
            "gsldlc1    $f2, 0x0f(%[input_ptr])                         \n\t"
            "gsldrc1    $f2, 0x08(%[input_ptr])                         \n\t"
            "gslqc1     $f14, $f12, 0x00(%[k1])                         \n\t"
            "madd.ps    $f4, $f4, $f12, $f0                             \n\t"
            "madd.ps    $f6, $f6, $f14, $f2                             \n\t"
            "gslqc1     $f14, $f12, 0x00(%[k2])                         \n\t"
            "madd.ps    $f8, $f8, $f12, $f0                             \n\t"
            "madd.ps    $f10, $f10, $f14, $f2                           \n\t"
            "daddiu     $9, $9, 0x04                                    \n\t"
            "daddiu     %[input_ptr], %[input_ptr], 0x10                \n\t"
            "daddiu     %[k1], %[k1], 0x10                              \n\t"
            "daddiu     %[k2], %[k2], 0x10                              \n\t"
            "dsubu      $8, $9, %[kKernelSize]                          \n\t"
            "bltz       $8, 1b                                          \n\t"
            "b          4f                                              \n\t"

            "2:                                                         \n\t"
            "dsubu      $8, $9, %[kKernelSize]                          \n\t"
            "bgez       $8, 4f                                          \n\t"
            "3:                                                         \n\t"
            "gslqc1     $f2, $f0, 0x00(%[input_ptr])                    \n\t"
            "gslqc1     $f14, $f12, 0x00(%[k1])                         \n\t"
            "madd.ps    $f4, $f4, $f12, $f0                             \n\t"
            "madd.ps    $f6, $f6, $f14, $f2                             \n\t"
            "gslqc1     $f14, $f12, 0x00(%[k2])                         \n\t"
            "madd.ps    $f8, $f8, $f12, $f0                             \n\t"
            "madd.ps    $f10, $f10, $f14, $f2                           \n\t"
            "daddiu     $9, $9, 0x04                                    \n\t"
            "daddiu     %[input_ptr], %[input_ptr], 0x10                \n\t"
            "daddiu     %[k1], %[k1], 0x10                              \n\t"
            "daddiu     %[k2], %[k2], 0x10                              \n\t"
            "dsubu      $8, $9, %[kKernelSize]                          \n\t"
            "bltz       $8, 3b                                          \n\t"

            "4:                                                         \n\t"
            "cvt.s.d    %[kif], %[kif]                                  \n\t"
            "cvt.s.d    $f16, $f16                                      \n\t"
            "punpcklwd  $f16, $f16, $f16                                \n\t"
            "punpcklwd  %[kif], %[kif], %[kif]                          \n\t"
            "mul.ps     $f4, $f4, $f16                                  \n\t"
            "mul.ps     $f6, $f6, $f16                                  \n\t"
            "madd.ps    $f4, $f4, $f8, %[kif]                           \n\t"
            "madd.ps    $f6, $f6, $f10, %[kif]                          \n\t"
            "add.ps     $f4, $f4, $f6                                   \n\t"
            "punpckhwd  $f10, $f4, $f4                                  \n\t"
            "add.s      %[result], $f4, $f10                            \n\t"
          : [result]"=f"((float)result)
          : [input_ptr]"r"((float *)input_ptr),     [k1]"r"((float *)k1),
            [k2]"r"((float *)k2),
            [kif]"f"((double)kernel_interpolation_factor),
            [kKernelSize]"r"(kKernelSize)
          : "memory","$8","$9","$f0","$f2","$f4","$f6","$f8","$f10","$f12",
            "$f14", "$f16"
        );

    return result;
#endif
}

}  // namespace webrtc
