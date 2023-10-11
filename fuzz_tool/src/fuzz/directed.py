# -*- coding: utf-8 -*-
import random
import datetime
import subprocess
import sys
import json
from time import time
import Levenshtein
from random import randint
import os
import numpy as np
import re


sys.path.append('../')
from utils.config import Config
from utils import *
from utils.dbutils import myDB

from utils.logger_tool import log
from fuzz.pass_enum import *

def get_rand_pass_pipe(pass_list: list,N) -> str:
    maxlen = len(pass_list)
    if len(pass_list)>=N:
        maxlen = N

    count = random.randint(1, maxlen)
    sub_list = random.sample(pass_list, count)
    return sub_list

def get_all_pass_pipe(pass_list: list) -> str:
    pass_pipe = " ".join(pass_list)
    return pass_pipe

def IRanalysis(temp_file, config:Config):
    """
    分析mlir文本的方言和pass
    """
    cmd = '%s %s -allow-unregistered-dialect -GetDialectName ' % (config.mlirfuzzer_opt, temp_file)
    start_time = int(time() * 1000)

    pro = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE, universal_newlines=True, encoding="utf-8")
    try:
        stdout, stderr = pro.communicate(timeout=30)
        return_code = pro.returncode
        # log.info(cmd)

    except subprocess.TimeoutExpired:
        pro.kill()
        stdout = ""
        stderr = "timeout, kill this process"
        return_code = -9
        # with open(config.analysis_seed_file, 'r', encoding='utf8') as temp:
            # json_data = json.load(temp)
            # if (json_data["LowerPassNum"] != 0) :
                # PassStr = " ".join(json_data["LowerPass"])
                # if (PassStr != "-convert-func-to-llvm -convert-func-to-spirv") :
                # no_candidate_lowerPass = False

            # cooperation = calculate_cooperation(json_data["dialect"], config)
            # if  (cooperation == 0):
            #     no_candidate_lowerPass = False


def getDiff(raw_mlir,processed_mlir):
    sim = Levenshtein.ratio(raw_mlir, processed_mlir)
    return 1-sim

def verifyIR(dialects,result):
    log.info("===Identification of invalid IR")
    """
    无效IR判定
    1.只有一个方言的情况，只有func、spv方言
    2.只有func和arith
    """
    vaild = True
    if (len(dialects) == 1):
        if "func" or "spv" or "llvm" in dialects:
            vaild = False
    elif (len(dialects) == 2):
        if "func" and "arith" in dialects:
            vaild = False
    if (vaild == False):
        log.info("=== invalid IR, report this bug")

    return vaild

def getFileContent(file):
    with open(file, 'r',encoding="utf-8") as f:
        content = f.read()
    return content

# 保存变异、优化种子
def analysis_and_save_seed(seed_file,temp_file,result, config: Config, flag):
    """
    计算变异前后种子差异
    如果相似度大于阈值，分析其方言和降级pass，将其存入数据库
    否则不保存
    计算优化前后种子差异
    如果相似度大于阈值，分析其方言和降级pass
    判断IR是否有效，如果IR无效，报告错误
    否则将其存入数据库
    """

    with open(seed_file, 'r',encoding="utf-8") as f:
        seed_mlir = f.read()
    
    with open(temp_file, 'r',encoding="utf-8") as f:
        processed_mlir = f.read()

    if seed_mlir==processed_mlir:
        print("diff = 0")
    else:
        IRanalysis(temp_file, config)
        from generator.tosaGen import seedAnalysis
        dialects,lowerPasses,operations = seedAnalysis(config,config.analysis_seed_file)
        if (dialects !=' '):  #变异前后差异性大于阈值,保存到种子池
            # log.info("===IRanalysis")
            # 读取temp.json文件，获取当前mlir文本中的方言和降级pass
            lowerPasses = lowerPasses.replace('  ', ' ')
            fuzzer = DTFuzz(config)
            DTFuzz.mutate_success_handler(fuzzer, result["sid"], result["mutate_type"], dialects,operations, processed_mlir,
                                                lowerPasses,flag)


def setOptSeq(dia):
    """
    1.启用脆弱性分析：优化序列：VP + 随机优化
    2.随机优化
    """
    list = []

    # list.extend(get_rand_pass_pipe(OptimizePass.all,10))
    savefile = "/../MLIRFuzzing/bugpre/data/vulpass.npy"
    npy_data = np.load(savefile,allow_pickle=True)
    if dia=="affine":
        list.extend(get_rand_pass_pipe(npy_data.tolist()["Affine"],5))

    random.shuffle(list)
    # list.append("-verify-each")
    passSeq = " ".join(list)
    pass_list = []
    pass_list.append(passSeq)

    return pass_list

