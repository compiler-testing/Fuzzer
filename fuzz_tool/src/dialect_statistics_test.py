
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
    for data in dataList:
        print(data)
        for dia in dialects:
            if dia in data:
                if dia=="gpu":
                    print("gpu")
                # print(dia)
                index = dialects.index(dia)
                count[index] = count[index] + 1

    for i in range(0,len(dialects)-1):
        print(dialects[i]," : ",count[i])
    return count

dialects = [
    "affine", "arith", "async",
    "bufferization", "builtin", "cf", "gpu",
    "linalg", "llvm", "math", "memref", "nvvm",
    "omp", "rocdl", "scf",
    "spirv", "tensor", "tosa", "vector"
]
def getCov(sqlname):
    # dialects = [
    #     "acc", "affine", "amdgpu", "amx", "arith", "arm_neon", "arm_sve", "async",
    #     "bufferization", "builtin", "cf", "complex", "dlti", "emitc", "func", "gpu",
    #     "index", "linalg", "llvm", "math", "memref", "ml_program", "nvgpu", "nvvm",
    #     "omp", "pdl", "pdl_interp", "quant", "rocdl", "scf", "shape", "sparse_tensor",
    #     "spirv", "tensor", "test", "test_dyn", "tosa", "transform", "vector",
    #     "x86vector"
    # ]
    sql = "select dialect,operation FROM %s" %sqlname
    dataList = dbutils.db.queryAll(sql)
    dialectList = [row[0] for row in dataList] 

    count = printDialect(dialects,dialectList)

    return count

config_path = '/../mytest/fuzz_tool/conf/conf.yml'  # 配置文件路径
conf = Config(config_path,"/../fuzzer/llvm-project-16/","Fuzzer",0)
logger_tool.get_logger()
dbutils.db = dbutils.myDB(conf) 

# sqlname = "seed_pool_test1"
# sqlname = "seed_pool_MLIRGen0714_Rep"
sqlname = "seed_pool_test1"
count = getCov(sqlname)

barWidth = 0.2
barWidth1= 0.2
fig,ax = plt.subplots(figsize=(8, 3.9))
# ax.tick_params(axis='x',length=0)

# ax = plt.subplot(2,1,1)
y = mtick.MultipleLocator(0.25)
ax.yaxis.set_major_locator(y)
ax.spines['right'].set_visible(False)
ax.spines['top'].set_visible(False)

br = np.arange(len(dialects))

plt.bar(br, count, color='#e2e2e2', width=barWidth,
        edgecolor=None, label='MLIRFuzzer_r*',zorder=10)
    #设置坐标轴范围
plt.xlim((-0.5, 21))
# plt.ylim((0, 1))

#设置坐标轴名称
# plt.xlabel('Dialects', fontweight='bold', fontsize=15)
plt.ylabel('Count', fontweight='bold', fontsize=12)

yticks = np.linspace(0,max(count),5)
plt.yticks(yticks,fontsize=15)
plt.legend(loc='lower left',mode="expand",ncol=4,bbox_to_anchor=(0,1.02,1,0.2),frameon=True,prop = {'size':12}, framealpha=1)


location=[r for r in range(len(dialects))]
# location[0] = location[0]-0.3
# location[3] = location[3]-0.8 
# location[7] = location[7]-0.4
# location[12] = location[12]-0.5
plt.xticks(location,dialects,rotation=45,fontsize=12)
#设置网格线
plt.grid(which='major',axis='y',zorder=0,color='#CCCCCC')

plt.tight_layout()
plt.show()
plt.savefig('./dialectstest.png')    
# plt.savefig('./dialectsDT.pdf')  