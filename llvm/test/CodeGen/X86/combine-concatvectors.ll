; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=x86_64-unknown-unknown -mattr=+avx < %s | FileCheck %s

define void @PR32957(<2 x float>* %in, <8 x float>* %out) {
; CHECK-LABEL: PR32957:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vmovsd {{.*#+}} xmm0 = mem[0],zero
; CHECK-NEXT:    vxorps %xmm1, %xmm1, %xmm1
; CHECK-NEXT:    vblendps {{.*#+}} xmm0 = xmm0[0,1],xmm1[2,3]
; CHECK-NEXT:    vmovaps %ymm0, (%rsi)
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retq
  %ld = load <2 x float>, <2 x float>* %in, align 8
  %ext = extractelement <2 x float> %ld, i64 0
  %ext2 = extractelement <2 x float> %ld, i64 1
  %ins = insertelement <8 x float> <float undef, float undef, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0, float 0.0>, float %ext, i64 0
  %ins2 = insertelement <8 x float> %ins, float %ext2, i64 1
  store <8 x float> %ins2, <8 x float>* %out, align 32
  ret void
}
