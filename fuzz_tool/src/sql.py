import sqlite3
conn = sqlite3.connect('../conf/random1.db')

cur = conn.cursor()
sql_text_2 = "INSERT INTO seed_pool VALUES('A', '一班', '男', 96, 94, 98)"
cur.execute(sql_text_2)

data = [('B', '一班', '女', 78, 87, 85),
        ('C', '一班', '男', 98, 84, 90),
        ]
cur.executemany('INSERT INTO scores VALUES (?,?,?,?,?,?)', data)

sql_text_3 = "SELECT * FROM scores WHERE 数学>90"
cur.execute(sql_text_3)
# 获取查询结果
cur.fetchall()