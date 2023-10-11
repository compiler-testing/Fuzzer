#!/usr/bin/env bash
<<'COMMENT'
Customized Configuration:
    --sqlName: set the name of the database table
    --mode : set the mode for creating tosa graphs. There are three modes available here.
            1）--mode=api，using a single operator to generate;
            2) --mode=chain，generated graph has a chain structure;
            3) --mode=multi-branch，generated graph has multi-branch structure. It's disabled by default.


COMMENT

python3 ./src/main.py --opt=generator  --sqlName=MLIRFuzz 
