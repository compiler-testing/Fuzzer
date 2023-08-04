# NOTE: Assertions have been autogenerated by utils/update_mca_test_checks.py
# RUN: llvm-mca -mtriple=aarch64-linux-gnu -mcpu=exynos-m3 -resource-pressure=false < %s | FileCheck %s -check-prefixes=ALL,M3
# RUN: llvm-mca -mtriple=aarch64-linux-gnu -mcpu=exynos-m4 -resource-pressure=false < %s | FileCheck %s -check-prefixes=ALL,M4
# RUN: llvm-mca -mtriple=aarch64-linux-gnu -mcpu=exynos-m5 -resource-pressure=false < %s | FileCheck %s -check-prefixes=ALL,M5

ld1	{v0.s}[0], [sp]
ld1r	{v0.2s}, [sp]
ld1	{v0.2s}, [sp]
ld1	{v0.2s, v1.2s}, [sp]
ld1	{v0.2s, v1.2s, v2.2s}, [sp]
ld1	{v0.2s, v1.2s, v2.2s, v3.2s}, [sp]

ld1	{v0.d}[0], [sp]
ld1r	{v0.2d}, [sp]
ld1	{v0.2d}, [sp]
ld1	{v0.2d, v1.2d}, [sp]
ld1	{v0.2d, v1.2d, v2.2d}, [sp]
ld1	{v0.2d, v1.2d, v2.2d, v3.2d}, [sp]

ld1	{v0.s}[0], [sp], #4
ld1r	{v0.2s}, [sp], #4
ld1	{v0.2s}, [sp], #8
ld1	{v0.2s, v1.2s}, [sp], #16
ld1	{v0.2s, v1.2s, v2.2s}, [sp], #24
ld1	{v0.2s, v1.2s, v2.2s, v3.2s}, [sp], #32

ld1	{v0.d}[0], [sp], #8
ld1r	{v0.2d}, [sp], #8
ld1	{v0.2d}, [sp], #16
ld1	{v0.2d, v1.2d}, [sp], #32
ld1	{v0.2d, v1.2d, v2.2d}, [sp], #48
ld1	{v0.2d, v1.2d, v2.2d, v3.2d}, [sp], #64

ld1	{v0.s}[0], [sp], x0
ld1r	{v0.2s}, [sp], x0
ld1	{v0.2s}, [sp], x0
ld1	{v0.2s, v1.2s}, [sp], x0
ld1	{v0.2s, v1.2s, v2.2s}, [sp], x0
ld1	{v0.2s, v1.2s, v2.2s, v3.2s}, [sp], x0

ld1	{v0.d}[0], [sp], x0
ld1r	{v0.2d}, [sp], x0
ld1	{v0.2d}, [sp], x0
ld1	{v0.2d, v1.2d}, [sp], x0
ld1	{v0.2d, v1.2d, v2.2d}, [sp], x0
ld1	{v0.2d, v1.2d, v2.2d, v3.2d}, [sp], x0

# ALL:      Iterations:        100
# ALL-NEXT: Instructions:      3600

# M3-NEXT:  Total Cycles:      14903
# M4-NEXT:  Total Cycles:      14703
# M5-NEXT:  Total Cycles:      17203

# ALL-NEXT: Total uOps:        10200

# ALL:      Dispatch Width:    6

# M3-NEXT:  uOps Per Cycle:    0.68
# M3-NEXT:  IPC:               0.24

# M4-NEXT:  uOps Per Cycle:    0.69
# M4-NEXT:  IPC:               0.24

# M5-NEXT:  uOps Per Cycle:    0.59
# M5-NEXT:  IPC:               0.21

# ALL-NEXT: Block RThroughput: 39.0

# ALL:      Instruction Info:
# ALL-NEXT: [1]: #uOps
# ALL-NEXT: [2]: Latency
# ALL-NEXT: [3]: RThroughput
# ALL-NEXT: [4]: MayLoad
# ALL-NEXT: [5]: MayStore
# ALL-NEXT: [6]: HasSideEffects (U)

# ALL:      [1]    [2]    [3]    [4]    [5]    [6]    Instructions:

