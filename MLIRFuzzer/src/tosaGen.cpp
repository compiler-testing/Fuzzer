//
// Created by nisl812 on 9/12/22.
//

#include "TosaGen/utils.h"
#include "TosaGen/create.h"
#include "TosaGen/opinfo.h"
#include "TosaGen/transfer.h"

using namespace mlir;
using namespace std;

Utils genUtils;
InfoGen infogen;
Create create;
opInfo info;
Transfer transfer;

namespace {
struct tosaGenpass
    : public PassWrapper<tosaGenpass, OperationPass<func::FuncOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(tosaGenpass)
  StringRef getArgument() const final { return "tosaGen"; }
  StringRef getDescription() const final { return "tosaGen."; }
  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<tosa::TosaDialect>();
  }
  void runOnOperation() override {
    auto funcOp = getOperation();

    Block *firstbb = &(*funcOp.begin());
    Location loc = firstbb->begin()->getLoc();
    ImplicitLocOpBuilder b = ImplicitLocOpBuilder::atBlockBegin(loc, firstbb);
    b.setInsertionPointToStart(firstbb);

    //TODO: more operators
    // "select","transpose_conv2d""equal","greater"...

    llvm::ArrayRef<StringRef> opPool = {"concat","abs","bitwise_not", "ceil","clz","exp","floor","log","logical_not",
                                         "reciprocal","rsqrt","sigmoid","tanh","argmax","reduce_all","reduce_any","reduce_max","reduce_min","reduce_prod",
                                         "reduce_sum","reverse","add","bitwise_and","bitwise_or","bitwise_xor","div","greater_equal",
                                        "logical_and","logical_left_shift","logical_or","logical_right_shift","logical_xor","maximum","minimum","pow","sub","conv2d",
                                         "conv3d","depthwise_conv2d","avg_pool2d","max_pool2d","reshape","cast"};

    
    //  llvm::ArrayRef<StringRef> opPool = {"concat","abs","bitwise_not","pow","sub","conv2d", "conv3d"};                               

        //    // 选择要创建的op个数
    int opNum = genUtils.genRandomN(1, 20);
    opNum = 10;
    cout << opNum << endl;

    // 创建第一个op
    Value newOp;
    Value preOp;
    int flag_shape= 0;
    int flag_type= 0;

    Type pre_result;
    string selectedOp;
    SmallVector<Value> valuePool;
    DenseMap<int, Value> tails;
    for (int i = 0; i < opNum; i++) {
      selectedOp = genUtils.getRandomOp(opPool);
      cout << "===" << i << "  testing opName:" << selectedOp << endl;
      if (i == 0) {
        
        newOp = create.createNewBranch(b, loc, funcOp, selectedOp);
        valuePool.push_back(newOp);
        tails.insert(std::make_pair(0, newOp));
      }else {
        infogen.initInfo(selectedOp);

        // find a compatible insertion
        Value t;
        Value insertion;
        SmallVector<int,8> index;
        int chainIndex = -1;

        SmallVector<Value> insertPoints = genUtils.typeMatch(valuePool);
        if(!insertPoints.empty()){//存在兼容节点
          cout<<"insertPoints"<<insertPoints.size()<<endl;
          //寻找兼容的尾部节点
          for(auto pair : tails){
            t = pair.second;
            for(auto vi : insertPoints){
              if(t==vi){
                  index.push_back(pair.first);
                  break;
              }
            }
          }
          if(!index.empty()){ //寻找兼容的尾部节点
            cout<<"tail insertion"<<endl;
            chainIndex = genUtils.genRandomN(0,index.size()-1);
            insertion = tails[index[chainIndex]];
          }else{//寻找兼容的插入点
            cout<<"Insert garbage node"<<endl;
            insertion = insertPoints[genUtils.genRandomN(0,insertPoints.size()-1)];
          }

        insertion.dump();
        infogen.addInputType(b, insertion);
        Value v = genUtils.skipMatch(valuePool);
        if(v==nullptr){
          infogen.addInputs(b, loc, funcOp, insertion);
        }else{
          info.inputs.push_back(insertion);
          info.inputs.push_back(v);
          info.inputType={};
          info.inputType.push_back(insertion.getType());
          info.inputType.push_back(v.getType());
        }
        infogen.addAttrs(b, loc);
        infogen.addResult(b);
        newOp = create.createOp(b, loc);
        valuePool.push_back(newOp);
        if(chainIndex!=-1){
            tails[chainIndex] = newOp;
        }

        }else{//插入新的分支
          cout<<"create new branch"<<endl;
          newOp = create.createNewBranch(b, loc, funcOp, selectedOp);
          valuePool.push_back(newOp);
          tails.insert(std::make_pair(tails.size(), newOp));
        }
      }
    }


      func::ReturnOp returnOp;
      if (!firstbb->empty())
        returnOp = dyn_cast<func::ReturnOp>(firstbb->back());
      
      Value output;
  
      cout<<"=================tail============"<<endl;;
      for(auto pair:tails){

          cout<<pair.first<<endl;
          pair.second.dump();
      }
      for(int i=0;i<tails.size();i++){
          returnOp->insertOperands(i,tails[i]); 
          funcOp.insertResult(i,tails[i].getType(),DictionaryAttr::get(&getContext())); 
      }
      // returnOp->insertOperands(0,preOp);  //插入return的操作数
      // funcOp.insertResult(0,returnOp.getOperand(0).getType(),DictionaryAttr::get(&getContext()));  //插入func的返回值
  }
};

}

namespace mlir {
void registertosaGen() { PassRegistration<tosaGenpass>();
}}

