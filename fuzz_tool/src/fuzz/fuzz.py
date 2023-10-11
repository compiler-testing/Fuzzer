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
            log.info("===IRanalysis")
            # 读取temp.json文件，获取当前mlir文本中的方言和降级pass
            lowerPasses = lowerPasses.replace('  ', ' ')
            fuzzer = Fuzz(config)
            Fuzz.mutate_success_handler(fuzzer, result["sid"], result["mutate_type"], dialects,operations, processed_mlir,
                                                lowerPasses,flag)
        
        if flag=="mutate":
            with open(seed_file, 'w') as f:
                f.write(processed_mlir)

def setOptSeqRR():
    """
    1.启用脆弱性分析：优化序列：VP + 随机优化
    2.随机优化
    """
    list = []
    list.extend(get_rand_pass_pipe(AllPass.all,10))
    random.shuffle(list)
    passSeq = " ".join(list)
    pass_list = []
    pass_list.append(passSeq)

    return pass_list

def setOptSeqR():
    """
    1.启用脆弱性分析：优化序列：VP + 随机优化
    2.随机优化
    """
    list = []
    list.extend(get_rand_pass_pipe(OptimizePass.all,20))
    random.shuffle(list)
    passSeq = " ".join(list)
    pass_list = []
    pass_list.append(passSeq)

    return pass_list
def setLowerSeqR():
    # log.info("======combine candidates and randPass")
    list = []
    randoms = get_rand_pass_pipe(LowerPass.all,1)
    list.extend(randoms)
    # random.shuffle(list)
    passSeq = " ".join(list)
    pass_list = []
    pass_list.append(passSeq)
    return pass_list

def setLowerSeqOne(lower_pass):
    # log.info("======combine candidates and randPass")
    list = []
    randoms = get_rand_pass_pipe(str_to_list(lower_pass),1)
    # randoms = get_rand_pass_pipe(LowerPass.all,3)
    # randoms = get_rand_pass_pipe(LowerPass.all,1)

    for i, x in enumerate(randoms):
        if x.find("linalg-to") != -1:
            # print(randoms[i])
            randoms[i] = "-func-bufferize -linalg-bufferize " + randoms[i]

    for i, x in enumerate(randoms):
        if x.find("convert-parallel-loops-to-gpu") != -1:
            # print(randoms[i])
            randoms[i] = "-gpu-map-parallel-loops " + randoms[i]

    list.extend(randoms)
    # random.shuffle(list)
    passSeq = " ".join(list)
    pass_list = []
    pass_list.append(passSeq)
    return pass_list


def setLowerSeqC(lower_pass):
    # log.info("======combine candidates and randPass")
    list = []
    candidates = get_rand_pass_pipe(str_to_list(lower_pass),5)
    randoms = get_rand_pass_pipe(LowerPass.all,3)

    for i, x in enumerate(candidates) :
        if x.find("linalg-to")!=-1:
            # print(candidates[i])
            candidates[i] = "-linalg-bufferize " +candidates[i]   #-func-bufferize 

    for i, x in enumerate(randoms):
        if x.find("linalg-to") != -1:
            # print(randoms[i])
            randoms[i] = "-linalg-bufferize " + randoms[i]

    for i, x in enumerate(candidates) :
        if x.find("convert-parallel-loops-to-gpu")!=-1:
            # print(candidates[i])
            candidates[i] = "-gpu-map-parallel-loops " +candidates[i]   #-func-bufferize 

    for i, x in enumerate(randoms):
        if x.find("convert-parallel-loops-to-gpu") != -1:
            # print(randoms[i])
            randoms[i] = "-gpu-map-parallel-loops " + randoms[i]

    list.extend(candidates)
    list.extend(randoms)

    # random.shuffle(list)
    pass_list = []
    # pass_list.append(" ".join(list))
    pass_list.append(" ".join(list))
    pass_list.append(" ".join(candidates))

    return pass_list


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
    # log.info(singlePass)
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

