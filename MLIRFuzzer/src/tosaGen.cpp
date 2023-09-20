//
// Created by nisl812 on 9/12/22.
//

#include "TosaGen/utils.h"
#include "TosaGen/create.h"
#include "TosaGen/opinfo.h"
#include "TosaGen/transfer.h"

using namespace mlir;
using namespace std;

//Instantiate objects of these classes
Utils genUtils;     //commonly used utility functions
InfoGen infogen;    //processing related to the opInfo struct
Create create;      //implementation of creating op
Transfer transfer;  //infer op's result according to the input and attribute 

//Instantiate the struct(opInfo) 
opInfo info;


namespace {
struct tosaGenpass
    : public PassWrapper<tosaGenpass, OperationPass<func::FuncOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(tosaGenpass)
  StringRef getArgument() const final { return "tosaGen"; }
  StringRef getDescription() const final { return "generate tosa graph with multi-branch structure."; }
  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<tosa::TosaDialect>();
  }
  void runOnOperation() override {
    auto funcOp = getOperation();
    Block *firstbb = &(*funcOp.begin());
    Location loc = firstbb->begin()->getLoc();
    ImplicitLocOpBuilder b = ImplicitLocOpBuilder::atBlockBegin(loc, firstbb);

    //Set the insertion point of first op
    b.setInsertionPointToStart(firstbb);

    //TODO: more operators
    // "select","transpose_conv2d""equal","greater"...

    llvm::ArrayRef<StringRef> opPool = {"concat","abs","bitwise_not", "ceil","clz","exp","floor","log","logical_not",
                                         "reciprocal","rsqrt","sigmoid","tanh","argmax","reduce_all","reduce_any","reduce_max","reduce_min","reduce_prod",
                                         "reduce_sum","reverse","add","bitwise_and","bitwise_or","bitwise_xor","div","greater_equal",
                                        "logical_and","logical_left_shift","logical_or","logical_right_shift","logical_xor","maximum","minimum","pow","sub","conv2d",
                                         "conv3d","depthwise_conv2d","avg_pool2d","max_pool2d","reshape","cast"};

    
                         

    // set the number of operators 
    int opNum = genUtils.genRandomN(1, 20);
    opNum = 10;
    cout << opNum << endl;

    Value newOp;
    Value preOp;
    int flag_shape= 0;
    int flag_type= 0;

    Type pre_result;
    string selectedOp;
    SmallVector<Value> valuePool;  // Save all the created ops.
    DenseMap<int, Value> tails;    // Save the tail nodes of all branches in the graph.

    // Incrementally insert nodes into the graph until the model size (opNum) is satisfied.
    for (int i = 0; i < opNum; i++) {
      selectedOp = genUtils.getRandomOp(opPool);       // select operator randomly
      cout << "===" << i << "  testing opName:" << selectedOp << endl;
      // create the first op, which is a new branch.
      if (i == 0) {
        newOp = create.createNewBranch(b, loc, funcOp, selectedOp);
        valuePool.push_back(newOp);        // update valuePool
        tails.insert(std::make_pair(0, newOp));    // update tails
      }else {
        //Initialize the struct opinfo.
        infogen.initInfo(selectedOp);

        Value t;
        Value insertion;
        SmallVector<int,8> index;
        int chainIndex = -1;

        //Find a set of feasible insertion points S.
        SmallVector<Value> insertPoints = genUtils.typeMatch(valuePool);

        //The insertion point is available
        if(!insertPoints.empty()){
          cout<<"insertPoints"<<insertPoints.size()<<endl;
          // Check whether the insertion point includes the tail node
          // If it does, perform tail insertion.  
          // otherwise, use random insertion.
          for(auto pair : tails){
            t = pair.second;
            for(auto vi : insertPoints){
              if(t==vi){
                  index.push_back(pair.first);
                  break;
              }
            }
          }

          // 1. tail insertion
          if(!index.empty()){
            cout<<"tail insertion"<<endl;
            chainIndex = genUtils.genRandomN(0,index.size()-1);
            insertion = tails[index[chainIndex]];
          }else{   // 2.insertion randomly
            cout<<"insert randomly"<<endl;
            insertion = insertPoints[genUtils.genRandomN(0,insertPoints.size()-1)];
          }
          insertion.dump();

          //Set the input type that satisfies the op's constraints.
          infogen.addInputType(b, insertion);

          //Find a compatible nodes to establish skip connections
          Value v = genUtils.skipMatch(valuePool);

          //If none is found, randomly generate inputs that satisfy op's constraints."
          if(v==nullptr){
            infogen.addInputs(b, loc, funcOp, insertion);
          }else{  //add skip connections
            info.inputs.push_back(insertion);
            info.inputs.push_back(v);
            info.inputType={};
            info.inputType.push_back(insertion.getType());
            info.inputType.push_back(v.getType());
          }

          //Set the value of attribute.
          infogen.addAttrs(b, loc);

          //Infer the shape and type of op's result
          infogen.addResult(b);

          //create op
          newOp = create.createOp(b, loc);
          valuePool.push_back(newOp);      // update valuePool
          if(chainIndex!=-1){
              tails[chainIndex] = newOp;   // update tails
          }

        }else{  // 3.create a new branch 
          cout<<"create new branch"<<endl;
          newOp = create.createNewBranch(b, loc, funcOp, selectedOp);
          valuePool.push_back(newOp);                         // update valuePool
          tails.insert(std::make_pair(tails.size(), newOp));  // update tails
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

