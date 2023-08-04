; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+amx-tile,+amx-int8,+amx-fp16,+avx512f -verify-machineinstrs | FileCheck %s

define void @test_amx(ptr %pointer, ptr %base, i64 %stride) {
; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vxorps %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    vmovups %zmm0, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movb $1, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movb $8, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movw $8, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movb $8, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movw $8, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movb $8, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movw $8, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    ldtilecfg -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movw $8, %ax
; CHECK-NEXT:    tileloadd (%rsi,%rdx), %tmm0
; CHECK-NEXT:    tileloadd (%rsi,%rdx), %tmm1
; CHECK-NEXT:    tilezero %tmm2
; CHECK-NEXT:    tdpfp16ps %tmm1, %tmm0, %tmm2
; CHECK-NEXT:    tileloaddt1 (%rsi,%rdx), %tmm0
; CHECK-NEXT:    tilestored %tmm2, (%rdi,%rdx)
; CHECK-NEXT:    tilerelease
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retq
  %a = call x86_amx @llvm.x86.tileloadd64.internal(i16 8, i16 8, ptr %base, i64 %stride)
  %b = call x86_amx @llvm.x86.tileloadd64.internal(i16 8, i16 8, ptr %base, i64 %stride)
  %c = call x86_amx @llvm.x86.tilezero.internal(i16 8, i16 8)
  %d = call x86_amx @llvm.x86.tdpfp16ps.internal(i16 8, i16 8, i16 8, x86_amx %c, x86_amx %a, x86_amx %b)
  %e = call x86_amx @llvm.x86.tileloaddt164.internal(i16 8, i16 8, ptr %base, i64 %stride)
  call void @llvm.x86.tilestored64.internal(i16 8, i16 8, ptr %pointer, i64 %stride, x86_amx %d)

  ret void
}

declare x86_amx @llvm.x86.tilezero.internal(i16, i16)
declare x86_amx @llvm.x86.tileloadd64.internal(i16, i16, ptr, i64)
declare x86_amx @llvm.x86.tileloaddt164.internal(i16, i16, ptr, i64)
declare x86_amx @llvm.x86.tdpfp16ps.internal(i16, i16, i16, x86_amx, x86_amx, x86_amx)
declare void @llvm.x86.tilestored64.internal(i16, i16, ptr, i64, x86_amx)