def setLowerSeq(lower_pass):
    # log.info("======combine candidates and randPass")
    list = []
    candidates = get_rand_pass_pipe(str_to_list(lower_pass),3)
    randoms = get_rand_pass_pipe(LowerPass.all,3)

    for i, x in enumerate(candidates) :
        if x.find("linalg-to")!=-1:
            # print(candidates[i])
            candidates[i] = "-func-bufferize -linalg-bufferize " +candidates[i]

    for i, x in enumerate(randoms):
        if x.find("linalg-to") != -1:
            # print(randoms[i])
            randoms[i] = "-func-bufferize -linalg-bufferize " + randoms[i]

    for i, x in enumerate(randoms):
        if x.find("convert-parallel-loops-to-gpu") != -1:
            # print(randoms[i])
            randoms[i] = "-gpu-map-parallel-loops " + randoms[i]

    # list.extend(candidates)
    # list.extend(randoms)

    # # random.shuffle(list)
    # passSeq = " ".join(list)

    # pass_list = []
    # pass_list.append(" ".join(candidates))
    # pass_list.append(passSeq)

    return " ".join(candidates)


def setOptSeqR():
    """
    1.启用脆弱性分析：优化序列：VP + 随机优化
    2.随机优化
    """
    list = []
    list.extend(get_rand_pass_pipe(OptimizePass.all,20))
    random.shuffle(list)
    passSeq = " ".join(list)
    # pass_list = []
    # pass_list.append(passSeq)

    return passSeq

def combine_PassList(dialect_list, candidate_lower_pass) -> str:
    """
    组成单步Pass:
    全部优化pass+随机选择普通优化pass+全部脆弱降级pass+全部候选降级pass
    """
    log.info("======combine optPass and lowerPass")
    list = []

    # 1.选择全部脆弱优化pass
    list.append(get_all_pass_pipe(OptimizePass.vp))

    # if 'affine' in dialect_list:
    #     list.append(get_all_pass_pipe(OptimizePass.vp))
    # # if 'linalg' in dialect_list:
    # #     list.append(get_all_pass_pipe(WeakOptPass.linalg))
    # if 'scf' in dialect_list:
    #     list.append(get_all_pass_pipe(WeakOptPass.scf))

    # # 2.选择随机优化pass
    # for item in dialect_list:
    #     # print(type(dialects[i]))
    #     dia = item + '_pass'

    #     # list.append(ExcuteConfig(get_rand_pass_pipe(OptimizePass.dia)))

    #     if dia.find("affine_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.affine_pass))
    #     # if dia.find("linalg_pass") != -1:
    #     #     list.append(get_rand_pass_pipe(OptimizePass.linalg_pass))
    #     if dia.find("scf_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.scf_pass))
    #     if dia.find("bufferize_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.bufferize_pass))
    #     if dia.find("memref_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.memref_pass))
    #     if dia.find("general_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.general_pass))
    #     if dia.find("async_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.async_pass))
    #     if dia.find("vector_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.vector_pass))
    #     if dia.find("gpu_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.gpu_pass))
    #     if dia.find("spirv_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.spirv_pass))
    #     if dia.find("llvm_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.llvm_pass))
    #     if dia.find("tosa_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.tosa_pass))
    #     if dia.find("arith_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.arith_pass))
    #     if dia.find("func_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.func_pass))
    #     if dia.find("tensor_pass") != -1:
    #         list.append(get_rand_pass_pipe(OptimizePass.tensor_pass))

    # 3.选择脆弱降级pass
    # if 'tosa' in dialect_list:
    #     list.append(get_all_pass_pipe(WeakLowerPass.tosa))
    if 'scf' in dialect_list:
        list.append(get_all_pass_pipe(WeakLowerPass.scf))
    if 'vector' in dialect_list:
        list.append(get_all_pass_pipe(WeakLowerPass.vector))
    if 'gpu' in dialect_list:
        list.append(get_all_pass_pipe(WeakLowerPass.gpu))

    # 4.候选降级Pass全选
    list.append(candidate_lower_pass)

    # 5.将list进行组合
    singlePass = " ".join(list)
    log.info(singlePass)
    return singlePass


def fixlowerpass(singlePass):
    # singlePass = "-pass-pipeline=\"builtin.module(func.func(tosa-to-linalg-named,tosa-to-linalg))\" -test-linalg-elementwise-fusion-patterns=fuse-generic-ops-control -test-vector-transferop-opt -mlir-print-ir-module-scope -sparse-compiler=\"enable-index-optimizations=true\" -view-op-graph=\"print-data-flow-edges=true\""
    newpass = []
    passlist = str_to_list(singlePass)
    if len(passlist) == 1:   #singlePass只有一个pass
        newpass.append(singlePass)
    else:
        passlist = list(filter(lambda x: x != '', passlist))
        locs = []
        for i, x in enumerate(passlist):
            if x.find("-pass-pipeline") >= 0:
                locs.append(i) #保存所有-pass-pipeline的位置
                
        if len(locs) == 0:
            newpass.append(singlePass)
        else:
            l = locs[0]
            if l == 0:
                newpass.append(passlist[0])
            else:
                newpass.append(' '.join(passlist[0:l]))
                newpass.append(passlist[l])
            if (len(locs) == 1 and l!=len(passlist)-1):
                newpass.append(' '.join(passlist[l + 1:]))
            else:
                for i in locs[1:]:
                    if (i - l > 1):
                        newpass.append(' '.join(passlist[l + 1:i]))
                    newpass.append(passlist[i])
                    if i == locs[-1]:
                        if i != len(passlist) - 1:
                            newpass.append(' '.join(passlist[i + 1:]))
                    l = i
    #log.info(newpass)
    return newpass