# M3-NEXT:   2      7     1.00    *                   ld1	{ v0.s }[0], [sp]
# M3-NEXT:   1      5     0.50    *                   ld1r	{ v0.2s }, [sp]
# M3-NEXT:   1      5     0.50    *                   ld1	{ v0.2s }, [sp]
# M3-NEXT:   2      5     1.00    *                   ld1	{ v0.2s, v1.2s }, [sp]
# M3-NEXT:   3      6     1.50    *                   ld1	{ v0.2s, v1.2s, v2.2s }, [sp]
# M3-NEXT:   4      6     2.00    *                   ld1	{ v0.2s, v1.2s, v2.2s, v3.2s }, [sp]
# M3-NEXT:   2      6     1.00    *                   ld1	{ v0.d }[0], [sp]
# M3-NEXT:   1      5     0.50    *                   ld1r	{ v0.2d }, [sp]
# M3-NEXT:   1      5     0.50    *                   ld1	{ v0.2d }, [sp]
# M3-NEXT:   2      5     1.00    *                   ld1	{ v0.2d, v1.2d }, [sp]
# M3-NEXT:   3      6     1.50    *                   ld1	{ v0.2d, v1.2d, v2.2d }, [sp]
# M3-NEXT:   4      6     2.00    *                   ld1	{ v0.2d, v1.2d, v2.2d, v3.2d }, [sp]
# M3-NEXT:   3      7     1.00    *                   ld1	{ v0.s }[0], [sp], #4
# M3-NEXT:   2      5     0.50    *                   ld1r	{ v0.2s }, [sp], #4
# M3-NEXT:   2      5     0.50    *                   ld1	{ v0.2s }, [sp], #8
# M3-NEXT:   3      5     1.00    *                   ld1	{ v0.2s, v1.2s }, [sp], #16
# M3-NEXT:   4      6     1.50    *                   ld1	{ v0.2s, v1.2s, v2.2s }, [sp], #24
# M3-NEXT:   5      6     2.00    *                   ld1	{ v0.2s, v1.2s, v2.2s, v3.2s }, [sp], #32
# M3-NEXT:   3      6     1.00    *                   ld1	{ v0.d }[0], [sp], #8
# M3-NEXT:   2      5     0.50    *                   ld1r	{ v0.2d }, [sp], #8
# M3-NEXT:   2      5     0.50    *                   ld1	{ v0.2d }, [sp], #16
# M3-NEXT:   3      5     1.00    *                   ld1	{ v0.2d, v1.2d }, [sp], #32
# M3-NEXT:   4      6     1.50    *                   ld1	{ v0.2d, v1.2d, v2.2d }, [sp], #48
# M3-NEXT:   5      6     2.00    *                   ld1	{ v0.2d, v1.2d, v2.2d, v3.2d }, [sp], #64
# M3-NEXT:   3      7     1.00    *                   ld1	{ v0.s }[0], [sp], x0
# M3-NEXT:   2      5     0.50    *                   ld1r	{ v0.2s }, [sp], x0
# M3-NEXT:   2      5     0.50    *                   ld1	{ v0.2s }, [sp], x0
# M3-NEXT:   3      5     1.00    *                   ld1	{ v0.2s, v1.2s }, [sp], x0
# M3-NEXT:   4      6     1.50    *                   ld1	{ v0.2s, v1.2s, v2.2s }, [sp], x0
# M3-NEXT:   5      6     2.00    *                   ld1	{ v0.2s, v1.2s, v2.2s, v3.2s }, [sp], x0
# M3-NEXT:   3      6     1.00    *                   ld1	{ v0.d }[0], [sp], x0
# M3-NEXT:   2      5     0.50    *                   ld1r	{ v0.2d }, [sp], x0
# M3-NEXT:   2      5     0.50    *                   ld1	{ v0.2d }, [sp], x0
# M3-NEXT:   3      5     1.00    *                   ld1	{ v0.2d, v1.2d }, [sp], x0
# M3-NEXT:   4      6     1.50    *                   ld1	{ v0.2d, v1.2d, v2.2d }, [sp], x0
# M3-NEXT:   5      6     2.00    *                   ld1	{ v0.2d, v1.2d, v2.2d, v3.2d }, [sp], x0

