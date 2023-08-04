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
# sys.path.append('../')
# sys.path.append('./fuzz_tool/src')
import re
from utils.config import *
from utils import *
from utils.dbutils import myDB

from utils.logger_tool import log

# 检查目录是否已存在
def directoryCheck(filepath):
    if not os.path.exists(filepath):
        os.makedirs(filepath)
        print(f"目录 {filepath} 创建成功")
    else:
        print(f"目录 {filepath} 已存在")
        return 0
    
#保存数据库的数据：函数存入.cpp文件,其他存入npy
def saveFuncToFile(dataList,flag,savepath):
    # 要创建的目录路径
    filepath = savepath+'funcs_'+flag
    directoryCheck(filepath)
    
    for data in dataList:
        save_path =filepath +'/func_' + str(data[0]) + '.cpp'
        print("saving ",save_path)
        with open(save_path, 'w') as f:
            f.write(data[4])

    print("saved all ", flag," files")
    print("file num: ",len(dataList))

    #数据保存为npy文件
    
    col1 = [row[0] for row in dataList]  # 提取第一列
    col5 = [row[-1] for row in dataList]  # 提取第五列

    list = []
    for row in dataList:
        list.append([row[0],row[-1]])
                    
    dataArray = np.array(list)
    np.save(savepath+ flag+".npy", dataArray)

    print("saved all ", flag," list")


# 从cvs中读取数据
def readdata(path):
    data = pd.read_csv(path, encoding="GBK") 
    data_array = np.array(data[['File Name','Lines*', 'Statements','Percent Lines with Comments','Percent Branch Statements','Maximum Complexity*','Maximum Block Depth']])  #将pandas读取的数据转化为array
    return data_array


def mergedata(flag,csv_data,npy_data):
    for i in range(0,len(csv_data)):
        str = csv_data[i][0]
        numbers = re.findall(r'\d+', str)
        result = int(''.join(numbers))+1
        csv_data[i][0] = result
    
    # csv_list = csv_data.astype(float)
    merge_data = []
    for data1 in npy_data:
        for data2 in csv_data:
            if(data1[0]==data2[0]):
                data = []
                data.extend(data1)
                data.extend(data2[1:])
                merge_data.append(data)

    # sorted_list = sorted(csv_list, key=lambda x: x[0])
    # mergedata = np.hstack((npy_data, sorted_list))
    np.save(savepath+ flag+"_merge.npy", merge_data)
    print("merge ", flag," feature")
    print(len(merge_data))
def saveDatafromSql(savepath,sqltable):
    flag="non"
    sql = "select id,func_name,file_name,path,content,count,call_hierarchy,pass FROM %s where pass != '' " %sqltable
    dataList = dbutils.db.queryAll(sql)
    saveFuncToFile(dataList,"16",savepath)


if __name__ == '__main__':
    config_path = '/../mytest/fuzz_tool/conf/conf1.yml'  # 配置文件路径
    conf = Config(config_path,"/../fuzzer/llvm-project-16/","Fuzzer",0)
    dbutils.db = dbutils.myDB(conf)

    savepath = "/../MLIRFuzzing/bugpre/data/funs/"
    
    flag = "16"
    target_sqltable = "current_func_info"

    #  1.保存所有file为.cpp文件
    # if directoryCheck(savepath)!=0:
    saveDatafromSql(savepath,target_sqltable)
   
    # #  2.手动预处理

    # #  3.读取处理后的scv 以及 npy
    # if flag == "bug":
    #     path = savepath+ '/bug_file.csv'
    # if flag == "non":
    #     path = savepath+ '/non_file.csv'
    # csv_data = readdata(path)

    # # 读取.npy文件
    # npy_data = np.load(savepath+ flag+".npy",allow_pickle=True)

    # #  4.融合代码特征和变更特征
    # mergedata(flag,csv_data,npy_data)

    #  5.保存测试特征
    # flag = "16"
    # target_sqltable = "func_info_16"
    # sql = "select id,path,func_name,content FROM %s " %target_sqltable
    # dataList = dbutils.db.queryAll(sql)
    # saveFuncToFile(dataList,flag,savepath)





