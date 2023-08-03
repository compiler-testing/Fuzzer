import yaml
import os


def load_conf(conf_path):
    with open(conf_path, 'r',encoding='utf-8') as f:
        content = f.read()
    conf = yaml.load(content, Loader=yaml.FullLoader)
    return conf


class Config:
    def __init__(self, conf_path,project_path,sqlName,enbaleTS):
        conf = load_conf(conf_path)
        # 1. database config
        database = conf['database']
        self.host = database['host']
        self.port = database['port']
        self.username = database['username']
        self.passwd = database['passwd']
        self.db = database['db']
        label = sqlName
        self.seed_pool_table = 'seed_pool_' + label
        self.result_table = 'result_' + label
        self.report_table = 'report_' + label
        # 2. common config
        common = conf['common']
        mlir_base_path = project_path +common['mlir_base_path']
        self.mlir_opt = mlir_base_path + common['mlir_opt']
        self.temp_dir = mlir_base_path + common['temp_dir']+ label+'/'

        if not os.path.exists(self.temp_dir):
            os.makedirs(self.temp_dir)
        # 3. generator config
        generator = conf['generator']
        self.empty_func_file = mlir_base_path + generator['empty_func_file']
        self.count = generator['count']
        if enbaleTS=="1":
            self.type = "o"
        else:
            self.type = "u"
        # self.type = generator['type']
        # 4. fuzz config
        fuzz = conf['fuzz']
        self.run_time = fuzz['run_time']
        self.analysis_seed_file = mlir_base_path + fuzz['analysis_seed_file']
        self.Nmax = fuzz['Nmax']
        self.mutate_flag = fuzz['mutate_flag']
        self.flag_mutate = fuzz['flag_mutate']
        self.flag_VPA = fuzz['flag_VPA']
        self.threshold = fuzz['diff_threshold']

        self.Iter = 0

global Iter




