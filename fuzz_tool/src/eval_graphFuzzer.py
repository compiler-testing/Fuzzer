# -*- coding: utf-8 -*-

import argparse
import sys
import time
import datetime
from utils.config import Config
from utils import *
import pymysql
import os
from utils.logger_tool import log



def get_args():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('--path', required=True)
    arg_parser.add_argument('--opt', required=True,
                            choices=['generator', 'fuzz', 'report'])
    arg_parser.add_argument('--TS',choices=['1', '0'])
    arg_parser.add_argument('--sqlName',required=True)
    arg_parser.add_argument('--Mut',required=True,choices=['1', '0'])
    return arg_parser.parse_args(sys.argv[1:])


def create_new_table(conf: Config):
    # with open('../conf/init.sql', 'r',encoding="utf-8") as f:
    with open('/../mytest/fuzz_tool/conf/init.sql', 'r',encoding="utf-8") as f:

        sql = f.read().replace('seed_pool_table', conf.seed_pool_table) \
            .replace('result_table', conf.result_table) \
            .replace('report_table', conf.report_table)
    try:
        dbutils.db.connect_mysql()
        sql_list = sql.split(';')[:-1]
        for item in sql_list:
            dbutils.db.cursor.execute(item)
        dbutils.db.db.commit()
        print("database init success!")
    except pymysql.Error as e:
        print("SQL ERROR:=======>", e)
    finally:
        dbutils.db.close()




def load_cases(config: Config):
    sys.path.append('../')
    # folder_path = '/../mytest/fuzz_tool/src/data/nnsmith_mlir_testcase'
    # folder_path = '/../mytest/fuzz_tool/src/data/graphfuzzer-mlir'
    # folder_path = '/../mytest/fuzz_tool/src/data/lemon_mlir'
    folder_path = '/../mytest/fuzz_tool/src/data/muffin_mlir'

    mlir_files = [os.path.join(folder_path, f) for f in os.listdir(folder_path) if f.endswith('.mlir')]  
    count=0;
    
    from generator.tosaGen import seedAnalysis
    # 循环遍历所有txt文件
    for file in mlir_files:
        # file = "/../mytest/data/nnsmith_mlir_testcase/1797.onnx.mlir"
        dialects,candidate_lower_pass,operations = seedAnalysis(config,file)
 
        print("loading", file)
        with open(file, 'r') as f:
            content = f.read()
        
        if len(content)>3000000:
            continue
        try:
            sql = "insert into "+ config.seed_pool_table + \
                    " (preid,source,mtype,dialect,operation,content,n, candidate_lower_pass) " \
                    "values ('%s','%s','%s','%s','%s','%s','%s','%s')" \
                    % \
                    (0,'G','',dialects,operations, content, 0, candidate_lower_pass)
            # print(sql)
            dbutils.db.executeSQL(sql)
            count = count+ 1
        except Exception as e:
            log.error('sql error', e)
        if count==config.count:
            break;
    return

def main():
    args = get_args()  # 运行参数，例如Namespace(opt='fuzz', config='../conf/conf.json')
    config_path = '/../mytest/fuzz_tool/conf/conf.yml'  # 配置文件路径
    conf = Config(config_path,args.path,args.sqlName,args.TS)
    
    logger_tool.get_logger()
    dbutils.db = dbutils.myDB(conf)
    if args.opt == 'generator':
        # 生成新数据前, 默认初始化
        create_new_table(conf)
        load_cases(conf)
    elif args.opt == 'fuzz':
        from fuzz.fuzz import Fuzz
        fuzzer = Fuzz(conf)
        start = datetime.datetime.now()
        end = start + datetime.timedelta(minutes=conf.run_time)
        while True:
            now = datetime.datetime.now()
            # print(datetime.datetime.now())
            conf.Iter +=1    
            fuzzer.process(conf.flag_VPA,args.Mut)
            if now.__gt__(end):
                break
    elif args.opt == 'report':
        from report.result_analysis import Analysis
        an = Analysis(conf)
        an.stackStatistic()


# RUN : python main.py --opt=fuzz
if __name__ == '__main__':
    main()