def execute_pass(input_file,output_file, result, singlePass, config:Config,flag):
    # log.info("========= execute pass ========")

    # 文件写入，初始化写入raw_file
    # 变异后刷新raw_file,raw_file是原始mlir或者变异mlir，保存
    # 启用优化；生成opt_mlir,保存
    # 启用降级：对raw_file进行降级

    if singlePass.find("func.func") >= 0:
        newpass = []
        newpass = fixlowerpass(singlePass)
        cmd_ = '%s %s -allow-unregistered-dialect %s' % (config.mlir_opt, input_file, newpass[0])
        cmd1 = cmd_
        if len(newpass)>1:
            for seg in newpass[1:]:
                cmd2 = '| %s -allow-unregistered-dialect %s' % (config.mlir_opt, seg)
                cmd1 = cmd1 + cmd2
            cmd_ = cmd1
        cmd=  cmd_  + ' -o %s' % (output_file)
    else:
        cmd = '%s %s -allow-unregistered-dialect %s -o %s' % (config.mlir_opt, input_file, singlePass, output_file)
    
     
    s1 = cmd.split("seed.mlir ")[1]
    save_pass = s1.split(" -o")[0]

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

    result["cmd"] = save_pass
    result["return_code"] = return_code
    result["stdout"] = stdout
    result["stderr"] = stderr
    result["duration"] = duration

    return result

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
    
    s1 = cmd.split("seed.mlir ")[1]
    save_pass = s1.split(" -o")[0]

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
        "cmd":save_pass,
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

def execute_mlir(input_file, output_file, sid, raw_mlir, pass_list, config: Config, flag,OPdict) -> str:
    """
    执行编译
    """
    result1 = {
        "sid":sid,
        "input":raw_mlir,
        "mutate_type":"",
        "cmd":"",
        "return_code":"",
        "stdout":"",
        "stderr":"",
        "duration":"",
    }

    if (flag == "mutate"):
        result1["mutate_type"] = pass_list[0]

    if pass_list[0]=="-Rep":
        print("=============RepMut============")
        return RepMut(output_file,result1,OPdict)
    
    
    # 执行mlir
    result = execute_pass(input_file, output_file, result1, pass_list[0], config,flag)
    # result = execute_pass(input_file, output_file, sid, raw_mlir, pass_list[0], config,flag)
    fuzzer = Fuzz(config)


    if(flag!="mutate"):
        if (result["return_code"] != 0): 

            # 运行结果出错，存入数据库，reslut
            Fuzz.failer_handler(fuzzer, result,flag,config)

            if flag == "lower" and len(pass_list)!=1:
                log.info("======only run lowering pass")
                result = runPass_OnlyLower(input_file, output_file, result, pass_list[1], config, flag)  #只跑降级pass
      

    return result  # 跑完命令之后，产生的mlir文本



def Mutator(input_file, output_file, sid, raw_mlir, mflag, config: Config,dialects,operation) -> str:
    """
    变异
    """
    if mflag=='1':
        mtype = '-Mix'
    elif mflag=='2':
        mtype = '-Rep'
    elif mflag=='3':
        mtype = '-ALL'

    result = {
        "sid":sid,
        "input":raw_mlir,
        "mutate_type":"",
        "cmd":"",
        "return_code":1,
        "stdout":"",
        "stderr":"",
        "duration":"",
    }


    if  mflag=='2' and "affine.load" in operation:
        print("=============RepMut============")
        result["mutate_type"] = "Rep"
        return RepMut(output_file,result,operation)
    
    if  mflag!='2':
        if "linalg" in dialects: 
            print("=============MixMut============")

            cmd = '%s %s %s -o %s' % (config.mlir_opt, input_file, "-Mix", output_file)
    
            start_time = int(time() * 1000)
            #  shell 为true，执行shell内置命令  subprocess.PIPE 表示为子进程创建新的管道
            pro = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, universal_newlines=True, encoding="utf-8")
            try:
                stdout, stderr = pro.communicate(timeout=10)
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
            result["mutate_type"] = "Mix"

            if  mflag=='3' and "affine.load" in operation:
                result["mutate_type"] = "ALL"
                print("=============RepMut============")
                return RepMut(output_file,result,operation)

    return result


def runPass_OnlyLower(input_file, output_file, result, candidate_lowerPass, config: Config,flag):
    result = execute_pass(input_file, output_file, result, candidate_lowerPass, config,flag)
    fuzzer = Fuzz(config)
    if (result["return_code"] != 0):  # 运行结果出错，存入数据库，reslut
        Fuzz.failer_handler(fuzzer, result,flag,config)

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

def select_emiPass(dialects,operation,mflag)->str: 
    emi_pass = "" 
    if mflag=='2':
        if "affine.load" in operation:
            emi_pass = "-Rep"
    if mflag=='1':
        if "affine.load" in operation:
            emi_pass = "-Mix"

    if mflag=='2':
        if "affine.load" in operation:
            emi_pass = "-Mix -Rep"

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






