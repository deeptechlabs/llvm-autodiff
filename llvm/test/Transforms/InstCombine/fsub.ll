; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -instcombine -S | FileCheck %s

; PR4374

define float @test1(float %a, float %b) {
; CHECK-LABEL: @test1(
; CHECK-NEXT:    [[T1:%.*]] = fsub float [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    [[T2:%.*]] = fsub float -0.000000e+00, [[T1]]
; CHECK-NEXT:    ret float [[T2]]
;
  %t1 = fsub float %a, %b
  %t2 = fsub float -0.0, %t1
  ret float %t2
}

; <rdar://problem/7530098>

define double @test2(double %x, double %y) {
; CHECK-LABEL: @test2(
; CHECK-NEXT:    [[T1:%.*]] = fadd double [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[T2:%.*]] = fsub double [[X]], [[T1]]
; CHECK-NEXT:    ret double [[T2]]
;
  %t1 = fadd double %x, %y
  %t2 = fsub double %x, %t1
  ret double %t2
}

; X - C --> X + (-C)

define float @constant_op1(float %x, float %y) {
; CHECK-LABEL: @constant_op1(
; CHECK-NEXT:    [[R:%.*]] = fadd float [[X:%.*]], -4.200000e+01
; CHECK-NEXT:    ret float [[R]]
;
  %r = fsub float %x, 42.0
  ret float %r
}

define <2 x float> @constant_op1_vec(<2 x float> %x, <2 x float> %y) {
; CHECK-LABEL: @constant_op1_vec(
; CHECK-NEXT:    [[R:%.*]] = fadd <2 x float> [[X:%.*]], <float -4.200000e+01, float 4.200000e+01>
; CHECK-NEXT:    ret <2 x float> [[R]]
;
  %r = fsub <2 x float> %x, <float 42.0, float -42.0>
  ret <2 x float> %r
}

define <2 x float> @constant_op1_vec_undef(<2 x float> %x, <2 x float> %y) {
; CHECK-LABEL: @constant_op1_vec_undef(
; CHECK-NEXT:    [[R:%.*]] = fadd <2 x float> [[X:%.*]], <float 0x7FF8000000000000, float 4.200000e+01>
; CHECK-NEXT:    ret <2 x float> [[R]]
;
  %r = fsub <2 x float> %x, <float undef, float -42.0>
  ret <2 x float> %r
}

; X - (-Y) --> X + Y

define float @neg_op1(float %x, float %y) {
; CHECK-LABEL: @neg_op1(
; CHECK-NEXT:    [[R:%.*]] = fadd float [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    ret float [[R]]
;
  %negy = fsub float -0.0, %y
  %r = fsub float %x, %negy
  ret float %r
}

define <2 x float> @neg_op1_vec(<2 x float> %x, <2 x float> %y) {
; CHECK-LABEL: @neg_op1_vec(
; CHECK-NEXT:    [[R:%.*]] = fadd <2 x float> [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    ret <2 x float> [[R]]
;
  %negy = fsub <2 x float> <float -0.0, float -0.0>, %y
  %r = fsub <2 x float> %x, %negy
  ret <2 x float> %r
}

define <2 x float> @neg_op1_vec_undef(<2 x float> %x, <2 x float> %y) {
; CHECK-LABEL: @neg_op1_vec_undef(
; CHECK-NEXT:    [[R:%.*]] = fadd <2 x float> [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    ret <2 x float> [[R]]
;
  %negy = fsub <2 x float> <float -0.0, float undef>, %y
  %r = fsub <2 x float> %x, %negy
  ret <2 x float> %r
}

