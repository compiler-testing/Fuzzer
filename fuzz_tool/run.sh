#!/usr/bin/env bash
python ../fuzz_tool/src/main.py --opt=generator \
        --path=/home/ty/llvm-project-16/   --sqlName=MLIRGen0713 --TS=0 --Mut=0 

# python /home/ty/MLIRFuzzing/fuzz_tool/src/main.py --opt=fuzz \
#          --path=/home/ty/llvm-project-16/  --sqlName=MLIRGen0713_chain --Mut=0