class Fuzz:
    def __init__(self, config: Config):
        self.config = config


    def query_result(self,sid):
        log.info("================== get trigger pass====================")
        sql = "select * from " + self.config.result_table + " where id = '%s' ORDER BY rand() limit 1 " % sid
        
        sql_result = dbutils.db.queryAll(sql)[0]
        return sql_result[1],sql_result[3],[sql_result[6]]  # 一行数据

    def select_seed(self,Nmax):
        log.info("================== select seed at random====================")
        sql = "select * from " + self.config.seed_pool_table + " where n < '%s' ORDER BY rand() limit 1 " % Nmax
        # sql = "select * from " + self.config.seed_pool_table + " where sid = 211815" 
        # sql = "update " + self.config.seed_pool_table + " set n = 0 where sid = '%s'" % sid
        seed_types = dbutils.db.queryAll(sql)
        return seed_types  # 一行数据
    
    def select_seed_sid(self,sid):
        log.info("================== select seed with sid====================")
        sql = "select * from " + self.config.seed_pool_table + " where sid = '%s' ORDER BY rand() limit 1 " % sid
        print(sql)
        seed_types = dbutils.db.queryAll(sql)
        return seed_types  # 一行数据

    def select_seed_DT(self,Nmax,dia):
        sql = "select * from " + self.config.seed_pool_table + " where n < '%s' and dialect like '%%" % Nmax +dia+"%%' ORDER BY rand()" 
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
            MLIR_phase = "O"
        elif (flag=="mutate"):
            MLIR_phase = "M"
        elif (flag == "lower"):
            MLIR_phase = "L"
        try:
            sql = "insert into " + self.config.seed_pool_table + \
                  " (preid,source,mtype,dialect,operation, content,n, candidate_lower_pass) " \
                  "values ('%s','%s','%s','%s','%s','%s','%s','%s')" \
                  % \
                  (sid,MLIR_phase,mtype,dialects, operations,content, 0, candidate_lower_pass)

            dbutils.db.executeSQL(sql)
            # log.info(content)
            log.info("======save the result as seed")
        except Exception as e:
            log.error('sql error', e)



    def SplitContent(self,content:str)->str:
        content_list = content.split(("\n"))
        if content.find("LLVM ERROR:") >= 0:
            for item in content_list:
                if item.find("LLVM ERROR:") >= 0:
                    errorMessage = "LLVM ERROR:" + item.split("LLVM ERROR:")[1]


        errorFunc = ''
        errorMessage = ''
        if content.find("Assertion") >= 0:
            for i in range(0,len(content_list)-1):
                if content_list[i].find("Assertion") >= 0:
                    num = re.findall(r':(\d+):', content_list[i])
                    errorMessage = content_list[i].split(num[0]+': ')[1]   
                    break


        if content.find("Segmentation fault") >= 0:
            for i in range(0,len(content_list)-1):
                if content_list[i].find("__restore_rt") >= 0:
                    errorFunc = content_list[i+1]  #提取__restore_rt下面的一行
                    errorFunc = errorFunc.split("(/home")[0]  #去掉报错的地址
                    errorFunc = errorFunc[22:]
                    break
            errorMessage = "Segmentation fault:" + errorFunc
        return errorMessage
        
    def updateReportModel(self,MLIR_phase,error,report_stack_dict,stderr,return_code,content,report_model_dict):
        #error = error.replace('\\\'','\'')
        error = error.replace('\'','')
        if error in report_stack_dict and return_code!=-9 and error.find("PLEASE submit a bug report")<=0:
            log.info("===== update sids in report table =====")

        else:
            log.info("===== add new report record in reportObject =====")
            model = reportObject(ids=MLIR_phase, stderror=stderr, returnCode=return_code, mlirContent=content)
            report_model_dict[error] = model
    
    #TODO
    def isNew(self,error):
        return '0'
    
    def bugReport(self,MLIR_phase,return_code,stderr,content,passes,conf):
        log.info("===== starting analysis error =====")
        sql = "select * from " + self.config.report_table + " where stderr is not NULL and stderr != ''"
        data = dbutils.db.queryAll(sql)
        report_model_dict = {}   
        report_stack_dict = []  #存放当前repor表中的所有的stack信息
        for item in data:
            report_stack_dict.append(item[0])

        #1. bug信息提取
        firstLineInStderr = stderr.split("\n")[0] #提取错误信息的第一行
        error = ''
        #crash & segmentation fault
        if stderr.find("Assertion") >= 0 or stderr.find("LLVM ERROR:") >=0 or stderr.find("Segmentation fault (core dumped)")>=0:
            error = Fuzz.SplitContent(self,stderr)
        else:  # time out & others
            error = firstLineInStderr

        #2. 保存bug
        #2.1 过滤已保存的bug
        Fuzz.updateReportModel(self,MLIR_phase,error,report_stack_dict,stderr,return_code,content,report_model_dict)
        for key,value in report_model_dict.items():
            log.info("===== new report record insert table =====")
            now = datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d %H:%M:%S')
            #key = key.replace('\'', '\\\'')
            returnCode = value.returnCode
            #stderr = value.stderror.replace('\'', '\\\'')
            mlirContent = value.mlirContent.replace('\'', '\\\'')
            # stderr = stderr.replace('\'','')
            
            #TODO 识别该bug是否为未曾报告的bug  new=1表示新的bug
            new = Fuzz.isNew(self,error)

            #2.2 保存bug到report_table
            try:
                sql = "insert into " + self.config.report_table  + \
                " (new,stack,phase,datetime,stderr,returnCode,mlirContent,passes) " \
                " values('%s','%s','%s','%s','%s','%s','%s','%s')" \
                % \
                (new,key, MLIR_phase, now, stderr, returnCode, mlirContent,passes)
                # log.info("===== sql: " + str(sql) )
                dbutils.db.executeSQL(sql)
                log.info("======report this error successfully")
            except Exception as e:
                log.error('sql error', e)

    def failer_handler(self,result,flag,conf):
        result_list = list(result.values());
        sid, content = result_list[0:2]
        if (flag=="opt"):
            MLIR_phase = "O"
        elif (flag=="mutate"):
            MLIR_phase = "M"
        elif (flag == "lower"):
            MLIR_phase = "L"

        passes, return_code, stdout,stderr, duration = result_list[3:]

        returnCode_list = [-9,134,139]

        if return_code in returnCode_list:
            Fuzz.bugReport(self,MLIR_phase,return_code,stderr,content,passes,conf)

        self.update_nindex(sid,False)

    def FuzzingSave(result,flag,conf,dialects,operation):
        result_list = list(result.values());
        sid, content = result_list[0:2]
        if (flag=="opt"):
            MLIR_phase = "O"
        elif (flag=="mutate"):
            MLIR_phase = "M"
        elif (flag == "lower"):
            MLIR_phase = "L"

        cmd, return_code, stdout,stderr, duration = result_list[3:]

        now = datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d %H:%M:%S')
        try:
            sql = "insert into  " + conf.result_table + \
                  " (sid, content,phase,dialect,operation,passes,returnCode,stdout,stderr,duration,datetime) " \
                  "values ('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')" \
                  % \
                  (sid, content, MLIR_phase,dialects,operation, cmd, return_code, stdout, stderr, duration, now)
            # log.info(sql)
            dbutils.db.executeSQL(sql)
            log.info("======save result")
        except Exception as e:
            log.error('sql error', e)




    def FuzzingSave111(result,flag,conf,dialects,operation,passes):
        result_list = list(result.values());
        sid, content = result_list[0:2]
        if (flag=="opt"):
            MLIR_phase = "O"
        elif (flag=="mutate"):
            MLIR_phase = "M"
        elif (flag == "lower"):
            MLIR_phase = "L"

        cmd, return_code, stdout,stderr, duration = result_list[3:]

        now = datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d %H:%M:%S')
        try:
            sql = "insert into  " + conf.result_table + \
                  " (sid, content,phase,dialect,operation,passes,returnCode,stdout,stderr,duration,datetime) " \
                  "values ('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')" \
                  % \
                  (sid, content, MLIR_phase,dialects,operation, passes, return_code, stdout, stderr, duration, now)
            # log.info(sql)
            dbutils.db.executeSQL(sql)
            log.info("======save result")
        except Exception as e:
            log.error('sql error', e)

    def update_nindex(self, sid, reset=False):
        if reset:
            sql = "update " + self.config.seed_pool_table + " set n = 0 where sid = '%s'" % sid
            dbutils.db.executeSQL(sql)
        else:
            sql = "update " + self.config.seed_pool_table + " set n = n +1 where sid = '%s'" % sid
            dbutils.db.executeSQL(sql)

    
    def process(self,Mut,DT):
        conf = self.config
        log.info("Iter :"+ str(conf.Iter))
        seed_file = conf.temp_dir + "seed" + ".mlir"

        if not os.path.exists(seed_file):
            os.system(r"touch {}".format(seed_file))
        mut_file = conf.temp_dir + "mut" + ".mlir"
        opt_file = conf.temp_dir + "opt" + ".mlir"
        lower_file = conf.temp_dir + "lower" + ".mlir"
        
        from fuzz.directed import DTFuzz
        if(DT=="dt"):
            log.info("Directed Testing")
            DTFuzz.DirectedLower(self,conf,seed_file,lower_file,opt_file,Mut,mut_file)
     
        seeds = self.select_seed(conf.Nmax)
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

            if Mut=='1':
                Mut = np.random.choice(['0','1','0'])

            if(Mut!='0'):  # 是否启用变异
                log.info("================== Enable mutation ====================")
                flag = "mutate"
                # mutate_pass = select_emiPass(Mut,dialects,operation)

                result = Mutator(seed_file, mut_file, sid, raw_mlir, Mut, conf, dialects,operation)
                if (result["return_code"] == 0) :
                    analysis_and_save_seed(seed_file,mut_file,result, conf,flag)

            #完全随机的选择
            if(DT=="rr"):
                log.info("================== Enable optimization ====================")
                flag = "opt"
                #待修改
                opt_pass= setOptSeqRR()
                result = execute_mlir(seed_file, opt_file, sid, raw_mlir, opt_pass, conf,flag,[])
                Fuzz.FuzzingSave(result,flag,conf,dialects,operation)
                if (result["return_code"] == 0):
                    analysis_and_save_seed(seed_file,opt_file,result, conf,flag)

            #每次选择
            if(DT=="r"):
                log.info("================== Enable optimization ====================")
                flag = "opt"
         
                opt_pass= setOptSeqR()
                result = execute_mlir(seed_file, opt_file, sid, raw_mlir, opt_pass, conf,flag,[])
                # Fuzz.FuzzingSave(result,flag,conf,dialects,operation)
                Fuzz.FuzzingSave111(result,flag,conf,dialects,operation,opt_pass[0])
                if (result["return_code"] == 0):
                    analysis_and_save_seed(seed_file,opt_file,result, conf,flag)

                log.info("================== Enable Lowering ====================")
                flag = "lower"
                lower_pass = setLowerSeqR()
                result = execute_mlir(seed_file, lower_file, sid, raw_mlir, lower_pass, conf, flag,[])
                Fuzz.FuzzingSave111(result,flag,conf,dialects,operation,lower_pass[0])
                # 如果运行结果没有报错，则将新生成的种子存入seed_pool，需要存入方言，测试用例，n，候选降级pass
                if (result["return_code"] == 0) :
                    analysis_and_save_seed(seed_file, lower_file, result, conf, flag)
                else:
                    log.error("current seed need to analysis，without dialects and lowerPass")

            #每次选择一个
            if(DT=="c"): 
                log.info("================== Enable optimization ====================")
                flag = "opt"
                opt_pass= setOptSeqR()
                result = execute_mlir(seed_file, opt_file, sid, raw_mlir, opt_pass, conf,flag,[])
                Fuzz.FuzzingSave(result,flag,conf,dialects,operation)
                if (result["return_code"] == 0):
                    analysis_and_save_seed(seed_file,opt_file,result, conf,flag)

                log.info("================== Enable Lowering ====================")
                flag = "lower"
                lower_pass = setLowerSeqOne(lowerPass)
                result = execute_mlir(seed_file, lower_file, sid, raw_mlir, lower_pass, conf, flag,[])
                Fuzz.FuzzingSave(result,flag,conf,dialects,operation)
                # 如果运行结果没有报错，则将新生成的种子存入seed_pool，需要存入方言，测试用例，n，候选降级pass
                if (result["return_code"] == 0) :
                    analysis_and_save_seed(seed_file, lower_file, result, conf, flag)
                else:
                    log.error("current seed need to analysis，without dialects and lowerPass")


    def debug(self):
        result_id = "204889"

        sid,phase,trigger_pass= self.query_result(result_id)
        
        conf = self.config
        log.info("Iter :"+ str(conf.Iter))
        seed_file = conf.temp_dir + "seed" + ".mlir"

        if not os.path.exists(seed_file):
            os.system(r"touch {}".format(seed_file))
        mut_file = conf.temp_dir + "mut" + ".mlir"
        output_file = conf.temp_dir + "opt" + ".mlir"
        lower_file = conf.temp_dir + "lower" + ".mlir"
        
        seeds = self.select_seed_sid(sid)
        selected_seed = seeds[0]
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

     
            log.info("================== Enable optimization ====================")
            
            if phase=='O':
                flag = "opt"
            else:
                flag = "lower"

            result = execute_mlir(seed_file, output_file, sid, raw_mlir, trigger_pass, conf,flag,[])
            # Fuzz.FuzzingSave(result,flag,conf,dialects,operation)
            Fuzz.FuzzingSave111(result,flag,conf,dialects,operation,trigger_pass[0])
            if (result["return_code"] == 0):
                analysis_and_save_seed(seed_file,output_file,result, conf,flag)

