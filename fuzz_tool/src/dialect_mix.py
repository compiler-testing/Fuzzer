
import sys
from time import time
import pandas as pd
from random import randint
import os
import numpy as np
sys.path.append('../')

from utils.config import *
from utils import *
from utils.dbutils import myDB
from utils.logger_tool import log
import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties
import matplotlib.ticker as mtick

def printDialect(dialects,dataList):
    count = [0] * len(dialects)


    return count

dialects = [
    "affine", "arith", "async",
    "bufferization", "builtin", "cf", "gpu",
    "linalg", "llvm", "math", "memref", "nvvm",
    "omp", "rocdl", "scf",
    "spirv", "tensor", "tosa", "vector"
]
def getCov(sqlname):
    sql = "select dialect,operation FROM %s" %sqlname
    dataList = dbutils.db.queryAll(sql)
    count_list = []
    for data in dataList:
        x = data[0].split(',')
        count_list.append(len(x))
    unique_list = list(set(count_list))
    print(unique_list)
    return unique_list

config_path = '/../mytest/fuzz_tool/conf/conf.yml'  # 配置文件路径
conf = Config(config_path,"/../fuzzer/llvm-project-16/","Fuzzer",0)
logger_tool.get_logger()
dbutils.db = dbutils.myDB(conf) 

# sqlname = "seed_pool_test1"
# sqlname = "seed_pool_MLIRGen0714_Rep"
sqlname = "seed_pool_MLIRFuzzer0719_rr"
count = getCov(sqlname)

sqlname = "seed_pool_MLIRFuzzer0719_r"
count1 = getCov(sqlname)

sqlname = "seed_pool_MLIRFuzzer0719_c"
count2 = getCov(sqlname)

sqlname = "seed_pool_MLIRFuzzer0719_dt"
count3 = getCov(sqlname)


