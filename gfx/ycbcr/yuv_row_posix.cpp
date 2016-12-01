// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "yuv_row.h"
#include "mozilla/SSE.h"

#define DCHECK(a)

extern "C" {

#if defined(ARCH_CPU_X86_64)

// We don't need CPUID guards here, since x86-64 implies SSE2.

// AMD64 ABI uses register paremters.
void FastConvertYUVToRGB32Row(const uint8* y_buf,  // rdi
                              const uint8* u_buf,  // rsi
                              const uint8* v_buf,  // rdx
                              uint8* rgb_buf,      // rcx
                              int width) {         // r8
  asm(
  "jmp    1f\n"
"0:"
  "movzb  (%1),%%r10\n"
  "add    $0x1,%1\n"
  "movzb  (%2),%%r11\n"
  "add    $0x1,%2\n"
  "movq   2048(%5,%%r10,8),%%xmm0\n"
  "movzb  (%0),%%r10\n"
  "movq   4096(%5,%%r11,8),%%xmm1\n"
  "movzb  0x1(%0),%%r11\n"
  "paddsw %%xmm1,%%xmm0\n"
  "movq   (%5,%%r10,8),%%xmm2\n"
  "add    $0x2,%0\n"
  "movq   (%5,%%r11,8),%%xmm3\n"
  "paddsw %%xmm0,%%xmm2\n"
  "paddsw %%xmm0,%%xmm3\n"
  "shufps $0x44,%%xmm3,%%xmm2\n"
  "psraw  $0x6,%%xmm2\n"
  "packuswb %%xmm2,%%xmm2\n"
  "movq   %%xmm2,0x0(%3)\n"
  "add    $0x8,%3\n"
"1:"
  "sub    $0x2,%4\n"
  "jns    0b\n"

"2:"
  "add    $0x1,%4\n"
  "js     3f\n"

  "movzb  (%1),%%r10\n"
  "movq   2048(%5,%%r10,8),%%xmm0\n"
  "movzb  (%2),%%r10\n"
  "movq   4096(%5,%%r10,8),%%xmm1\n"
  "paddsw %%xmm1,%%xmm0\n"
  "movzb  (%0),%%r10\n"
  "movq   (%5,%%r10,8),%%xmm1\n"
  "paddsw %%xmm0,%%xmm1\n"
  "psraw  $0x6,%%xmm1\n"
  "packuswb %%xmm1,%%xmm1\n"
  "movd   %%xmm1,0x0(%3)\n"
"3:"
  :
  : "r"(y_buf),  // %0
    "r"(u_buf),  // %1
    "r"(v_buf),  // %2
    "r"(rgb_buf),  // %3
    "r"(width),  // %4
    "r" (kCoefficientsRgbY)  // %5
  : "memory", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3"
);
}

void ScaleYUVToRGB32Row(const uint8* y_buf,  // rdi
                        const uint8* u_buf,  // rsi
                        const uint8* v_buf,  // rdx
                        uint8* rgb_buf,      // rcx
                        int width,           // r8
                        int source_dx) {     // r9
  asm(
  "xor    %%r11,%%r11\n"
  "sub    $0x2,%4\n"
  "js     1f\n"

"0:"
  "mov    %%r11,%%r10\n"
  "sar    $0x11,%%r10\n"
  "movzb  (%1,%%r10,1),%%rax\n"
  "movq   2048(%5,%%rax,8),%%xmm0\n"
  "movzb  (%2,%%r10,1),%%rax\n"
  "movq   4096(%5,%%rax,8),%%xmm1\n"
  "lea    (%%r11,%6),%%r10\n"
  "sar    $0x10,%%r11\n"
  "movzb  (%0,%%r11,1),%%rax\n"
  "paddsw %%xmm1,%%xmm0\n"
  "movq   (%5,%%rax,8),%%xmm1\n"
  "lea    (%%r10,%6),%%r11\n"
  "sar    $0x10,%%r10\n"
  "movzb  (%0,%%r10,1),%%rax\n"
  "movq   (%5,%%rax,8),%%xmm2\n"
  "paddsw %%xmm0,%%xmm1\n"
  "paddsw %%xmm0,%%xmm2\n"
  "shufps $0x44,%%xmm2,%%xmm1\n"
  "psraw  $0x6,%%xmm1\n"
  "packuswb %%xmm1,%%xmm1\n"
  "movq   %%xmm1,0x0(%3)\n"
  "add    $0x8,%3\n"
  "sub    $0x2,%4\n"
  "jns    0b\n"

"1:"
  "add    $0x1,%4\n"
  "js     2f\n"

  "mov    %%r11,%%r10\n"
  "sar    $0x11,%%r10\n"
  "movzb  (%1,%%r10,1),%%rax\n"
  "movq   2048(%5,%%rax,8),%%xmm0\n"
  "movzb  (%2,%%r10,1),%%rax\n"
  "movq   4096(%5,%%rax,8),%%xmm1\n"
  "paddsw %%xmm1,%%xmm0\n"
  "sar    $0x10,%%r11\n"
  "movzb  (%0,%%r11,1),%%rax\n"
  "movq   (%5,%%rax,8),%%xmm1\n"
  "paddsw %%xmm0,%%xmm1\n"
  "psraw  $0x6,%%xmm1\n"
  "packuswb %%xmm1,%%xmm1\n"
  "movd   %%xmm1,0x0(%3)\n"

"2:"
  :
  : "r"(y_buf),  // %0
    "r"(u_buf),  // %1
    "r"(v_buf),  // %2
    "r"(rgb_buf),  // %3
    "r"(width),  // %4
    "r" (kCoefficientsRgbY),  // %5
    "r"(static_cast<long>(source_dx))  // %6
  : "memory", "r10", "r11", "rax", "xmm0", "xmm1", "xmm2"
);
}

void LinearScaleYUVToRGB32Row(const uint8* y_buf,
                              const uint8* u_buf,
                              const uint8* v_buf,
                              uint8* rgb_buf,
                              int width,
                              int source_dx) {
  asm(
  "xor    %%r11,%%r11\n"   // x = 0
  "sub    $0x2,%4\n"
  "js     2f\n"
  "cmp    $0x20000,%6\n"   // if source_dx >= 2.0
  "jl     0f\n"
  "mov    $0x8000,%%r11\n" // x = 0.5 for 1/2 or less
"0:"

"1:"
  "mov    %%r11,%%r10\n"
  "sar    $0x11,%%r10\n"

  "movzb  (%1, %%r10, 1), %%r13 \n"
  "movzb  1(%1, %%r10, 1), %%r14 \n"
  "mov    %%r11, %%rax \n"
  "and    $0x1fffe, %%rax \n"
  "imul   %%rax, %%r14 \n"
  "xor    $0x1fffe, %%rax \n"
  "imul   %%rax, %%r13 \n"
  "add    %%r14, %%r13 \n"
  "shr    $17, %%r13 \n"
  "movq   2048(%5,%%r13,8), %%xmm0\n"

  "movzb  (%2, %%r10, 1), %%r13 \n"
  "movzb  1(%2, %%r10, 1), %%r14 \n"
  "mov    %%r11, %%rax \n"
  "and    $0x1fffe, %%rax \n"
  "imul   %%rax, %%r14 \n"
  "xor    $0x1fffe, %%rax \n"
  "imul   %%rax, %%r13 \n"
  "add    %%r14, %%r13 \n"
  "shr    $17, %%r13 \n"
  "movq   4096(%5,%%r13,8), %%xmm1\n"

  "mov    %%r11, %%rax \n"
  "lea    (%%r11,%6),%%r10\n"
  "sar    $0x10,%%r11\n"
  "paddsw %%xmm1,%%xmm0\n"

  "movzb  (%0, %%r11, 1), %%r13 \n"
  "movzb  1(%0, %%r11, 1), %%r14 \n"
  "and    $0xffff, %%rax \n"
  "imul   %%rax, %%r14 \n"
  "xor    $0xffff, %%rax \n"
  "imul   %%rax, %%r13 \n"
  "add    %%r14, %%r13 \n"
  "shr    $16, %%r13 \n"
  "movq   (%5,%%r13,8),%%xmm1\n"

  "mov    %%r10, %%rax \n"
  "lea    (%%r10,%6),%%r11\n"
  "sar    $0x10,%%r10\n"

  "movzb  (%0,%%r10,1), %%r13 \n"
  "movzb  1(%0,%%r10,1), %%r14 \n"
  "and    $0xffff, %%rax \n"
  "imul   %%rax, %%r14 \n"
  "xor    $0xffff, %%rax \n"
  "imul   %%rax, %%r13 \n"
  "add    %%r14, %%r13 \n"
  "shr    $16, %%r13 \n"
  "movq   (%5,%%r13,8),%%xmm2\n"

  "paddsw %%xmm0,%%xmm1\n"
  "paddsw %%xmm0,%%xmm2\n"
  "shufps $0x44,%%xmm2,%%xmm1\n"
  "psraw  $0x6,%%xmm1\n"
  "packuswb %%xmm1,%%xmm1\n"
  "movq   %%xmm1,0x0(%3)\n"
  "add    $0x8,%3\n"
  "sub    $0x2,%4\n"
  "jns    1b\n"

"2:"
  "add    $0x1,%4\n"
  "js     3f\n"

  "mov    %%r11,%%r10\n"
  "sar    $0x11,%%r10\n"

  "movzb  (%1,%%r10,1), %%r13 \n"
  "movq   2048(%5,%%r13,8),%%xmm0\n"

  "movzb  (%2,%%r10,1), %%r13 \n"
  "movq   4096(%5,%%r13,8),%%xmm1\n"

  "paddsw %%xmm1,%%xmm0\n"
  "sar    $0x10,%%r11\n"

  "movzb  (%0,%%r11,1), %%r13 \n"
  "movq   (%5,%%r13,8),%%xmm1\n"

  "paddsw %%xmm0,%%xmm1\n"
  "psraw  $0x6,%%xmm1\n"
  "packuswb %%xmm1,%%xmm1\n"
  "movd   %%xmm1,0x0(%3)\n"

"3:"
  :
  : "r"(y_buf),  // %0
    "r"(u_buf),  // %1
    "r"(v_buf),  // %2
    "r"(rgb_buf),  // %3
    "r"(width),  // %4
    "r" (kCoefficientsRgbY),  // %5
    "r"(static_cast<long>(source_dx))  // %6
  : "memory", "r10", "r11", "r13", "r14", "rax", "xmm0", "xmm1", "xmm2"
);
}

#elif defined(MOZILLA_MAY_SUPPORT_SSE) && defined(ARCH_CPU_X86_32) && !defined(__PIC__)

// PIC version is slower because less registers are available, so
// non-PIC is used on platforms where it is possible.
void FastConvertYUVToRGB32Row_SSE(const uint8* y_buf,
                                  const uint8* u_buf,
                                  const uint8* v_buf,
                                  uint8* rgb_buf,
                                  int width);
  asm(
  ".text\n"
  ".global FastConvertYUVToRGB32Row_SSE\n"
  ".type FastConvertYUVToRGB32Row_SSE, @function\n"
"FastConvertYUVToRGB32Row_SSE:\n"
  "pusha\n"
  "mov    0x24(%esp),%edx\n"
  "mov    0x28(%esp),%edi\n"
  "mov    0x2c(%esp),%esi\n"
  "mov    0x30(%esp),%ebp\n"
  "mov    0x34(%esp),%ecx\n"
  "jmp    1f\n"

"0:"
  "movzbl (%edi),%eax\n"
  "add    $0x1,%edi\n"
  "movzbl (%esi),%ebx\n"
  "add    $0x1,%esi\n"
  "movq   kCoefficientsRgbY+2048(,%eax,8),%mm0\n"
  "movzbl (%edx),%eax\n"
  "paddsw kCoefficientsRgbY+4096(,%ebx,8),%mm0\n"
  "movzbl 0x1(%edx),%ebx\n"
  "movq   kCoefficientsRgbY(,%eax,8),%mm1\n"
  "add    $0x2,%edx\n"
  "movq   kCoefficientsRgbY(,%ebx,8),%mm2\n"
  "paddsw %mm0,%mm1\n"
  "paddsw %mm0,%mm2\n"
  "psraw  $0x6,%mm1\n"
  "psraw  $0x6,%mm2\n"
  "packuswb %mm2,%mm1\n"
  "movntq %mm1,0x0(%ebp)\n"
  "add    $0x8,%ebp\n"
"1:"
  "sub    $0x2,%ecx\n"
  "jns    0b\n"

  "and    $0x1,%ecx\n"
  "je     2f\n"

  "movzbl (%edi),%eax\n"
  "movq   kCoefficientsRgbY+2048(,%eax,8),%mm0\n"
  "movzbl (%esi),%eax\n"
  "paddsw kCoefficientsRgbY+4096(,%eax,8),%mm0\n"
  "movzbl (%edx),%eax\n"
  "movq   kCoefficientsRgbY(,%eax,8),%mm1\n"
  "paddsw %mm0,%mm1\n"
  "psraw  $0x6,%mm1\n"
  "packuswb %mm1,%mm1\n"
  "movd   %mm1,0x0(%ebp)\n"
"2:"
  "popa\n"
  "ret\n"
#if !defined(XP_MACOSX)
  ".previous\n"
#endif
);

void FastConvertYUVToRGB32Row(const uint8* y_buf,
                              const uint8* u_buf,
                              const uint8* v_buf,
                              uint8* rgb_buf,
                              int width)
{
  if (mozilla::supports_sse()) {
    FastConvertYUVToRGB32Row_SSE(y_buf, u_buf, v_buf, rgb_buf, width);
    return;
  }

  FastConvertYUVToRGB32Row_C(y_buf, u_buf, v_buf, rgb_buf, width, 1);
}


void ScaleYUVToRGB32Row_SSE(const uint8* y_buf,
                            const uint8* u_buf,
                            const uint8* v_buf,
                            uint8* rgb_buf,
                            int width,
                            int source_dx);
  asm(
  ".text\n"
  ".global ScaleYUVToRGB32Row_SSE\n"
  ".type ScaleYUVToRGB32Row_SSE, @function\n"
"ScaleYUVToRGB32Row_SSE:\n"
  "pusha\n"
  "mov    0x24(%esp),%edx\n"
  "mov    0x28(%esp),%edi\n"
  "mov    0x2c(%esp),%esi\n"
  "mov    0x30(%esp),%ebp\n"
  "mov    0x34(%esp),%ecx\n"
  "xor    %ebx,%ebx\n"
  "jmp    1f\n"

"0:"
  "mov    %ebx,%eax\n"
  "sar    $0x11,%eax\n"
  "movzbl (%edi,%eax,1),%eax\n"
  "movq   kCoefficientsRgbY+2048(,%eax,8),%mm0\n"
  "mov    %ebx,%eax\n"
  "sar    $0x11,%eax\n"
  "movzbl (%esi,%eax,1),%eax\n"
  "paddsw kCoefficientsRgbY+4096(,%eax,8),%mm0\n"
  "mov    %ebx,%eax\n"
  "add    0x38(%esp),%ebx\n"
  "sar    $0x10,%eax\n"
  "movzbl (%edx,%eax,1),%eax\n"
  "movq   kCoefficientsRgbY(,%eax,8),%mm1\n"
  "mov    %ebx,%eax\n"
  "add    0x38(%esp),%ebx\n"
  "sar    $0x10,%eax\n"
  "movzbl (%edx,%eax,1),%eax\n"
  "movq   kCoefficientsRgbY(,%eax,8),%mm2\n"
  "paddsw %mm0,%mm1\n"
  "paddsw %mm0,%mm2\n"
  "psraw  $0x6,%mm1\n"
  "psraw  $0x6,%mm2\n"
  "packuswb %mm2,%mm1\n"
  "movntq %mm1,0x0(%ebp)\n"
  "add    $0x8,%ebp\n"
"1:"
  "sub    $0x2,%ecx\n"
  "jns    0b\n"

  "and    $0x1,%ecx\n"
  "je     2f\n"

  "mov    %ebx,%eax\n"
  "sar    $0x11,%eax\n"
  "movzbl (%edi,%eax,1),%eax\n"
  "movq   kCoefficientsRgbY+2048(,%eax,8),%mm0\n"
  "mov    %ebx,%eax\n"
  "sar    $0x11,%eax\n"
  "movzbl (%esi,%eax,1),%eax\n"
  "paddsw kCoefficientsRgbY+4096(,%eax,8),%mm0\n"
  "mov    %ebx,%eax\n"
  "sar    $0x10,%eax\n"
  "movzbl (%edx,%eax,1),%eax\n"
  "movq   kCoefficientsRgbY(,%eax,8),%mm1\n"
  "paddsw %mm0,%mm1\n"
  "psraw  $0x6,%mm1\n"
  "packuswb %mm1,%mm1\n"
  "movd   %mm1,0x0(%ebp)\n"

"2:"
  "popa\n"
  "ret\n"
#if !defined(XP_MACOSX)
  ".previous\n"
#endif
);

void ScaleYUVToRGB32Row(const uint8* y_buf,
                        const uint8* u_buf,
                        const uint8* v_buf,
                        uint8* rgb_buf,
                        int width,
                        int source_dx)
{
  if (mozilla::supports_sse()) {
    ScaleYUVToRGB32Row_SSE(y_buf, u_buf, v_buf, rgb_buf,
                           width, source_dx);
  }

  ScaleYUVToRGB32Row_C(y_buf, u_buf, v_buf, rgb_buf,
                       width, source_dx);
}

void LinearScaleYUVToRGB32Row_SSE(const uint8* y_buf,
                                  const uint8* u_buf,
                                  const uint8* v_buf,
                                  uint8* rgb_buf,
                                  int width,
                                  int source_dx);
  asm(
  ".text\n"
  ".global LinearScaleYUVToRGB32Row_SSE\n"
  ".type LinearScaleYUVToRGB32Row_SSE, @function\n"
"LinearScaleYUVToRGB32Row_SSE:\n"
  "pusha\n"
  "mov    0x24(%esp),%edx\n"
  "mov    0x28(%esp),%edi\n"
  "mov    0x30(%esp),%ebp\n"

  // source_width = width * source_dx + ebx
  "mov    0x34(%esp), %ecx\n"
  "imull  0x38(%esp), %ecx\n"
  "mov    %ecx, 0x34(%esp)\n"

  "mov    0x38(%esp), %ecx\n"
  "xor    %ebx,%ebx\n"     // x = 0
  "cmp    $0x20000,%ecx\n" // if source_dx >= 2.0
  "jl     1f\n"
  "mov    $0x8000,%ebx\n"  // x = 0.5 for 1/2 or less
  "jmp    1f\n"

"0:"
  "mov    %ebx,%eax\n"
  "sar    $0x11,%eax\n"

  "movzbl (%edi,%eax,1),%ecx\n"
  "movzbl 1(%edi,%eax,1),%esi\n"
  "mov    %ebx,%eax\n"
  "andl   $0x1fffe, %eax \n"
  "imul   %eax, %esi \n"
  "xorl   $0x1fffe, %eax \n"
  "imul   %eax, %ecx \n"
  "addl   %esi, %ecx \n"
  "shrl   $17, %ecx \n"
  "movq   kCoefficientsRgbY+2048(,%ecx,8),%mm0\n"

  "mov    0x2c(%esp),%esi\n"
  "mov    %ebx,%eax\n"
  "sar    $0x11,%eax\n"

  "movzbl (%esi,%eax,1),%ecx\n"
  "movzbl 1(%esi,%eax,1),%esi\n"
  "mov    %ebx,%eax\n"
  "andl   $0x1fffe, %eax \n"
  "imul   %eax, %esi \n"
  "xorl   $0x1fffe, %eax \n"
  "imul   %eax, %ecx \n"
  "addl   %esi, %ecx \n"
  "shrl   $17, %ecx \n"
  "paddsw kCoefficientsRgbY+4096(,%ecx,8),%mm0\n"

  "mov    %ebx,%eax\n"
  "sar    $0x10,%eax\n"
  "movzbl (%edx,%eax,1),%ecx\n"
  "movzbl 1(%edx,%eax,1),%esi\n"
  "mov    %ebx,%eax\n"
  "add    0x38(%esp),%ebx\n"
  "andl   $0xffff, %eax \n"
  "imul   %eax, %esi \n"
  "xorl   $0xffff, %eax \n"
  "imul   %eax, %ecx \n"
  "addl   %esi, %ecx \n"
  "shrl   $16, %ecx \n"
  "movq   kCoefficientsRgbY(,%ecx,8),%mm1\n"

  "cmp    0x34(%esp), %ebx\n"
  "jge    2f\n"

  "mov    %ebx,%eax\n"
  "sar    $0x10,%eax\n"
  "movzbl (%edx,%eax,1),%ecx\n"
  "movzbl 1(%edx,%eax,1),%esi\n"
  "mov    %ebx,%eax\n"
  "add    0x38(%esp),%ebx\n"
  "andl   $0xffff, %eax \n"
  "imul   %eax, %esi \n"
  "xorl   $0xffff, %eax \n"
  "imul   %eax, %ecx \n"
  "addl   %esi, %ecx \n"
  "shrl   $16, %ecx \n"
  "movq   kCoefficientsRgbY(,%ecx,8),%mm2\n"

  "paddsw %mm0,%mm1\n"
  "paddsw %mm0,%mm2\n"
  "psraw  $0x6,%mm1\n"
  "psraw  $0x6,%mm2\n"
  "packuswb %mm2,%mm1\n"
  "movntq %mm1,0x0(%ebp)\n"
  "add    $0x8,%ebp\n"

"1:"
  "cmp    0x34(%esp), %ebx\n"
  "jl     0b\n"
  "popa\n"
  "ret\n"

"2:"
  "paddsw %mm0, %mm1\n"
  "psraw $6, %mm1\n"
  "packuswb %mm1, %mm1\n"
  "movd %mm1, (%ebp)\n"
  "popa\n"
  "ret\n"
#if !defined(XP_MACOSX)
  ".previous\n"
#endif
);

void LinearScaleYUVToRGB32Row(const uint8* y_buf,
                              const uint8* u_buf,
                              const uint8* v_buf,
                              uint8* rgb_buf,
                              int width,
                              int source_dx)
{
  if (mozilla::supports_sse()) {
    LinearScaleYUVToRGB32Row_SSE(y_buf, u_buf, v_buf, rgb_buf,
                                 width, source_dx);
  }

  LinearScaleYUVToRGB32Row_C(y_buf, u_buf, v_buf, rgb_buf,
                             width, source_dx);
}

#elif defined(MOZILLA_MAY_SUPPORT_SSE) && defined(ARCH_CPU_X86_32) && defined(__PIC__)

void PICConvertYUVToRGB32Row_SSE(const uint8* y_buf,
                                 const uint8* u_buf,
                                 const uint8* v_buf,
                                 uint8* rgb_buf,
                                 int width,
                                 const int16 *kCoefficientsRgbY);

  asm(
  ".text\n"
#if defined(XP_MACOSX)
"_PICConvertYUVToRGB32Row_SSE:\n"
#else
"PICConvertYUVToRGB32Row_SSE:\n"
#endif
  "pusha\n"
  "mov    0x24(%esp),%edx\n"
  "mov    0x28(%esp),%edi\n"
  "mov    0x2c(%esp),%esi\n"
  "mov    0x30(%esp),%ebp\n"
  "mov    0x38(%esp),%ecx\n"

  "jmp    1f\n"

"0:"
  "movzbl (%edi),%eax\n"
  "add    $0x1,%edi\n"
  "movzbl (%esi),%ebx\n"
  "add    $0x1,%esi\n"
  "movq   2048(%ecx,%eax,8),%mm0\n"
  "movzbl (%edx),%eax\n"
  "paddsw 4096(%ecx,%ebx,8),%mm0\n"
  "movzbl 0x1(%edx),%ebx\n"
  "movq   0(%ecx,%eax,8),%mm1\n"
  "add    $0x2,%edx\n"
  "movq   0(%ecx,%ebx,8),%mm2\n"
  "paddsw %mm0,%mm1\n"
  "paddsw %mm0,%mm2\n"
  "psraw  $0x6,%mm1\n"
  "psraw  $0x6,%mm2\n"
  "packuswb %mm2,%mm1\n"
  "movntq %mm1,0x0(%ebp)\n"
  "add    $0x8,%ebp\n"
"1:"
  "subl   $0x2,0x34(%esp)\n"
  "jns    0b\n"

  "andl   $0x1,0x34(%esp)\n"
  "je     2f\n"

  "movzbl (%edi),%eax\n"
  "movq   2048(%ecx,%eax,8),%mm0\n"
  "movzbl (%esi),%eax\n"
  "paddsw 4096(%ecx,%eax,8),%mm0\n"
  "movzbl (%edx),%eax\n"
  "movq   0(%ecx,%eax,8),%mm1\n"
  "paddsw %mm0,%mm1\n"
  "psraw  $0x6,%mm1\n"
  "packuswb %mm1,%mm1\n"
  "movd   %mm1,0x0(%ebp)\n"
"2:"
  "popa\n"
  "ret\n"
#if !defined(XP_MACOSX)
  ".previous\n"
#endif
);

void FastConvertYUVToRGB32Row(const uint8* y_buf,
                              const uint8* u_buf,
                              const uint8* v_buf,
                              uint8* rgb_buf,
                              int width)
{
  if (mozilla::supports_sse()) {
    PICConvertYUVToRGB32Row_SSE(y_buf, u_buf, v_buf, rgb_buf, width,
                                &kCoefficientsRgbY[0][0]);
    return;
  }

  FastConvertYUVToRGB32Row_C(y_buf, u_buf, v_buf, rgb_buf, width, 1);
}

void PICScaleYUVToRGB32Row_SSE(const uint8* y_buf,
                               const uint8* u_buf,
                               const uint8* v_buf,
                               uint8* rgb_buf,
                               int width,
                               int source_dx,
                               const int16 *kCoefficientsRgbY);

  asm(
  ".text\n"
#if defined(XP_MACOSX)
"_PICScaleYUVToRGB32Row_SSE:\n"
#else
"PICScaleYUVToRGB32Row_SSE:\n"
#endif
  "pusha\n"
  "mov    0x24(%esp),%edx\n"
  "mov    0x28(%esp),%edi\n"
  "mov    0x2c(%esp),%esi\n"
  "mov    0x30(%esp),%ebp\n"
  "mov    0x3c(%esp),%ecx\n"
  "xor    %ebx,%ebx\n"
  "jmp    1f\n"

"0:"
  "mov    %ebx,%eax\n"
  "sar    $0x11,%eax\n"
  "movzbl (%edi,%eax,1),%eax\n"
  "movq   2048(%ecx,%eax,8),%mm0\n"
  "mov    %ebx,%eax\n"
  "sar    $0x11,%eax\n"
  "movzbl (%esi,%eax,1),%eax\n"
  "paddsw 4096(%ecx,%eax,8),%mm0\n"
  "mov    %ebx,%eax\n"
  "add    0x38(%esp),%ebx\n"
  "sar    $0x10,%eax\n"
  "movzbl (%edx,%eax,1),%eax\n"
  "movq   0(%ecx,%eax,8),%mm1\n"
  "mov    %ebx,%eax\n"
  "add    0x38(%esp),%ebx\n"
  "sar    $0x10,%eax\n"
  "movzbl (%edx,%eax,1),%eax\n"
  "movq   0(%ecx,%eax,8),%mm2\n"
  "paddsw %mm0,%mm1\n"
  "paddsw %mm0,%mm2\n"
  "psraw  $0x6,%mm1\n"
  "psraw  $0x6,%mm2\n"
  "packuswb %mm2,%mm1\n"
  "movntq %mm1,0x0(%ebp)\n"
  "add    $0x8,%ebp\n"
"1:"
  "subl   $0x2,0x34(%esp)\n"
  "jns    0b\n"

  "andl   $0x1,0x34(%esp)\n"
  "je     2f\n"

  "mov    %ebx,%eax\n"
  "sar    $0x11,%eax\n"
  "movzbl (%edi,%eax,1),%eax\n"
  "movq   2048(%ecx,%eax,8),%mm0\n"
  "mov    %ebx,%eax\n"
  "sar    $0x11,%eax\n"
  "movzbl (%esi,%eax,1),%eax\n"
  "paddsw 4096(%ecx,%eax,8),%mm0\n"
  "mov    %ebx,%eax\n"
  "sar    $0x10,%eax\n"
  "movzbl (%edx,%eax,1),%eax\n"
  "movq   0(%ecx,%eax,8),%mm1\n"
  "paddsw %mm0,%mm1\n"
  "psraw  $0x6,%mm1\n"
  "packuswb %mm1,%mm1\n"
  "movd   %mm1,0x0(%ebp)\n"

"2:"
  "popa\n"
  "ret\n"
#if !defined(XP_MACOSX)
  ".previous\n"
#endif
);

void ScaleYUVToRGB32Row(const uint8* y_buf,
                        const uint8* u_buf,
                        const uint8* v_buf,
                        uint8* rgb_buf,
                        int width,
                        int source_dx)
{
  if (mozilla::supports_sse()) {
    PICScaleYUVToRGB32Row_SSE(y_buf, u_buf, v_buf, rgb_buf, width, source_dx,
                              &kCoefficientsRgbY[0][0]);
    return;
  }

  ScaleYUVToRGB32Row_C(y_buf, u_buf, v_buf, rgb_buf, width, source_dx);
}

void PICLinearScaleYUVToRGB32Row_SSE(const uint8* y_buf,
                                     const uint8* u_buf,
                                     const uint8* v_buf,
                                     uint8* rgb_buf,
                                     int width,
                                     int source_dx,
                                     const int16 *kCoefficientsRgbY);

  asm(
  ".text\n"
#if defined(XP_MACOSX)
"_PICLinearScaleYUVToRGB32Row_SSE:\n"
#else
"PICLinearScaleYUVToRGB32Row_SSE:\n"
#endif
  "pusha\n"
  "mov    0x24(%esp),%edx\n"
  "mov    0x30(%esp),%ebp\n"
  "mov    0x34(%esp),%ecx\n"
  "mov    0x3c(%esp),%edi\n"
  "xor    %ebx,%ebx\n"

  // source_width = width * source_dx + ebx
  "mov    0x34(%esp), %ecx\n"
  "imull  0x38(%esp), %ecx\n"
  "mov    %ecx, 0x34(%esp)\n"

  "mov    0x38(%esp), %ecx\n"
  "xor    %ebx,%ebx\n"     // x = 0
  "cmp    $0x20000,%ecx\n" // if source_dx >= 2.0
  "jl     1f\n"
  "mov    $0x8000,%ebx\n"  // x = 0.5 for 1/2 or less
  "jmp    1f\n"

"0:"
  "mov    0x28(%esp),%esi\n"
  "mov    %ebx,%eax\n"
  "sar    $0x11,%eax\n"

  "movzbl (%esi,%eax,1),%ecx\n"
  "movzbl 1(%esi,%eax,1),%esi\n"
  "mov    %ebx,%eax\n"
  "andl   $0x1fffe, %eax \n"
  "imul   %eax, %esi \n"
  "xorl   $0x1fffe, %eax \n"
  "imul   %eax, %ecx \n"
  "addl   %esi, %ecx \n"
  "shrl   $17, %ecx \n"
  "movq   2048(%edi,%ecx,8),%mm0\n"

  "mov    0x2c(%esp),%esi\n"
  "mov    %ebx,%eax\n"
  "sar    $0x11,%eax\n"

  "movzbl (%esi,%eax,1),%ecx\n"
  "movzbl 1(%esi,%eax,1),%esi\n"
  "mov    %ebx,%eax\n"
  "andl   $0x1fffe, %eax \n"
  "imul   %eax, %esi \n"
  "xorl   $0x1fffe, %eax \n"
  "imul   %eax, %ecx \n"
  "addl   %esi, %ecx \n"
  "shrl   $17, %ecx \n"
  "paddsw 4096(%edi,%ecx,8),%mm0\n"

  "mov    %ebx,%eax\n"
  "sar    $0x10,%eax\n"
  "movzbl (%edx,%eax,1),%ecx\n"
  "movzbl 1(%edx,%eax,1),%esi\n"
  "mov    %ebx,%eax\n"
  "add    0x38(%esp),%ebx\n"
  "andl   $0xffff, %eax \n"
  "imul   %eax, %esi \n"
  "xorl   $0xffff, %eax \n"
  "imul   %eax, %ecx \n"
  "addl   %esi, %ecx \n"
  "shrl   $16, %ecx \n"
  "movq   (%edi,%ecx,8),%mm1\n"

  "cmp    0x34(%esp), %ebx\n"
  "jge    2f\n"

  "mov    %ebx,%eax\n"
  "sar    $0x10,%eax\n"
  "movzbl (%edx,%eax,1),%ecx\n"
  "movzbl 1(%edx,%eax,1),%esi\n"
  "mov    %ebx,%eax\n"
  "add    0x38(%esp),%ebx\n"
  "andl   $0xffff, %eax \n"
  "imul   %eax, %esi \n"
  "xorl   $0xffff, %eax \n"
  "imul   %eax, %ecx \n"
  "addl   %esi, %ecx \n"
  "shrl   $16, %ecx \n"
  "movq   (%edi,%ecx,8),%mm2\n"

  "paddsw %mm0,%mm1\n"
  "paddsw %mm0,%mm2\n"
  "psraw  $0x6,%mm1\n"
  "psraw  $0x6,%mm2\n"
  "packuswb %mm2,%mm1\n"
  "movntq %mm1,0x0(%ebp)\n"
  "add    $0x8,%ebp\n"

"1:"
  "cmp    %ebx, 0x34(%esp)\n"
  "jg     0b\n"
  "popa\n"
  "ret\n"

"2:"
  "paddsw %mm0, %mm1\n"
  "psraw $6, %mm1\n"
  "packuswb %mm1, %mm1\n"
  "movd %mm1, (%ebp)\n"
  "popa\n"
  "ret\n"
#if !defined(XP_MACOSX)
  ".previous\n"
#endif
);


void LinearScaleYUVToRGB32Row(const uint8* y_buf,
                              const uint8* u_buf,
                              const uint8* v_buf,
                              uint8* rgb_buf,
                              int width,
                              int source_dx)
{
  if (mozilla::supports_sse()) {
    PICLinearScaleYUVToRGB32Row_SSE(y_buf, u_buf, v_buf, rgb_buf, width,
                                    source_dx, &kCoefficientsRgbY[0][0]);
    return;
  }

  LinearScaleYUVToRGB32Row_C(y_buf, u_buf, v_buf, rgb_buf, width, source_dx);
}
#else
void FastConvertYUVToRGB32Row(const uint8* y_buf,
                              const uint8* u_buf,
                              const uint8* v_buf,
                              uint8* rgb_buf,
                              int width) {
  FastConvertYUVToRGB32Row_C(y_buf, u_buf, v_buf, rgb_buf, width, 1);
}

void ScaleYUVToRGB32Row(const uint8* y_buf,
                        const uint8* u_buf,
                        const uint8* v_buf,
                        uint8* rgb_buf,
                        int width,
                        int source_dx) {
  ScaleYUVToRGB32Row_C(y_buf, u_buf, v_buf, rgb_buf, width, source_dx);
}

void LinearScaleYUVToRGB32Row(const uint8* y_buf,
                              const uint8* u_buf,
                              const uint8* v_buf,
                              uint8* rgb_buf,
                              int width,
                              int source_dx) {
  LinearScaleYUVToRGB32Row_C(y_buf, u_buf, v_buf, rgb_buf, width, source_dx);
}
#endif

#if (defined(ARCH_CPU_MIPS64EL) || defined(ARCH_CPU_MIPS64EL)) && \
    defined(_MIPS_ARCH_LOONGSON3A)
void FastConvertYUVToRGB32Row_MMI(const uint8* y_buf,
                                  const uint8* u_buf,
                                  const uint8* v_buf,
                                  uint8* rgb_buf,
                                  int width,
                                  unsigned int x_shift) {
    __asm__ volatile (
        "li         $8, 0x006                                   \n\t"
        "mtc1       $8, $f6                                     \n\t"
        "j          1f                                          \n\t"
        "0:                                                     \n\t"
        "lbu        $8, 0x00(%[u_buf])                          \n\t"
        "daddiu     %[u_buf], %[u_buf], 0x01                    \n\t"
        "lbu        $9, 0x00(%[v_buf])                          \n\t"
        "daddiu     %[v_buf], %[v_buf], 0x01                    \n\t"
        "dsll       $8, 0x03                                    \n\t"
        "daddu      $8, $8, %[kCoefficientsRgbY]                \n\t"
        "daddiu     $8, $8, 2048                                \n\t"
        "gsldlc1    $f0, 0x07($8)                               \n\t"
        "gsldrc1    $f0, 0x00($8)                               \n\t"
        "lbu        $8, 0x00(%[y_buf])                          \n\t"
        "dsll       $9, 0x03                                    \n\t"
        "daddu      $9, $9, %[kCoefficientsRgbY]                \n\t"
        "daddiu     $9, $9, 4096                                \n\t"
        "gsldlc1    $f8, 0x07($9)                               \n\t"
        "gsldrc1    $f8, 0x00($9)                               \n\t"
        "paddsh     $f0, $f0, $f8                               \n\t"
        "lbu        $9, 0x01(%[y_buf])                          \n\t"
        "dsll       $8, 0x03                                    \n\t"
        "daddu      $8, $8, %[kCoefficientsRgbY]                \n\t"
        "gsldlc1    $f2, 0x07($8)                               \n\t"
        "gsldrc1    $f2, 0x00($8)                               \n\t"
        "daddiu     %[y_buf], %[y_buf], 0x02                    \n\t"
        "dsll       $9, 0x03                                    \n\t"
        "daddu      $9, $9, %[kCoefficientsRgbY]                \n\t"
        "gsldlc1    $f4, 0x07($9)                               \n\t"
        "gsldrc1    $f4, 0x00($9)                               \n\t"
        "paddsh     $f2, $f2, $f0                               \n\t"
        "paddsh     $f4, $f4, $f0                               \n\t"
        "psrah      $f2, $f2, $f6                               \n\t"
        "psrah      $f4, $f4, $f6                               \n\t"
        "packushb   $f2, $f2, $f4                               \n\t"
        "gssdlc1    $f2, 0x07(%[rgb_buf])                       \n\t"
        "gssdrc1    $f2, 0x00(%[rgb_buf])                       \n\t"
        "daddiu     %[rgb_buf], %[rgb_buf], 0x08                \n\t"
        "1:                                                     \n\t"
        "daddiu     %[width], %[width], -0x02                   \n\t"
        "bgez       %[width], 0b                                \n\t"
        "and        %[width], %[width], 0x01                    \n\t"
        "beqz       %[width], 2f                                \n\t"
        "lbu        $8, 0x00(%[u_buf])                          \n\t"
        "dsll       $8, 0x03                                    \n\t"
        "daddu      $8, $8, %[kCoefficientsRgbY]                \n\t"
        "daddiu     $8, $8, 0x800                               \n\t"
        "gsldlc1    $f0, 0x07($8)                               \n\t"
        "gsldrc1    $f0, 0x00($8)                               \n\t"
        "lbu        $8, 0x00(%[v_buf])                          \n\t"
        "dsll       $8, 0x03                                    \n\t"
        "daddu      $8, $8, %[kCoefficientsRgbY]                \n\t"
        "daddiu     $8, $8, 4096                                \n\t"
        "gsldlc1    $f8, 0x07($8)                               \n\t"
        "gsldrc1    $f8, 0x00($8)                               \n\t"
        "paddsh     $f0, $f0, $f8                               \n\t"
        "lbu        $8, 0x00(%[y_buf])                          \n\t"
        "dsll       $8, 0x03                                    \n\t"
        "daddu      $8, $8, %[kCoefficientsRgbY]                \n\t"
        "gsldlc1    $f2, 0x07($8)                               \n\t"
        "gsldrc1    $f2, 0x00($8)                               \n\t"
        "paddsh     $f2, $f2, $f0                               \n\t"
        "psrah      $f2, $f2, $f6                               \n\t"
        "packushb   $f2, $f2, $f2                               \n\t"
        "gsswlc1    $f2, 0x03(%[rgb_buf])                       \n\t"
        "gsswrc1    $f2, 0x00(%[rgb_buf])                       \n\t"
        "2:                                                     \n\t"
        : [y_buf]"+&r"((uint8 *)y_buf),     [u_buf]"+&r"((uint8 *)u_buf),
          [v_buf]"+&r"((uint8 *)v_buf),     [rgb_buf]"+&r"((uint8 *)rgb_buf),
          [width]"+&r"((int)width)
        : [kCoefficientsRgbY]"r"(kCoefficientsRgbY)
        : "memory","$8","$9","$f0","$f2","$f4","$f6","$f8"
    );
}
#endif  /* (defined(ARCH_CPU_MIPS64EL) || defined(ARCH_CPU_MIPS64EL)) && \
           defined(_MIPS_ARCH_LOONGSON3A) */

}