# M4-NEXT:   2      6     1.00    *                   ld1	{ v0.s }[0], [sp]
# M4-NEXT:   1      5     0.50    *                   ld1r	{ v0.2s }, [sp]
# M4-NEXT:   1      5     0.50    *                   ld1	{ v0.2s }, [sp]
# M4-NEXT:   2      5     1.00    *                   ld1	{ v0.2s, v1.2s }, [sp]
# M4-NEXT:   3      6     1.50    *                   ld1	{ v0.2s, v1.2s, v2.2s }, [sp]
# M4-NEXT:   4      6     2.00    *                   ld1	{ v0.2s, v1.2s, v2.2s, v3.2s }, [sp]
# M4-NEXT:   2      6     1.00    *                   ld1	{ v0.d }[0], [sp]
# M4-NEXT:   1      5     0.50    *                   ld1r	{ v0.2d }, [sp]
# M4-NEXT:   1      5     0.50    *                   ld1	{ v0.2d }, [sp]
# M4-NEXT:   2      5     1.00    *                   ld1	{ v0.2d, v1.2d }, [sp]
# M4-NEXT:   3      6     1.50    *                   ld1	{ v0.2d, v1.2d, v2.2d }, [sp]
# M4-NEXT:   4      6     2.00    *                   ld1	{ v0.2d, v1.2d, v2.2d, v3.2d }, [sp]
# M4-NEXT:   3      6     1.00    *                   ld1	{ v0.s }[0], [sp], #4
# M4-NEXT:   2      5     0.50    *                   ld1r	{ v0.2s }, [sp], #4
# M4-NEXT:   2      5     0.50    *                   ld1	{ v0.2s }, [sp], #8
# M4-NEXT:   3      5     1.00    *                   ld1	{ v0.2s, v1.2s }, [sp], #16
# M4-NEXT:   4      6     1.50    *                   ld1	{ v0.2s, v1.2s, v2.2s }, [sp], #24
# M4-NEXT:   5      6     2.00    *                   ld1	{ v0.2s, v1.2s, v2.2s, v3.2s }, [sp], #32
# M4-NEXT:   3      6     1.00    *                   ld1	{ v0.d }[0], [sp], #8
# M4-NEXT:   2      5     0.50    *                   ld1r	{ v0.2d }, [sp], #8
# M4-NEXT:   2      5     0.50    *                   ld1	{ v0.2d }, [sp], #16
# M4-NEXT:   3      5     1.00    *                   ld1	{ v0.2d, v1.2d }, [sp], #32
# M4-NEXT:   4      6     1.50    *                   ld1	{ v0.2d, v1.2d, v2.2d }, [sp], #48
# M4-NEXT:   5      6     2.00    *                   ld1	{ v0.2d, v1.2d, v2.2d, v3.2d }, [sp], #64
# M4-NEXT:   3      6     1.00    *                   ld1	{ v0.s }[0], [sp], x0
# M4-NEXT:   2      5     0.50    *                   ld1r	{ v0.2s }, [sp], x0
# M4-NEXT:   2      5     0.50    *                   ld1	{ v0.2s }, [sp], x0
# M4-NEXT:   3      5     1.00    *                   ld1	{ v0.2s, v1.2s }, [sp], x0
# M4-NEXT:   4      6     1.50    *                   ld1	{ v0.2s, v1.2s, v2.2s }, [sp], x0
# M4-NEXT:   5      6     2.00    *                   ld1	{ v0.2s, v1.2s, v2.2s, v3.2s }, [sp], x0
# M4-NEXT:   3      6     1.00    *                   ld1	{ v0.d }[0], [sp], x0
# M4-NEXT:   2      5     0.50    *                   ld1r	{ v0.2d }, [sp], x0
# M4-NEXT:   2      5     0.50    *                   ld1	{ v0.2d }, [sp], x0
# M4-NEXT:   3      5     1.00    *                   ld1	{ v0.2d, v1.2d }, [sp], x0
# M4-NEXT:   4      6     1.50    *                   ld1	{ v0.2d, v1.2d, v2.2d }, [sp], x0
# M4-NEXT:   5      6     2.00    *                   ld1	{ v0.2d, v1.2d, v2.2d, v3.2d }, [sp], x0

