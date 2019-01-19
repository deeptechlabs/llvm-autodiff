; RUN: opt -mtriple=x86_64-- -codegenprepare                        %s -S -o - | FileCheck %s --check-prefixes=CGP,COMMON
; RUN: opt -mtriple=x86_64-- -codegenprepare -load-store-vectorizer %s -S -o - | FileCheck %s --check-prefixes=LSV,COMMON

; Make sure CodeGenPrepare doesn't emit multiple inttoptr instructions
; of the same integer value while sinking address computations, but
; rather CSEs them on the fly: excessive inttoptr's confuse SCEV
; into thinking that related pointers have nothing to do with each other.
;
; Triggering this problem involves having just right addressing modes,
; and verifying that the motivating pass (LoadStoreVectorizer) is able
; to benefit from it - just right LSV-policies. Hence the atypical combination
; of the target and datalayout / address spaces in this test.

target datalayout = "p1:32:32:32"

define void @test1(i32 %tmp, i32 %off) {
; COMMON-LABEL: @test1
; CGP:     = inttoptr
; CGP-NOT: = inttoptr
; LSV:     = load <2 x float>
; LSV:     = load <2 x float>
entry:
  %tmp1 = inttoptr i32 %tmp to float addrspace(1)*
  %arrayidx.i.7 = getelementptr inbounds float, float addrspace(1)* %tmp1, i32 %off
  %add20.i.7 = add i32 %off, 1
  %arrayidx22.i.7 = getelementptr inbounds float, float addrspace(1)* %tmp1, i32 %add20.i.7
  br label %for.body

for.body:
  %tmp8 = phi float [ undef, %entry ], [ %tmp62, %for.body ]
  %tmp28 = load float, float addrspace(1)* %arrayidx.i.7
  %tmp29 = load float, float addrspace(1)* %arrayidx22.i.7
  %arrayidx.i321.7 = getelementptr inbounds float, float addrspace(1)* %tmp1, i32 0
  %tmp43 = load float, float addrspace(1)* %arrayidx.i321.7
  %arrayidx22.i327.7 = getelementptr inbounds float, float addrspace(1)* %tmp1, i32 1
  %tmp44 = load float, float addrspace(1)* %arrayidx22.i327.7
  %tmp62 = tail call fast float @foo(float %tmp8, float %tmp44, float %tmp43, float %tmp29, float %tmp28)
  br label %for.body
}

define void @test2(i64 %a, i64 %b, i64 %c) {
; COMMON-LABEL: @test2
; CGP:    loop:
; CGP-NEXT: %mul =
; CGP-NEXT: = inttoptr i64 %mul
; CGP-NOT:  = inttoptr
; LSV:      store <2 x i64>
entry:
  %mul.neg.i630 = add nsw i64 %a, -16
  br label %loop

loop:
  %mul = mul nsw i64 %b, -16
  %sub.i631 = add nsw i64 %mul.neg.i630, %mul
  %tmp = inttoptr i64 %sub.i631 to i8*
  %tmp1 = inttoptr i64 %sub.i631 to i64*
  store i64 %c, i64* %tmp1, align 16
  %arrayidx172 = getelementptr inbounds i8, i8* %tmp, i64 8
  %tmp2 = bitcast i8* %arrayidx172 to i64*
  store i64 42, i64* %tmp2, align 8
  br label %loop
}

declare float @foo(float, float, float, float, float)
