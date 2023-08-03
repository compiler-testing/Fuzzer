import pymysql
from utils.config import Config
from utils.logger_tool import log

db=None


class myDB:

    def __init__(self, config: Config):
        self.host = config.host
        self.username = config.username
        self.password = config.passwd
        self.port = config.port
        self.database = config.db

    def connect_mysql(self):
        '''
        连接数据库
        '''
        # 连接数据库
        self.db = pymysql.connect(host=self.host, port=self.port, user=self.username, password=self.password,
                                  database=self.database)
        # 使用cursor()方法创建一个游标对象
        self.cursor = self.db.cursor()

    def executeSQL(self, sql):
        """
        数据库操作:增删改查
        :param sql: insert / update / delete
        :return:
        """
        try:
            self.connect_mysql()
            # 使用execute()方法执行SQL语句
            self.cursor.execute(sql)
            self.db.commit()
        except Exception as e:
            log.info("SQL ERROR:=======>", e)
            log.info("wrong SQL:=======>", sql)
            # print("SQL ERROR:=======>", e)
            # print(sql) 
        finally:
            self.close()

    def queryAll(self, sql):
        '''
        查询所有数据
        :param sql: select
        :return:
        '''
        self.connect_mysql()
        self.cursor.execute(sql)
        data = self.cursor.fetchall()
        self.close()
        return data

    def queryOne(self, sql):
        '''
        查询所有数据
        :param sql: select
        :return:
        '''
        self.connect_mysql()
        self.cursor.execute(sql)
        data = self.cursor.fetchone()
        self.close()
        return data  #类型是tuple

    def close(self):
        '''
        关闭cursor和connection连接
        '''
        self.cursor.close()
        self.db.close()
