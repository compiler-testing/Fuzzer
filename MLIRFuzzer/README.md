# Intall mlirfuzzer-opt tools

## Building

This setup assumes that you have built LLVM and MLIR in `$BUILD_DIR` and installed them to `$PREFIX`. To build and launch the tests, run
```sh
$ export BUILD_DIR=/path/compiler-testing/external/llvm/build
$ cd MLIRFuzzer
$ mkdir build && cd build
$ cmake -G Ninja .. \
    -DMLIR_DIR=$BUILD_DIR/lib/cmake/mlir \
    -DLLVM_EXTERNAL_LIT=$BUILD_DIR/bin/llvm-lit \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++
$ ninja
```
**Note**: Make sure to pass `-DLLVM_INSTALL_UTILS=ON` when building LLVM with CMake in order to install `FileCheck` to the chosen installation prefix.

