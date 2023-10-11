## Setup
This section gives the steps, explanations and examples for getting the project running.

### 1. Clone this repo
```
git clone https://github.com/compiler-testing/Fuzzer.git
```

init submodules
```
git submodule update --init
```

### 2. Building
#### 1) Build dependency LLVM
Please refer to the [LLVM Getting Started](https://llvm.org/docs/GettingStarted.html) in general to build LLVM. Below are quick instructions to build MLIR with LLVM.

The following instructions for compiling and testing MLIR assume that you have git, [ninja](https://ninja-build.org/), and a working C++ toolchain (see [LLVM requirements](https://llvm.org/docs/GettingStarted.html#requirements)).

```
$ cd external
$ mkdir llvm/build
$ cd llvm/build
$ cmake -G Ninja ../llvm \
    -DLLVM_ENABLE_PROJECTS=mlir \
    -DLLVM_TARGETS_TO_BUILD="X86" \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_ASSERTIONS=ON \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ 
$ ninja
```
#### 2) Build MLIRFuzzer
[MLIRFuzzer](https://github.com/compiler-testing/Fuzzer/tree/master/MLIRFuzzer) contains two components: Tosa graph generation and IR mutation. Please run the steps in the [README.md](https://github.com/compiler-testing/Fuzzer/blob/master/MLIRFuzzer/README.md) to build them.

### 3. Run the testcase

- generate tosa graph
```
cd fuzz_tool
bash generator.sh
```
- run fuzzing loop

```
cd fuzz_tool
bash run_fuzz.sh
```

- vscode debug  调试代码请按如下配置
    - 修改/compiler-testing/fuzz_tool/conf/conf.yml文件中的项目路径project_path 
    - 修改/compiler-testing/fuzz_tool/src/main.py中line 47的配置文件路径config_path


- debug 复现result_table中的一条记录
    - 实现：Fuzz.debug
    - 配置：设置你要重现的result_id
    - Run或者Debug: fuzz_tool/run_fuzz.sh  
    ```
    python3 ./src/main.py --opt=fuzz  --sqlName=MLIRFuzz --debug='1'
    ```

### 4. Detection Structure