def execute_pass1(input_file,output_file, sid, raw_mlir, singlePass, config:Config,flag):
    # log.info("========= execute pass ========")

    # 文件写入，初始化写入raw_file
    # 变异后刷新raw_file,raw_file是原始mlir或者变异mlir，保存
    # 启用优化；生成opt_mlir,保存
    # 启用降级：对raw_file进行降级
    
    # cmd = '/../llvm-project-16/mlir//build/bin/mlir-opt /../llvm-project-16/mlir//mytest/fuzz_tool/case/032201ty_o0/seed.mlir -test-scf-pipelining=no-epilogue-peeling -test-options-pass=string-list=a -arith-emulate-wide-int -test-memref-stride-calculation -test-linalg-transform-patterns="test-swap-subtensor-padtensor " -scf-parallel-loop-collapsing -test-linalg-transform-patterns="test-transform-pad-tensor " -async-parallel-for -verify-each -o /../llvm-project-16/mlir//mytest/fuzz_tool/case/032201ty_o0/opt.mlir'
    # cmd = '/../llvm-project-16/mlir//build/bin/mlir-opt /../llvm-project-16/mlir//mytest/fuzz_tool/case/032201ty_o0/seed.mlir -resolve-shaped-type-result-dims -canonicalize=enable-patterns=TestRemoveOpWithInnerOps -affine-loop-fusion=mode=producer -linalg-named-op-conversion -tosa-validate -one-shot-bufferize="analysis-fuzzer-seed=23"  -test-vector-warp-distribute=rewrite-warp-ops-to-scf-if -test-affine-data-copy -verify-each -o /../llvm-project-16/mlir//mytest/fuzz_tool/case/032201ty_o0/opt.mlir '

    if singlePass.find("func.func") >= 0:
        newpass = []
        newpass = fixlowerpass(singlePass)
        cmd_ = '%s %s -allow-unregistered-dialect %s' % (config.mlir_opt, input_file, newpass[0])
        cmd1 = cmd_
        if len(newpass)>1:
            for seg in newpass[1:]:
                cmd2 = '| %s %s' % (config.mlir_opt, seg)
                # cmd2 = '| %s %s %s' % (config.mlir_opt, input_file, seg)
                cmd1 = cmd1 + cmd2
            cmd_ = cmd1
        cmd=  cmd_  + ' -o %s' % (output_file)
    else:
        cmd = '%s %s -allow-unregistered-dialect %s -o %s' % (config.mlir_opt, input_file, singlePass, output_file)
    
    start_time = int(time() * 1000)
    #  shell 为true，执行shell内置命令  subprocess.PIPE 表示为子进程创建新的管道
    pro = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE, universal_newlines=True, encoding="utf-8")
    try:
        stdout, stderr = pro.communicate(timeout=30)
        return_code = pro.returncode
        # log.info(getFileContent(input_file))
        log.info(cmd)
        if(stderr!=""):
            log.error(stderr)
    except subprocess.TimeoutExpired:
        pro.kill()
        stdout = ""
        stderr = "timeout, kill this process"
        return_code = -9

    stderr = stderr.replace('\'', '')
    end_time = int(time() * 1000)
    duration = int(round(end_time - start_time))

    if (flag == "mutate"):
        mutate_type =singlePass
    else:
        mutate_type = ''

    result = {
        "sid":sid,
        "input":raw_mlir,
        "mutate_type":mutate_type,
        "cmd":cmd,
        "return_code":return_code,
        "stdout":stdout,
        "stderr":stderr,
        "duration":duration
    }

    return result

def RepMut(output_file,result,OPdict):
    raw_mlir = result["input"]
    output = raw_mlir
    result["return_code"] = 1
    if "affine" in OPdict:
        sub1 = re.sub(r'affine.load', 'memref.load', raw_mlir)
        output = re.sub(r'affine.store', 'memref.store', sub1)
        print("affine mutation")
        result["return_code"] = 0
    # if "linalg" in OPdict:
    #     candidates = ["pooling_nwc_min,pooling_nwc_min_unsigned,pooling_nwc_max_unsigned"]  #"pooling_nwc_sum","pooling_nwc_max",
    #     print("insert linalg")
    with open(output_file, 'w') as f:
        f.write(output)
    return result 

