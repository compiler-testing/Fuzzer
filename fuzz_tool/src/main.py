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
reported_errors = []

def get_args():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('--opt', required=True,
                            choices=['generator', 'fuzz', 'report'])
    arg_parser.add_argument('--sqlName',required=True)
    arg_parser.add_argument('--mode',default='multi-branch',choices=['api', 'chain', 'multi-branch'])
    arg_parser.add_argument('--Mut',default='0',choices=['0', '1', '2', '3'])  #no mix rep mut
    arg_parser.add_argument('--DT',default='dt',choices=['r', 'rr','c','dt'])
    arg_parser.add_argument('--debug',default='0',choices=['0', '1'])
    return arg_parser.parse_args(sys.argv[1:])


def create_new_table(conf: Config):
    with open(conf.project_path + '/fuzz_tool/conf/init.sql', 'r',encoding="utf-8") as f:
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


def main():
    args = get_args()  
    #==================need to modify with your path=============#
    config_path = '/home/ty/compiler-testing/fuzz_tool/conf/conf.yml'  # debug
    # config_path = './conf/conf.yml'  # 配置文件路径   # Run
    #============================================================#
    

    conf = Config(config_path,args.sqlName)
    
    logger_tool.get_logger()
    dbutils.db = dbutils.myDB(conf)
    if args.opt == 'generator':
        from generator.tosaGen import generate_user_cases
        # initialize database
        create_new_table(conf)
        # generate tosa graphs
        generate_user_cases(conf, conf.count,args.mode)

    elif args.opt == 'fuzz':
        from fuzz.fuzz import Fuzz
        fuzzer = Fuzz(conf)
        start = datetime.datetime.now()
        end = start + datetime.timedelta(minutes=conf.run_time)
        st= start.timestamp()
        nt = st
        while (nt-st<43200):
            now = datetime.datetime.now()
            nt= now.timestamp()
            print(datetime.datetime.now())
            conf.Iter +=1    
            if args.debug != '0':
                fuzzer.debug()
                break
            fuzzer.process(args.Mut,args.DT)
            if now.__gt__(end):
                break
        print("time out!!!")



# RUN : python main.py --opt=fuzz
if __name__ == '__main__':
    main()
