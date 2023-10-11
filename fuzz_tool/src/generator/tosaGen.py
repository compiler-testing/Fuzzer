# -*- coding: utf-8 -*-
# import sys
# sys.path.append(r"/../Documents/MLIR/MLIR_code")


# v0.10.1及以下
import os
import subprocess
import sys

sys.path.append('../')
from utils import *
from utils.logger_tool import log
from utils.config import Config
from fuzz.fuzz import *

def seedAnalysis(config, target_file):
    IRanalysis(target_file, config)
    with open(config.analysis_seed_file, 'r', encoding='utf8') as temp:
        json_data = json.load(temp)
        dialect_list = " "
        lowerPass_list = " "
        operations_list = " "
        if int(json_data["DialectNum"]) != 0:
            dialect_list = json_data["dialect"]
        if int(json_data["LowerPassNum"]) != 0:
            lowerPass_list = json_data["LowerPass"]
        if int(json_data["LowerPassNum"]) != 0:
            operations_list = json_data["operation"]
     
    # 将方言列表和候选降级列表组成字符串，方便后续存入数据库
    dialects = ','.join(dialect_list)
    lowerPasses = " ".join(lowerPass_list)
    operations = ','.join(operations_list)
    return dialects,lowerPasses,operations


def generate_user_cases(config: Config, seeds_count, mode):
    # for i in range(seeds_count):
    count=0;
    i = 0
    start = datetime.datetime.now()
    st= start.timestamp()
        
    while(count< seeds_count):
        log.info("======generate seed : " + str(count))
        target_file = config.temp_dir + str(count) + '.mlir'
        genrateOpt = "-tosaGen"
        if mode=="api":
            genrateOpt = "-tosaGenU"
        elif mode=="chain":
            genrateOpt = "-tosaGenC"

        cmd = '%s %s %s -o %s' % (config.mlirfuzzer_opt, config.empty_func_file, genrateOpt,target_file)
        #subprocess.PIPE 表示为子进程创建新的管道
        pro = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, universal_newlines=True, encoding="utf-8")
        try:
            i = i+1
            stdout, stderr = pro.communicate(timeout=5)
            returnCode = pro.returncode
            log.info(cmd)
            if not os.path.exists(target_file):
                # 报错不会生成, 打印错误日志
                log.error(stderr)
                continue
            dialects,candidate_lower_pass,operations = seedAnalysis(config,target_file)
            log.info(candidate_lower_pass)
            log.info(operations)
            with open(target_file, 'r') as f:
                content = f.read()
            log.info(len(content))
            if len(content)>10:
                try:
                    sql = "insert into "+ config.seed_pool_table + \
                          " (preid,source,mtype,dialect,operation,content,n, candidate_lower_pass) " \
                          "values ('%s','%s','%s','%s','%s','%s','%s','%s')" \
                          % \
                          (0,'G','','tosa', operations,content, 0, candidate_lower_pass)
                    dbutils.db.executeSQL(sql)
                    count = count+ 1
                except Exception as e:
                    log.error('sql error', e)
                os.remove(target_file)
            else:
               log.info("Insufficient seed length")
        except subprocess.TimeoutExpired:
            pro.kill()
            stdout = ""
            stderr = "timeout, kill this process"
            returnCode = -9

    print(count/i)
    now = datetime.datetime.now()
    nw= now.timestamp()
    print(nw-st)