def execute_mlir(input_file, output_file, sid, raw_mlir, singlePass, config: Config, flag,OPdict) -> str:
    """
    执行编译
    """
    result = {
        "sid":sid,
        "input":raw_mlir,
        "mutate_type":"",
        "cmd":"",
        "return_code":"",
        "stdout":"",
        "stderr":"",
        "duration":"",
    }

    if singlePass.find("func.func") >= 0:
        newpass = []
        newpass = fixlowerpass(singlePass)
        cmd_ = '%s %s  %s' % (config.mlir_opt, input_file, newpass[0])
        cmd1 = cmd_
        if len(newpass)>1:
            for seg in newpass[1:]:
                cmd2 = '| %s %s' % (config.mlir_opt, seg)
                cmd1 = cmd1 + cmd2
            cmd_ = cmd1
        cmd=  cmd_  + ' -o %s' % (output_file)
    else:
        cmd = '%s %s -allow-unregistered-dialect %s -o %s' % (config.mlir_opt, input_file, singlePass, output_file)
    
    start_time = int(time() * 1000)
    #  shell 为true，执行shell内置命令  subprocess.PIPE 表示为子进程创建新的管道
    pro = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE, universal_newlines=True, encoding="utf-8")
    try:
        stdout, stderr = pro.communicate(timeout=30)
        return_code = pro.returncode
        # log.info(getFileContent(input_file))
        log.info(cmd)
        if(stderr!=""):
            log.error(stderr)
    except subprocess.TimeoutExpired:
        pro.kill()
        stdout = ""
        stderr = "timeout, kill this process"
        return_code = -9

    stderr = stderr.replace('\'', '')
    end_time = int(time() * 1000)
    duration = int(round(end_time - start_time))

    result["cmd"] = cmd
    result["return_code"] = return_code
    result["stdout"] = stdout
    result["stderr"] = stderr
    result["duration"] = duration

    fuzzer = DTFuzz(config)
    from fuzz.fuzz import Fuzz
    if (result["return_code"] != 0):  # 运行结果出错，存入数据库，reslut
        Fuzz.failer_handler(fuzzer, result,flag,config)

    return result


def runPass_OnlyLower(input_file, output_file, result, candidate_lowerPass, config: Config,flag):
    result = execute_pass(input_file, output_file, result, candidate_lowerPass, config,flag)
    fuzzer = Fuzz(config)
    if (result["return_code"] != 0):  # 运行结果出错，存入数据库，reslut
        DTFuzz.failer_handler(fuzzer, result,flag,config)

    return result



def str_to_list(content: str) -> list:
    list = content.split(' ')
    return list

def filter_seed(dialects,candidate_lowerPass)->bool:
    #dialect_list = str_to_list(dialects)
    #candidate_lowerPass_list = str_to_list(candidate_lowerPass)
    is_filter = True
    for item in AllWeakLowerPass.all_weak_lower_pass:
        if item in candidate_lowerPass:
            is_filter = False
    return is_filter

def select_emiPass(dialects,operation)->str: 
    emi_pass = "" 
    # if "affine.load" in operation:
    #     emi_pass = "-Rep"
    if "cf" not in dialects:
        emi_pass =  random.choice(["" ,"-BCF"])
    # else: 
    # emi_pass = random.choice(["" ,"-Mix"])

    # emi_pass = "-Mix"

    # emi_pass = random.choice(EmiPassClass.all_emi_pass)
    pass_list = []
    pass_list.append(emi_pass)
    return pass_list

def calculate_cooperation(dialect_list:list, config:Config)->float:
    cooperation = config.cooperation
    cooperation_result = 0
    for dialect in dialect_list:
        if dialect == "llvm":
            continue
        for key, value in cooperation.items():
            if dialect == key :
                cooperation_result = cooperation_result + value

    result = cooperation_result/75
    return result

class reportObject:
    def __init__(self, ids, stderror, returnCode, mlirContent):
        self.ids = ids
        self.stderror = stderror
        self.returnCode = returnCode
        self.mlirContent = mlirContent





