from z3 import *
import numpy as np
a = Real('a')#定义一个整形 a
b = Real('b')#定义一个整形 b
c = Real('c')#定义一个整形 a
d = Real('d')#定义一个整形 b
# set_option(rational_to_decimal=True)
s = Solver()#生成一个约束求解器
s.add(a*37 + b*5 + c*3 + d*1 == 1)
s.add(a-b>0)
s.add(b-c>0)
s.add(c-d>0)
s.add(d>0)

print(s.check())#检查约束求解器是否有解，如果有,返回sat; 如果不满足,返回unsat
print(s.model())#输出结果


