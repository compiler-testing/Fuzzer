#test.py
print('Hello World!')
 
def aaa():
    print('this message is from aaa  function')
 
def main():
    path = 'mlir/mytest/fuzz_tool/src/utils/passinfo.txt'
    listOfLines = list()
    with open(path, 'r', encoding='utf8') as file:
        passinfo = file.read()
      
    lines = passinfo.split("--")
    passlist = []
    for line in lines:
        if line.find('     -')!=-1 and line.find('=')==-1 :
            temp = line.split('      -')[0].strip()
            single = '\n\'-'+ temp+'\'';
            passlist.append(single)
            print(single)
       # break
    
    passStr = ','.join(passlist)
    print(len(lines))
    print(len(passlist))
    print(passStr)
if __name__ == '__main__':
    main()