# Directed Testing
class DTFuzz:
    def __init__(self, config: Config):
        self.config = config

    def select_seed(self,Nmax,num):
        sql = "select * from " + self.config.seed_pool_table + " where n < '%s' " % Nmax + "ORDER BY rand() limit %s" %num
        # sql = "select * from " + self.config.seed_pool_table + " where sid = 211815" 
        # sql = "update " + self.config.seed_pool_table + " set n = 0 where sid = '%s'" % sid
        seed_types = dbutils.db.queryAll(sql)
        return seed_types  # 一行数据

    def select_seed_DT(self,Nmax,dia,num):
        sql = "select * from " + self.config.seed_pool_table + " where n < '%s' and dialect like '%%" % Nmax +dia+"%%' ORDER BY rand() limit %s" %num
        seed_types = dbutils.db.queryAll(sql)

        return seed_types
    
    def select_seed_DT_op(self,Nmax,op,num):
        sql = "select * from " + self.config.seed_pool_table + " where n < '%s' and operation like '%%" % Nmax +op+"%%' ORDER BY rand() limit %s" %num
        seed_types = dbutils.db.queryAll(sql)
        return seed_types
    

    def select_seed_DT_limit(self,Nmax,dia,num):
        sql = "select * from " + self.config.seed_pool_table + " where n < '%s' and dialect like '%%" % Nmax +dia+"%%' ORDER BY rand() limit %s" %num
        seed_types = dbutils.db.queryAll(sql)
        return seed_types  # 一行数据
    
    def success_handler(self, sid, result_type, content, candidate_lower_pass, cooperation, from_content, from_cmd):
        try:
            sql = "insert into " + self.config.seed_pool_table + \
                  " (dialect,content,n, candidate_lower_pass, cooperation, from_sid, from_content, from_cmd) " \
                  "values ('%s','%s','%s','%s','%s','%s','%s','%s')" \
                  % \
                  (result_type, content, 0, candidate_lower_pass, cooperation, sid, from_content, from_cmd)
            dbutils.db.executeSQL(sql)
            self.update_nindex(sid)
        except Exception as e:
            log.error('sql error', e)

    def mutate_success_handler(self, sid, mtype,dialects,operations, content, candidate_lower_pass,flag):
        if (flag=="opt"):
            source = "O"
        elif (flag=="mutate"):
            source = "M"
        elif (flag == "lower"):
            source = "L"
        try:
            sql = "insert into " + self.config.seed_pool_table + \
                  " (preid,source,mtype,dialect,operation, content,n, candidate_lower_pass) " \
                  "values ('%s','%s','%s','%s','%s','%s','%s','%s')" \
                  % \
                  (sid,source,mtype,dialects, operations,content, 0, candidate_lower_pass)

            dbutils.db.executeSQL(sql)
            log.info(content)
            log.info("======save the result as seed")
        except Exception as e:
            log.error('sql error', e)

    def SplitContent(self,content:str)->str:
        content_list = content.split(("\n"))
        if content.find("LLVM ERROR:") >= 0:
            for item in content_list:
                if item.find("LLVM ERROR:") >= 0:
                    return "LLVM ERROR:" + item.split("LLVM ERROR:")[1]
        errorFunc = ''
        errorMessage = ''
        if content.find("Assertion") >= 0:
            for i in range(0,len(content_list)-1):
                if content_list[i].find("Assertion") >= 0:
                    errorMessage = content_list[i].split("Assertion")[1]
                if content_list[i].find("__assert_fail_base") >= 0:
                    errorFunc = content_list[i+2]  #提取__assert_fail_base下面第二行
                    errorFunc = errorFunc[23:]   #去掉报错的地址
                    break
                
            # if errorFunc == '':
            #     errorFunc = content
            return "Assertion:" + errorMessage + "\n" + errorFunc

        if content.find("Segmentation fault") >= 0:
            for i in range(0,len(content_list)-1):
                if content_list[i].find("__restore_rt") >= 0:
                    errorFunc = content_list[i+1]  #提取__restore_rt下面的一行
                    errorFunc = errorFunc.split("(/home")[0]  #去掉报错的地址
                    errorFunc = errorFunc[22:]
                    break
            return "Segmentation fault:" + errorFunc

    def updateReportModel(self,sid,error,report_stack_dict,stderr,return_code,content,report_model_dict):
        #error = error.replace('\\\'','\'')
        error = error.replace('\'','')
        if error in report_stack_dict.keys() and return_code!=-9 and error.find("PLEASE submit a bug report")<=0:
            log.info("===== update sids in report table =====")
        #     for key,value in report_stack_dict.items():
        #         if error == key:
        #             #更新sids
        #             sids = value + '|' + str(sid)
        #             #key = key.replace('\'', '\\\'')
        #             sql = "update " + self.config.report_table + " set sids = '%s' where stack = '%s' " % (sids,key)
        #             log.info("===== sql: " + str(sql) )
        #             dbutils.db.executeSQL(sql)
        else:
            log.info("===== add new report record in reportObject =====")
            model = reportObject(ids=sid, stderror=stderr, returnCode=return_code, mlirContent=content)
            report_model_dict[error] = model
    

    def stackStatistic(self,sid,return_code,stderr,content,conf):
        log.info("===== starting analysis error =====")
        sql = "select * from " + self.config.report_table + " where stderr is not NULL and stderr != ''"
        data = dbutils.db.queryAll(sql)
        report_model_dict = {}   
        report_stack_dict = {}  #存放当前repor表中的所有的stack信息
        for item in data:
            stack_error = item[0]
            sids = item[1]
            report_stack_dict[stack_error] = sids

        firstLineInStderr = stderr.split("\n")[0] #提取错误信息的第一行
        # process stack error
    
        # log.info("===== current result returnCode is: " + str(return_code) )
        error = ''
        #crash & segmentation fault
        if stderr.find("Assertion") >= 0 or stderr.find("LLVM ERROR:") >=0 or stderr.find("Segmentation fault (core dumped)")>=0:
            error = DTFuzz.SplitContent(self,stderr)
        else:  # time out & others
            error = firstLineInStderr

        DTFuzz.updateReportModel(self,sid,error,report_stack_dict,stderr,return_code,content,report_model_dict)
         
        for key,value in report_model_dict.items():
            log.info("===== new report record insert table =====")
            now = datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d %H:%M:%S')
            #key = key.replace('\'', '\\\'')
            returnCode = value.returnCode
            #stderr = value.stderror.replace('\'', '\\\'')
            mlirContent = value.mlirContent.replace('\'', '\\\'')
            # stderr = stderr.replace('\'','')
            
            try:
                sql = "insert into " + self.config.report_table  + \
                " (stack,sids,datetime,stderr,returnCode,mlirContent) " \
                " values('%s','%s','%s','%s','%s','%s')" \
                % \
                (key, str(conf.Iter), now, stderr, returnCode, mlirContent)
                # log.info("===== sql: " + str(sql) )
                dbutils.db.executeSQL(sql)
                log.info("======report this error successfully")
            except Exception as e:
                log.error('sql error', e)



    def failer_handler(self,result,flag,conf):
        result_list = list(result.values());
        sid, content = result_list[0:2]
        if (flag=="opt"):
            source = "O"
        elif (flag=="mutate"):
            source = "M"
        elif (flag == "lower"):
            source = "L"

        cmd, return_code, stdout,stderr, duration = result_list[3:]

        # 获得 result_id,return_code,stderr,mlirContent
        # 实时分析result，存入report表中

        returnCode_list = [-9,134,139]

        if return_code in returnCode_list:
            DTFuzz.stackStatistic(self,sid,return_code,stderr,content,conf)

        now = datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d %H:%M:%S')
        try:
            sql = "insert into  " + self.config.result_table + \
                  " (sid, content,phase,cmd,returnCode,stdout,stderr,duration,datetime) " \
                  "values ('%s','%s','%s','%s','%s','%s','%s','%s','%s')" \
                  % \
                  (sid, content, source, cmd, return_code, stdout, stderr, duration, now)
            # log.info(sql)
            dbutils.db.executeSQL(sql)
            log.info("======save this error successfully")
        except Exception as e:
            log.error('sql error', e)
            with open(conf.temp_dir + '/saveerror.txt', mode='a+') as f:
                f.write(sql)
        self.update_nindex(sid,False)

    def update_nindex(self, sid, reset=False):
        if reset:
            sql = "update " + self.config.seed_pool_table + " set n = 0 where sid = '%s'" % sid
            dbutils.db.executeSQL(sql)
        else:
            sql = "update " + self.config.seed_pool_table + " set n = n +1 where sid = '%s'" % sid
            dbutils.db.executeSQL(sql)


    def generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file):
        if '.' in dia:
            op = dia
            seeds = DTFuzz.select_seed_DT_op(self,conf.Nmax,op,5)
        elif dia =="":
            seeds = DTFuzz.select_seed(self,conf.Nmax,5)
        else:
            seeds = DTFuzz.select_seed_DT(self,conf.Nmax,dia,10)

        for selected_seed in seeds:
            sid = selected_seed[0]
            dialects,operation,raw_mlir,n,lowerPass = selected_seed[-5:]

            dialect_list = dialects.split(',')
            OPdict = {key: [] for key in dialect_list}
            if operation !=' ':
                word_list = operation.split(',')
                for item in word_list:
                    d1, d2 = item.split('.',1)
                    if d1 in OPdict:
                        if d2 not in OPdict[d1]:
                            OPdict[d1].append(d2)
                
            # 向input_file写入初始种子
            f = open(seed_file, 'w', encoding="utf-8")  # w 的含义为可进行读写
            f.write(raw_mlir)  # file.write()为写入指令
            f.close()

            
            if Mut=='1':
                Mut = np.random.choice(['0','1'])

            if(Mut!='0'):  # 是否启用变异
                log.info("================== Enable mutation ====================")
                flag = "mutate"
                # mutate_pass = select_emiPass(Mut,dialects,operation)
                import fuzz.fuzz as Fuzz
                result = Fuzz.Mutator(seed_file, mut_file, sid, raw_mlir, Mut, conf, dialects,operation)
                if (result["return_code"] == 0) :
                    Fuzz.analysis_and_save_seed(seed_file,mut_file,result, conf,flag)

            log.info(sid)
            log.info("================== Enable Lowering ====================")


    
        
            if target == "scf.for":
                lower_pass = "-lower-affine"
            if target == "scf.parallel":
                lower_pass = "-linalg-bufferize -convert-linalg-to-parallel-loops"
            if target =="affine.parallel":
                lower_pass = "-affine-parallelize"
            if target == "affine":
                lower_pass = "-linalg-bufferize -convert-linalg-to-affine-loops -affine-super-vectorize=\"virtual-vector-size=32,256 test-fastest-varying=1,0\""
            if target == "linalg":
                lower_pass = "-pass-pipeline=\"builtin.module(func.func(tosa-to-linalg-named,tosa-to-linalg))\""
            if target == "gpu":
                lower_pass = "-gpu-map-parallel-loops -convert-parallel-loops-to-gpu"
            if target == "async":
                lower_pass = "-async-parallel-for"
            if target == "spirv":
                lower_pass = "-convert-math-to-spirv"
            if target == "gpu.launch_func":
                lower_pass = "-gpu-kernel-outlining"
            if target == "omp":
                lower_pass = "-convert-scf-to-openmp"
            if target == "nvvm":
                lower_pass = "--convert-gpu-to-nvvm"
            if target == "rocdl":
                lower_pass = "--convert-gpu-to-rocdl"
            if target =="":
                lower_pass = setLowerSeq(lowerPass)

            flag = "lower"
            result = execute_mlir(seed_file, lower_file, sid, raw_mlir, lower_pass, conf, flag,[])
            from fuzz.fuzz import Fuzz
            Fuzz.FuzzingSave(result,flag,conf,dialects,operation)
            # 如果运行结果没有报错，则将新生成的种子存入seed_pool，需要存入方言，测试用例，n，候选降级pass
            if (result["return_code"] == 0) :
                analysis_and_save_seed(seed_file, lower_file, result, conf, flag)
            else:
                log.error("current seed need to analysis，without dialects and lowerPass")

            log.info("================== Enable optimization ====================")
            flag = "opt"
            opt_pass= setOptSeqR()
            result = execute_mlir(seed_file, opt_file, sid, raw_mlir, opt_pass, conf,flag,[])
            Fuzz.FuzzingSave(result,flag,conf,dialects,operation)
            if (result["return_code"] == 0):
                analysis_and_save_seed(seed_file,opt_file,result, conf,flag)
        

    def DirectedLower(self,conf,seed_file,lower_file,opt_file,Mut,mut_file):
        
        dia = "tosa"
        target ="linalg"
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)

        dia ="linalg"
        target ="affine"
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)

        dia ="linalg"
        target ="scf.parallel"
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)

        dia ="affine"
        target ="affine.parallel"
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)

        dia ="affine"
        target ="scf.for"
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)


        dia ="scf.parallel"
        target ="gpu"
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)

        dia ="gpu"
        target ="async"
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)

        dia ="math"
        target ="spirv"
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)
        dia ="gpu.launch"
        target ="gpu.launch_func"
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)

        dia ="scf.parallel"
        target ="omp"
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)

        dia =""
        target =""
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)
        # dia ="vector"
        # target ="x86vector"
        # DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file)

        dia ="gpu.launch"
        target ="nvvm"
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)

        dia ="gpu.launch"
        target ="rocdl"
        log.info(dia + "-->" + target)
        DTFuzz.generateLowIR(self,dia,target,conf,seed_file,lower_file,opt_file,Mut,mut_file)

    def process(self,Mut):
        conf = self.config
        log.info("Iter :"+ str(conf.Iter))
        seed_file = conf.temp_dir + "seed" + ".mlir"

        if not os.path.exists(seed_file):
            os.system(r"touch {}".format(seed_file))
        mut_file = conf.temp_dir + "mut" + ".mlir"
        opt_file = conf.temp_dir + "opt" + ".mlir"
        lower_file = conf.temp_dir + "lower" + ".mlir"

        # 梳理流程：
        # 1.连数据库，选种子（暂定随机选）
        # 2.判断种子是否有候选降级pass
        # 2.1 有candidate_lower_pass
        # 2.1.1 直接组合优化降级pass，跑单步pass，结果存入result，新种子（跑的结果）存入seed_pool，并且分析新种子的方言以及候选降级pass，一起存入数据库
        # 2.2 无candidate_lower_pass
        # 2.2.1 有问题，存在种子池的种子一定要有方言级别和候选降级pass
        log.info("================== select seed at random====================")

        # 对图级IR降级
        # self.LowerGraphIR(conf,seed_file,lower_file)

        dia = "affine"
        seeds = self.select_seed_DT_limit(conf.Nmax,dia,50)
        # if len(seeds)==0:
        #     print("generate seed")
        for selected_seed in seeds:
            sid = selected_seed[0]
            dialects,operation,raw_mlir,n,lowerPass = selected_seed[-5:]

            dialect_list = dialects.split(',')
            OPdict = {key: [] for key in dialect_list}
            if operation !=' ':
                word_list = operation.split(',')
                for item in word_list:
                    d1, d2 = item.split('.',1)
                    if d1 in OPdict:
                        if d2 not in OPdict[d1]:
                            OPdict[d1].append(d2)
                
            # 向input_file写入初始种子
            f = open(seed_file, 'w', encoding="utf-8")  # w 的含义为可进行读写
            f.write(raw_mlir)  # file.write()为写入指令
            f.close()

            log.info(sid)
            # log.info(raw_mlir)

            if(Mut=='1'):  # 是否启用变异
                log.info("================== Enable mutation ====================")
                flag = "mutate"
                mutate_pass = select_emiPass(dialects,operation)

                if(mutate_pass[0]!=""):
                    result = execute_mlir(seed_file, mut_file, sid, raw_mlir, mutate_pass, conf, flag,OPdict)
                    if (result["return_code"] == 0) :
                        analysis_and_save_seed(seed_file,mut_file,result, conf,flag)

        
            log.info("================== Enable optimization ====================")
            flag = "opt"
            opt_pass= setOptSeq(dia)
            result = execute_mlir(seed_file, opt_file, sid, raw_mlir, opt_pass, conf,flag,[])
            if (result["return_code"] == 0):
                analysis_and_save_seed(seed_file,opt_file,result, conf,flag)


            log.info("================== Enable Lowering ====================")
            flag = "lower"
            lower_pass = setLowerSeq(lowerPass)
            result = execute_mlir(seed_file, lower_file, sid, raw_mlir, lower_pass, conf, flag,[])
            # 如果运行结果没有报错，则将新生成的种子存入seed_pool，需要存入方言，测试用例，n，候选降级pass
            if (result["return_code"] == 0) :
                analysis_and_save_seed(seed_file, lower_file, result, conf, flag)
            else:
                log.error("current seed need to analysis，without dialects and lowerPass")


    def affineGen(self):
        # sql = "select content FROM seed_pool_MLIRGen where dialect like 'tosa'" 
        sql = "select content FROM tosa_testcase" 
        
        # sql = "select content FROM seed_pool_Fuzzer0628_copy1" 
        # -convert-linalg-to-parallel-loops
        dataList = dbutils.db.queryAll(sql)
        conf = self.config
        i = 0
        for data in dataList:
            seed_file = conf.temp_dir + "seed" + ".mlir"
            if not os.path.exists(seed_file):
                os.system(r"touch {}".format(seed_file))
            
            # 向input_file写入初始种子
            f = open(seed_file, 'w', encoding="utf-8")  # w 的含义为可进行读写
            f.write(data[0])  # file.write()为写入指令
            f.close()

            i = i+1
 
            log.info("======generate seed : " + str(i))
            target_file = conf.temp_dir + str(i)+ ".mlir"

    
            singlePass = "-pass-pipeline=\"builtin.module(func.func(tosa-to-linalg-named,tosa-to-linalg))\" -linalg-bufferize -convert-linalg-to-affine-loops -affine-parallelize"
            
            # singlePass = "-linalg-bufferize -convert-linalg-to-affine-loops"
            cmd = '%s %s -allow-unregistered-dialect %s -o %s' % (conf.mlir_opt, seed_file, singlePass, target_file)
        
            #  shell 为true，执行shell内置命令  subprocess.PIPE 表示为子进程创建新的管道
            pro = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, universal_newlines=True, encoding="utf-8")
            try:
                stdout, stderr = pro.communicate(timeout=5)
                returnCode = pro.returncode
                log.info(cmd)
                if not os.path.exists(target_file):
                    # 报错不会生成, 打印错误日志
                    log.error(stderr)
                    continue
                from generator.tosaGen import seedAnalysis
                dialects,candidate_lower_pass,operations = seedAnalysis(conf,target_file)
                log.info(candidate_lower_pass)
                log.info(operations)
                with open(target_file, 'r') as f:
                    content = f.read()
                log.info(content)
                try:
                    sql = "insert into "+ conf.seed_pool_table + \
                        " (preid,source,mtype,dialect,operation,content,n, candidate_lower_pass) " \
                        "values ('%s','%s','%s','%s','%s','%s','%s','%s')" \
                        % \
                        (0,'L4','',dialects, operations,content, 0, candidate_lower_pass)
                    dbutils.db.executeSQL(sql)
                    # count = count+ 1object
                except Exception as e:
                    log.error('sql error', e)
                os.remove(target_file)
             
            except subprocess.TimeoutExpired:
                pro.kill()
                stdout = ""
                stderr = "timeout, kill this process"
                returnCode = -9

            if i==800:
                break


