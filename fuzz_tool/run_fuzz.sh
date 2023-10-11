#!/usr/bin/env bash
# Customized Configuration:
# --sqlName: set the name of the database table
# --sqlName: --mode {api,chain,multi-branch}

python3 ./src/main.py --opt=fuzz  --sqlName=MLIRFuzz
