//===-- RISCV_DWARF_Registers.h ---------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_UTILITY_RISCV_DWARF_REGISTERS_H
#define LLDB_SOURCE_UTILITY_RISCV_DWARF_REGISTERS_H

#include "lldb/lldb-private.h"

namespace riscv_dwarf {

enum {
  dwarf_gpr_x0 = 0,
  dwarf_gpr_x1,
  dwarf_gpr_x2,
  dwarf_gpr_x3,
  dwarf_gpr_x4,
  dwarf_gpr_x5,
  dwarf_gpr_x6,
  dwarf_gpr_x7,
  dwarf_gpr_x8,
  dwarf_gpr_x9,
  dwarf_gpr_x10,
  dwarf_gpr_x11,
  dwarf_gpr_x12,
  dwarf_gpr_x13,
  dwarf_gpr_x14,
  dwarf_gpr_x15,
  dwarf_gpr_x16,
  dwarf_gpr_x17,
  dwarf_gpr_x18,
  dwarf_gpr_x19,
  dwarf_gpr_x20,
  dwarf_gpr_x21,
  dwarf_gpr_x22,
  dwarf_gpr_x23,
  dwarf_gpr_x24,
  dwarf_gpr_x25,
  dwarf_gpr_x26,
  dwarf_gpr_x27,
  dwarf_gpr_x28,
  dwarf_gpr_x29,
  dwarf_gpr_x30,
  dwarf_gpr_x31 = 31,

  dwarf_fpr_f0 = 32,
  dwarf_fpr_f1,
  dwarf_fpr_f2,
  dwarf_fpr_f3,
  dwarf_fpr_f4,
  dwarf_fpr_f5,
  dwarf_fpr_f6,
  dwarf_fpr_f7,
  dwarf_fpr_f8,
  dwarf_fpr_f9,
  dwarf_fpr_f10,
  dwarf_fpr_f11,
  dwarf_fpr_f12,
  dwarf_fpr_f13,
  dwarf_fpr_f14,
  dwarf_fpr_f15,
  dwarf_fpr_f16,
  dwarf_fpr_f17,
  dwarf_fpr_f18,
  dwarf_fpr_f19,
  dwarf_fpr_f20,
  dwarf_fpr_f21,
  dwarf_fpr_f22,
  dwarf_fpr_f23,
  dwarf_fpr_f24,
  dwarf_fpr_f25,
  dwarf_fpr_f26,
  dwarf_fpr_f27,
  dwarf_fpr_f28,
  dwarf_fpr_f29,
  dwarf_fpr_f30,
  dwarf_fpr_f31 = 63,

  // alternate frame return column
  dwarf_alt_fr_col = 64,

  dwarf_v0 = 96,
  dwarf_v1,
  dwarf_v2,
  dwarf_v3,
  dwarf_v4,
  dwarf_v5,
  dwarf_v6,
  dwarf_v7,
  dwarf_v8,
  dwarf_v9,
  dwarf_v10,
  dwarf_v11,
  dwarf_v12,
  dwarf_v13,
  dwarf_v14,
  dwarf_v15,
  dwarf_v16,
  dwarf_v17,
  dwarf_v18,
  dwarf_v19,
  dwarf_v20,
  dwarf_v21,
  dwarf_v22,
  dwarf_v23,
  dwarf_v24,
  dwarf_v25,
  dwarf_v26,
  dwarf_v27,
  dwarf_v28,
  dwarf_v29,
  dwarf_v30,
  dwarf_v31 = 127,
  dwarf_first_csr = 4096,
  dwarf_fpr_fcsr = dwarf_first_csr + 0x003,
  dwarf_last_csr = 8191,

