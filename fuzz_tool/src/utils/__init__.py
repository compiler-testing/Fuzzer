from utils import *

__all__ = ['dbutils', 'config', 'logger_tool']


def read(file_name, mode='rb', encoding=None):
    with open(file_name, mode, encoding=encoding) as f:
        return f.read()
