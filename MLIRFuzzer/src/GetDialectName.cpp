
#include "TosaGen/utils.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"
#include <iostream>
#include <fstream>

#include <map>
#include "json/json.h"

#include <unistd.h>
#include "stdlib.h"
#include <stdio.h>
#include <string.h>

using namespace mlir;
using namespace std;



// extern TosaGenUtils genUtils;
//先不用
//void writeJson(string lobal,llvm::SmallVector<string> content)
//{
////  ofstream ofile;
////  ofile.open("/home/xmr/llvm-15/mlir/test/temp.json");
//
//  Json::Value root;
//  for (int i = 0; i < content.size(); ++i) {
//    root[lobal].append(content[i]);
//  }
//  ofstream os;
//  os.open("/home/xmr/llvm-project-15/mlir/test/temp.json", std::ios::out | std::ios::app);
//  if (!os.is_open())
//    cout << "error：can not find or create the file which named \" demo.json\"." << endl;
//  Json::StyledWriter sw;  //缩进输出
//  os << sw.write(root);
//  os.close();
//}
//

extern Utils genUtils;

namespace {
struct GetDialectName
    : public PassWrapper<GetDialectName, OperationPass<ModuleOp>> {  //func::FuncOp>
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(GetDialectName)
  void runOnOperation() override {

    llvm::SmallVector<string> dialects;  //mlir文本中提取的方言名
    llvm::SmallVector<string> operations; 
    llvm::SmallVector<string> lower_ops;      //要降级的op
    llvm::SmallVector<string> lower_pass;  //最终匹配的Pass
    llvm::SmallVector<string> disable;  //无效的pass
    //获得当前文件的绝对路径目录,用于下面的json文件路径
    //    char *path = NULL;
    //    path = getcwd(NULL,0);   // cout<<path: /home/xmr/llvm-project-15/mlir/test
    //    string current_dir;
    //    current_dir = path;
    //    free(path);

    char szPath[128];
    memset( szPath, 0x00, sizeof(szPath));
    // int ret =  readlink("/proc/self/exe", szPath, sizeof(szPath)-1 );
    int ret =  readlink("/proc/self/exe", szPath, 256);
    printf("ret:%d\n", ret);
    printf("path:%s\n", szPath);

    char *token= NULL;
    token = strtok(szPath, "b");

    string path = token;


    //path-> home/ty/llvm-project-16/mlir/
    //==================  修改成自己的路径  ================================
    string OpsTowardPass = "test/OpsTowardPass.json";
    string temp = "test/temp.json";
    //===================================================================

    string OpsTowardPass_json = path + OpsTowardPass;
    string temp_json = path + temp;

    //获取所有方言
    auto funcOp = getOperation();
    for (auto &bb : funcOp) {
      string dialct;
      string opname;
      getOperation().walk([&](Operation *op) {
        op->dump();
        if (op->getDialect()!=nullptr){
          dialct = op->getDialect()->getNamespace().str();
          opname = op->getName().getStringRef().data();
          auto isExist = std::find(dialects.begin(), dialects.end(), dialct);
          auto isExist1 = std::find(operations.begin(), operations.end(), opname);
          
          if(dialects.empty() || isExist==dialects.end()){ // dialects列表中没有这个方言
                                                             //          if(dialct!="func" && dialct!="llvm" && dialct!="builtin")  //
            if(dialct!="builtin")
             dialects.push_back(dialct);  //不保存 func 、 builtin
        }
        if(operations.empty() || isExist1==operations.end()){ // dialects列表中没有这个方言
             operations.push_back(opname); 
        }
        }
      });
    }


    //    cout<<dialects.size()<<endl;
    //对测试用例进行分析，分析所有的dialect，以及候选pass



    for (auto selected : dialects){
      //根据方言找到ops,，不分析"func"、"llvm"、"builtin"
      if(selected=="func" || selected=="llvm" || selected=="builtin" || selected=="bufferization")
        continue ;
      for (auto &bb : funcOp) {
        string opName;
        getOperation().walk([&](Operation *op) {
          if (op->getDialect()!=nullptr && op->getDialect()->getNamespace().str()==selected){
            opName = op->getName().getStringRef().str();
            if (opName.find("_")!=-1) //去掉下划线
              opName.erase(opName.find("_"),1);
            auto isExist = std::find(lower_ops.begin(), lower_ops.end(), opName);
            if(isExist==lower_ops.end())
              lower_ops.push_back(opName);
          }
        });
      }



      string Op;
      std::ifstream is;
      is.open (OpsTowardPass_json, std::ios::binary);
      Json::Reader reader;
      Json::Value value;
      Json::Value OneDialect;

      if (reader.parse(is, value)) // json字符串转为json对象
        OneDialect = value[selected];

      //5.根据ops匹配相对应的Pass
      if (selected == "linalg"){ //linalg一共有7个降级pass，这四个pass适合全部的linalg op
        //SelectedPass.push_back("-func-bufferize -linalg-bufferize -convert-linalg-to-affine-loops -convert-linalg-to-std -convert-linalg-to-loops -convert-linalg-to-parallel-loops -convert-linalg-to-llvm -convert-linalg-to-spirv");
        lower_pass.push_back("-convert-linalg-to-affine-loops -convert-linalg-to-loops -convert-linalg-to-parallel-loops");
      }
      if (selected == "llvm"){   //llvm一共有三个优化pass，这两个优化pass对应全部的llvm op
        lower_pass.push_back("-llvm-legalize-for-export -llvm-request-c-wrappers");
      }

      llvm::SmallVector<string> combine_ops;
      llvm::SmallVector<string> json_ops;
      string single = "";
      for (int i = 0; i < OneDialect["OPS_NUM"].asInt(); i++) {
        combine_ops.clear();
        json_ops.clear();
        auto ops = OneDialect["OPS"][i];
        for (auto op : ops["OPS_NAME"]) {
          //        cout<<op.asString()<<endl;
          json_ops.push_back(op.asString());
        }

        combine_ops.append(json_ops);
        combine_ops.append(lower_ops);

        //排序，去重，如果size减少表明有重复，将pass添加进lower_pass
        int size_before, size_after;
        size_before = combine_ops.size();
        sort(combine_ops);
        combine_ops.erase(unique(combine_ops.begin(), combine_ops.end()), combine_ops.end());
        size_after = combine_ops.size();

        if(size_before != size_after){
          single = OneDialect["PASS"][i].asString();
          if(single!="")
            lower_pass.push_back(single);
        }

//        if(selected=="arith"){
//          auto isExist = std::find(lower_pass.begin(), lower_pass.end(), "-convert-arith-to-spirv");
//          if(isExist!=lower_pass.end())
//            lower_pass.erase(isExist);
//        }
      }
    }

    //6.有重复的PASS，做一个过滤，
    sort(lower_pass.begin(),lower_pass.end());
    lower_pass.erase(unique(lower_pass.begin(), lower_pass.end()), lower_pass.end());

    //7.将方言和pass存入json文件
    ofstream os;
    os.open(temp_json);
    if (!os.is_open())
      cout << "error：can not find or create the file which named \" demo.json\"." << endl;
    Json::StyledWriter sw;  //缩进输出

    Json::Value root;
    root["LowerPassNum"] = std::to_string(lower_pass.size());
    root["DialectNum"] = std::to_string(dialects.size());
    for (int i = 0; i < lower_pass.size(); ++i) {
      root["LowerPass"].append(lower_pass[i]);
    }
    for (int i = 0; i < dialects.size(); ++i) {
      root["dialect"].append(dialects[i]);
    }

    for (int i = 0; i < operations.size(); ++i) {
      root["operation"].append(operations[i]);
    }
    os << sw.write(root);
    os.close();
  };
  StringRef getArgument() const final { return "GetDialectName"; }
  StringRef getDescription() const final { return "Get all Dialect Names in one mlir file"; }

};
}  // namespace
namespace mlir {
void registerGetDialectName() { PassRegistration<GetDialectName>(); }
} // namespace mlir