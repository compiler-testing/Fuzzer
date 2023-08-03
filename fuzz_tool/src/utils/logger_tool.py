import logging
import time
import os
log=None


def get_logger():   #log_dir:'/..xmr/llvm-project-15/mlir/mytest/fuzz_tool/logs/'
    global log
    if log is not None :
        return log
    logger = logging.getLogger()  # 获取日志器组件
    curdir = os.getcwd()
    log_dir = os.path.dirname(curdir) + '/logs/'
    if not os.path.exists(log_dir):
        os.mkdir(log_dir)
    print(log_dir)
    if not logger.handlers:  # 判断当前日志器对象是否拥有处理器组件

        logger.setLevel(logging.DEBUG)
        # 构建处理器对象: 文件输出流   控制台输出流
        uuid_str = time.strftime("%Y-%m-%d-%H", time.localtime())
        log_file = '%s%s.txt' % (log_dir, uuid_str)
        fh = logging.FileHandler(filename=log_file, encoding="utf8")
        sh = logging.StreamHandler()

        # 构建格式化组件
        fmt = logging.Formatter(fmt="[%(asctime)s]- %(levelname)s (%(lineno)s): %(message)s ", datefmt="%Y/%m/%d %H:%M:%S")

        # 处理器组件添加格式化组件
        fh.setFormatter(fmt)
        sh.setFormatter(fmt)

        sh.setLevel(logging.INFO)
        # 日志器对象添加处理器对象
        logger.addHandler(fh)
        logger.addHandler(sh)
        log = logger
