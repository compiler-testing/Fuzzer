## Setup
This section gives the steps, explanations and examples for getting the project running.

#### 1) Clone this repo
```
git clone https://github.com/compiler-testing/Fuzzer.git
```

init submodules
```
git submodule update --init
```

#### 2) Install MLIR
Please refer to the [LLVM Getting Started](https://llvm.org/docs/GettingStarted.html) in general to build LLVM. Below are quick instructions to build MLIR with LLVM.

The following instructions for compiling and testing MLIR assume that you have git, [ninja](https://ninja-build.org/), and a working C++ toolchain (see [LLVM requirements](https://llvm.org/docs/GettingStarted.html#requirements)).

```
mkdir llvm-project-16/build
cd llvm-project-16/build
cmake -G Ninja ../llvm \
   -DLLVM_ENABLE_PROJECTS=mlir \
   -DLLVM_TARGETS_TO_BUILD="X86" \
   -DCMAKE_BUILD_TYPE=Release \
   -DLLVM_ENABLE_ASSERTIONS=ON 
cmake --build . --target mlir-opt
```
#### 3) Run the testcase

```
python ../fuzz_tool/src/main.py --opt=generator \
--path=path/../llvm-project-16/ --sqlName=sqlname \
--Mut=0 --DT=dt
```
```
python ../fuzz_tool/src/main.py --opt=fuzz \
--path=path/../llvm-project-16/ --sqlName=sqlname \
--Mut=0 --DT=dt
```

### Detection Structure

``` 
llvm-project-16\mlir\test\lib:
-Pass
    CMakeLists.txt
    GetDialectName.cpp
    MIX.cpp      <- mixing mutation
    OpsTowardPass.json
    tosaGen.cpp  <- tosa graph generation
            
-TosaGen
    CMakeLists.txt
    create.cpp
    opinfo.cpp
    tosaOps.json
    transfer.cpp
    utils.cpp
```

```
fuzz_tool:.
├─case
├─conf
│      conf.yml
│      init.sql
└─src
    │  main.py
    ├─fuzz
    │    fuzz.py
    │    fuzz1.py
    │    pass_enum.py             
    ├─generator
    │     tosaGen.py          
    └─utils
        │  config.py
        │  dbutils.py
        │  logger_tool.py
        │  passinfo.txt
        │  pass_analysis.py
        │  test.py
        │  __init__.py
        │  
        └─__pycache__
                __init__.cpython-39.pyc
```