# M5-NEXT:   2      7     1.00    *                   ld1	{ v0.s }[0], [sp]
# M5-NEXT:   1      6     0.50    *                   ld1r	{ v0.2s }, [sp]
# M5-NEXT:   1      6     0.50    *                   ld1	{ v0.2s }, [sp]
# M5-NEXT:   2      6     1.00    *                   ld1	{ v0.2s, v1.2s }, [sp]
# M5-NEXT:   3      7     1.50    *                   ld1	{ v0.2s, v1.2s, v2.2s }, [sp]
# M5-NEXT:   4      7     2.00    *                   ld1	{ v0.2s, v1.2s, v2.2s, v3.2s }, [sp]
# M5-NEXT:   2      7     1.00    *                   ld1	{ v0.d }[0], [sp]
# M5-NEXT:   1      6     0.50    *                   ld1r	{ v0.2d }, [sp]
# M5-NEXT:   1      6     0.50    *                   ld1	{ v0.2d }, [sp]
# M5-NEXT:   2      6     1.00    *                   ld1	{ v0.2d, v1.2d }, [sp]
# M5-NEXT:   3      7     1.50    *                   ld1	{ v0.2d, v1.2d, v2.2d }, [sp]
# M5-NEXT:   4      7     2.00    *                   ld1	{ v0.2d, v1.2d, v2.2d, v3.2d }, [sp]
# M5-NEXT:   3      7     1.00    *                   ld1	{ v0.s }[0], [sp], #4
# M5-NEXT:   2      6     0.50    *                   ld1r	{ v0.2s }, [sp], #4
# M5-NEXT:   2      6     0.50    *                   ld1	{ v0.2s }, [sp], #8
# M5-NEXT:   3      6     1.00    *                   ld1	{ v0.2s, v1.2s }, [sp], #16
# M5-NEXT:   4      7     1.50    *                   ld1	{ v0.2s, v1.2s, v2.2s }, [sp], #24
# M5-NEXT:   5      7     2.00    *                   ld1	{ v0.2s, v1.2s, v2.2s, v3.2s }, [sp], #32
# M5-NEXT:   3      7     1.00    *                   ld1	{ v0.d }[0], [sp], #8
# M5-NEXT:   2      6     0.50    *                   ld1r	{ v0.2d }, [sp], #8
# M5-NEXT:   2      6     0.50    *                   ld1	{ v0.2d }, [sp], #16
# M5-NEXT:   3      6     1.00    *                   ld1	{ v0.2d, v1.2d }, [sp], #32
# M5-NEXT:   4      7     1.50    *                   ld1	{ v0.2d, v1.2d, v2.2d }, [sp], #48
# M5-NEXT:   5      7     2.00    *                   ld1	{ v0.2d, v1.2d, v2.2d, v3.2d }, [sp], #64
# M5-NEXT:   3      7     1.00    *                   ld1	{ v0.s }[0], [sp], x0
# M5-NEXT:   2      6     0.50    *                   ld1r	{ v0.2s }, [sp], x0
# M5-NEXT:   2      6     0.50    *                   ld1	{ v0.2s }, [sp], x0
# M5-NEXT:   3      6     1.00    *                   ld1	{ v0.2s, v1.2s }, [sp], x0
# M5-NEXT:   4      7     1.50    *                   ld1	{ v0.2s, v1.2s, v2.2s }, [sp], x0
# M5-NEXT:   5      7     2.00    *                   ld1	{ v0.2s, v1.2s, v2.2s, v3.2s }, [sp], x0
# M5-NEXT:   3      7     1.00    *                   ld1	{ v0.d }[0], [sp], x0
# M5-NEXT:   2      6     0.50    *                   ld1r	{ v0.2d }, [sp], x0
# M5-NEXT:   2      6     0.50    *                   ld1	{ v0.2d }, [sp], x0
# M5-NEXT:   3      6     1.00    *                   ld1	{ v0.2d, v1.2d }, [sp], x0
# M5-NEXT:   4      7     1.50    *                   ld1	{ v0.2d, v1.2d, v2.2d }, [sp], x0
# M5-NEXT:   5      7     2.00    *                   ld1	{ v0.2d, v1.2d, v2.2d, v3.2d }, [sp], x0