  // register ABI name
  dwarf_gpr_zero = dwarf_gpr_x0,
  dwarf_gpr_ra = dwarf_gpr_x1,
  dwarf_gpr_sp = dwarf_gpr_x2,
  dwarf_gpr_gp = dwarf_gpr_x3,
  dwarf_gpr_tp = dwarf_gpr_x4,
  dwarf_gpr_t0 = dwarf_gpr_x5,
  dwarf_gpr_t1 = dwarf_gpr_x6,
  dwarf_gpr_t2 = dwarf_gpr_x7,
  dwarf_gpr_fp = dwarf_gpr_x8,
  dwarf_gpr_s1 = dwarf_gpr_x9,
  dwarf_gpr_a0 = dwarf_gpr_x10,
  dwarf_gpr_a1 = dwarf_gpr_x11,
  dwarf_gpr_a2 = dwarf_gpr_x12,
  dwarf_gpr_a3 = dwarf_gpr_x13,
  dwarf_gpr_a4 = dwarf_gpr_x14,
  dwarf_gpr_a5 = dwarf_gpr_x15,
  dwarf_gpr_a6 = dwarf_gpr_x16,
  dwarf_gpr_a7 = dwarf_gpr_x17,
  dwarf_gpr_s2 = dwarf_gpr_x18,
  dwarf_gpr_s3 = dwarf_gpr_x19,
  dwarf_gpr_s4 = dwarf_gpr_x20,
  dwarf_gpr_s5 = dwarf_gpr_x21,
  dwarf_gpr_s6 = dwarf_gpr_x22,
  dwarf_gpr_s7 = dwarf_gpr_x23,
  dwarf_gpr_s8 = dwarf_gpr_x24,
  dwarf_gpr_s9 = dwarf_gpr_x25,
  dwarf_gpr_s10 = dwarf_gpr_x26,
  dwarf_gpr_s11 = dwarf_gpr_x27,
  dwarf_gpr_t3 = dwarf_gpr_x28,
  dwarf_gpr_t4 = dwarf_gpr_x29,
  dwarf_gpr_t5 = dwarf_gpr_x30,
  dwarf_gpr_t6 = dwarf_gpr_x31,

  dwarf_fpr_ft0 = dwarf_fpr_f0,
  dwarf_fpr_ft1 = dwarf_fpr_f1,
  dwarf_fpr_ft2 = dwarf_fpr_f2,
  dwarf_fpr_ft3 = dwarf_fpr_f3,
  dwarf_fpr_ft4 = dwarf_fpr_f4,
  dwarf_fpr_ft5 = dwarf_fpr_f5,
  dwarf_fpr_ft6 = dwarf_fpr_f6,
  dwarf_fpr_ft7 = dwarf_fpr_f7,
  dwarf_fpr_fs0 = dwarf_fpr_f8,
  dwarf_fpr_fs1 = dwarf_fpr_f9,
  dwarf_fpr_fa0 = dwarf_fpr_f10,
  dwarf_fpr_fa1 = dwarf_fpr_f11,
  dwarf_fpr_fa2 = dwarf_fpr_f12,
  dwarf_fpr_fa3 = dwarf_fpr_f13,
  dwarf_fpr_fa4 = dwarf_fpr_f14,
  dwarf_fpr_fa5 = dwarf_fpr_f15,
  dwarf_fpr_fa6 = dwarf_fpr_f16,
  dwarf_fpr_fa7 = dwarf_fpr_f17,
  dwarf_fpr_fs2 = dwarf_fpr_f18,
  dwarf_fpr_fs3 = dwarf_fpr_f19,
  dwarf_fpr_fs4 = dwarf_fpr_f20,
  dwarf_fpr_fs5 = dwarf_fpr_f21,
  dwarf_fpr_fs6 = dwarf_fpr_f22,
  dwarf_fpr_fs7 = dwarf_fpr_f23,
  dwarf_fpr_fs8 = dwarf_fpr_f24,
  dwarf_fpr_fs9 = dwarf_fpr_f25,
  dwarf_fpr_fs10 = dwarf_fpr_f26,
  dwarf_fpr_fs11 = dwarf_fpr_f27,
  dwarf_fpr_ft8 = dwarf_fpr_f28,
  dwarf_fpr_ft9 = dwarf_fpr_f29,
  dwarf_fpr_ft10 = dwarf_fpr_f30,
  dwarf_fpr_ft11 = dwarf_fpr_f31,

  // mock pc regnum
  dwarf_gpr_pc = 11451,
};

} // namespace riscv_dwarf

#endif // LLDB_SOURCE_UTILITY_RISCV_DWARF_REGISTERS_H