import random
import datetime
import subprocess
import sys
import json
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

#将数据库中读取的数据存入.cpp文件
def saveFuncToFile(i, funcName, funcContent):
    with open('/../mytest/bugPre/data/funcs_non/func_' + str(i) + '.cpp', 'w') as f:
        f.write(funcContent)



#将.csv文件数据读入，存成list
# 这个文件是通过工具解析的结果，需要修改成自己的路径  /..xmr/mlir/llvm-16/mlir/mytest/fuzz_tool/src/getFunc/func.csv
def readFileToList():
    path = '/..xmr/mlir/llvm-16/mlir/mytest/fuzz_tool/src/getFunc/func.csv'
    data = pd.read_csv(path, encoding="GBK") 
    data_array = np.array(data[['Lines', 'Statements','Percent Lines with Comments','Maximum Complexity*','Line Number of Deepest Block']])  #将pandas读取的数据转化为array
    data_list = data_array.tolist()   #将数组转化为list
    print(data_list)
    print(type(data_list))



if __name__ == '__main__':
    config_path = '/../mytest/fuzz_tool/conf/conf1.yml'  # 配置文件路径
    conf = Config(config_path,"/../fuzzer/llvm-project-16/","Fuzzer",0)
    #logger_tool.get_logger()
    dbutils.db = dbutils.myDB(conf)
    sql = "select name,content,cooperaion,revised FROM 15_16_diff_functions where flag = 'non' "
    dataList = dbutils.db.queryAll(sql)
    #dataList = dbutils.db.queryAll(sql)
    i = 0
    list2=[]
    for data in dataList:
        print("===> save to file")
        # saveFuncToFile(i, data[0], data[1])
        list1=[]
        list1.append(i)
        list1.append(data[-1:])
        list2.append(list1)

        i = i+1
    
    print(len(list2))
    # readFileToList()