//
// Created by nisl812 on 9/12/22.
//

#include "TestDialect.h"
#include "mlir/TosaGen/utils.h"
#include "mlir/TosaGen/create.h"
#include "mlir/TosaGen/opinfo.h"
#include "mlir/TosaGen/transfer.h"

using namespace mlir;
using namespace std;
//using namespace test;

extern Utils genUtils;
extern InfoGen infogen;
extern Create create;
extern opInfo info;
extern Transfer transfer;

namespace {
struct tosaGenChainpass
    : public PassWrapper<tosaGenChainpass, OperationPass<func::FuncOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(tosaGenChainpass)
  StringRef getArgument() const final { return "tosaGenC"; }
  StringRef getDescription() const final { return "tosaGenC"; }
  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<tosa::TosaDialect>();
  }
  void runOnOperation() override {
    auto funcOp = getOperation();

    Block *firstbb = &(*funcOp.begin());
    Location loc = firstbb->begin()->getLoc();
    ImplicitLocOpBuilder b = ImplicitLocOpBuilder::atBlockBegin(loc, firstbb);
    b.setInsertionPointToStart(firstbb);

    SmallVector<Value> valuePool;


    //    ,"select"
    //   ,"transpose_conv2d"
    //    "equal",
    // "maximum",
    //"greater",

    llvm::ArrayRef<StringRef> opPool= {"concat","abs","bitwise_not", "ceil","clz","exp","floor","log","logical_not",
                                        "reciprocal","rsqrt","sigmoid","tanh","argmax","reduce_all","reduce_any","reduce_max","reduce_min","reduce_prod",
                                        "reduce_sum","reverse","add","bitwise_and","bitwise_or","bitwise_xor","div","greater_equal",
                                        "logical_and","logical_left_shift","logical_or","logical_right_shift","logical_xor","minimum","pow","sub","conv2d",
                                        "conv3d","depthwise_conv2d","avg_pool2d","max_pool2d"};


    //    infogen.analyseAllOp(opPool);
    //
    //    // 选择要创建的op个数
    int opNum = genUtils.genRandomN(3, 10);
    // opNum =10;
    cout << opNum << endl;

    int max_same = genUtils.genRandomN(2, opNum/2);
//  int max_same = 3;
    // 创建第一个op
    Value newOp;
    Value preOp;
    int flag_shape= 0;
    int flag_type= 0;

    Type pre_result;
    for (int i = 0; i < opNum; i++) {
      string selectedOp = genUtils.getRandomOp(opPool);
      auto opConstraint = genUtils.getConstraint(selectedOp);

      if (i == 0) {
        cout << "===" << i << "  testing opName:" << selectedOp << endl;
        infogen.initInfo(selectedOp);
        infogen.addInputType(b);
        if(info.inputType.size()==0)
          continue;
        else
          create.insertFuncArg(b,funcOp,info.inputType); //插入index
        infogen.addInputs(b,loc,funcOp);
        infogen.addAttrs(b,loc);
        infogen.addResult(b);
        newOp = create.createOp(b,loc);
        valuePool.push_back(newOp);
        preOp = newOp;
      }else {
        llvm::ArrayRef<StringRef> change = {"reshape", "cast"};
        if (flag_shape == max_same && flag_type == max_same) {
          unsigned int r = genUtils.genRandomN(0, 1);
          selectedOp = change[r].str();
        } else if (flag_shape >= max_same) {
          selectedOp = change[0];
        } else if (flag_type >= max_same) {
          selectedOp = change[1];
        }
        flag_shape = 0;
        flag_type = 0;
        // find a compatible node
        do {
          selectedOp = genUtils.getRandomOp(opPool);
          cout << "===" << i << "  testing opName:" << selectedOp << endl;
          infogen.initInfo(selectedOp);
        } while (!genUtils.typeMatch(preOp));

        infogen.addInputType(b, preOp);
        Value v = genUtils.skipMatch(valuePool);
        if(v==nullptr){
          infogen.addInputs(b, loc, funcOp, preOp);
        }else{
          info.inputs.push_back(preOp);
          info.inputs.push_back(v);
        }

        infogen.addAttrs(b, loc);
        infogen.addResult(b);
        newOp = create.createOp(b, loc);
        valuePool.push_back(newOp);
        preOp = newOp;

      }
      auto preTensor = preOp.getType().cast<ShapedType>();
      auto curTensor = info.resultType.cast<ShapedType>();
      if(preTensor.getElementType()== curTensor.getElementType()){
        flag_type++;
      }
      if(preTensor.getShape()== curTensor.getShape()){
        flag_shape++;
      }
    }

      func::ReturnOp returnOp;

      if (!firstbb->empty())
        returnOp = dyn_cast<func::ReturnOp>(firstbb->back());
      returnOp->insertOperands(0,preOp);  //插入return的操作数
      funcOp.insertResult(0,returnOp.getOperand(0).getType(),DictionaryAttr::get(&getContext()));  //插入func的返回值
  }
};

}
namespace mlir {
namespace test {
void registertosaGenChain() { PassRegistration<tosaGenChainpass>();
}
}}

//// unary, have 0 attr
//llvm::ArrayRef<StringRef> opNamex = {
//    "abs",   "bitwise_not", "ceil", "clz",         "exp",
//    "floor", "identity",    "log",  "logical_not", "reciprocal",
//    "rsqrt", "sigmoid",     "tanh"}; // while_loop" , "identity",
//// unary, have 1 attr
//llvm::ArrayRef<StringRef> opNamen =
//    {"argmax", "reduce_all", "reduce_any",  "reduce_max",
//     "reduce_min", "reduce_prod", "reduce_sum",
//     "reshape",    "reverse", "concat"}; //"custom", , "negate",
//// unary, have 2 attrs
//llvm::ArrayRef<StringRef> opName2 = {"reluN", "slice"};
//// unary, have 3 attrs
//llvm::ArrayRef<StringRef> opName3 = {"max_pool2d"};
//// unary, have 4 attrs
//llvm::ArrayRef<StringRef> opName4 = {"avg_pool2d", "clamp"};
//// unary, have 7 attrs
//// unary, have 7 attrs
//llvm::ArrayRef<StringRef> opName5 = {"rescale", "resize"};
//
//// binary, have 0 attr
//llvm::ArrayRef<StringRef> opNameX = {"add",         "bitwise_and",
//                                     "bitwise_or",  "bitwise_xor",
//                                     "div",         "equal",
//                                     "greater",     "greater_equal",
//                                     "logical_and", "logical_left_shift",
//                                     "logical_or",  "logical_right_shift",
//                                     "logical_xor", "maximum",
//                                     "minimum",     "pow",
//                                     "sub"}; //,"cond_if" ,,"gather" ,"transpose" ,     "table"
//// binary, have 1 attr
//llvm::ArrayRef<StringRef> opName7 = {"arithmetic_right_shift", "mul",
//                                     "tile" , "concat"}; //,"matmul"
//
//// trinary, have 0 attr
//llvm::ArrayRef<StringRef> opName8 = {"select"}; //"scatter"
//                                                // trinary, have 1 attr
//                                                // llvm::ArrayRef<StringRef> opName9 = {"fully_connected"}; //,"pad"
//                                                // trinary, have 4 attr
//                                                // llvm::ArrayRef<StringRef> opName10 = {"conv2d","conv3d","depthwise_conv2d","transpose_conv2d"}; trinary, have 5 attr llvm::ArrayRef<StringRef> opName11 = {"transpose_conv2d"};



//    for(int i=0 ; i < opNum ; i++){
//      if(i==0){ //创建第一个op：随机选择opcode
//        string firstOp = infogen.getRandomOp(opPool1);
//        ops.push_back(firstOp);
//        cout <<"==="<<i <<"  testing opName:" << firstOp << endl;
//
//        //实例化info结构体，用于创建op
//        auto opConstraint = genUtils.getConstraint(firstOp);
//        infogen.setFirstOpInfo(b,loc,opConstraint);
//      }
//      else{ //创建第i个op：根据上一条op的tensor形状和类型创建
//        funcOp.dump();
//        string selectedOp = infogen.getNextOp(lastOp);
//        if(selectedOp == " "){
//          cout<<"==============break=================="<<endl;
//          break;
//        }
//        //如果连续三个opshape或者type不变，更改shape或者type，避免同质化
//        if(flag>=max_same){
//          llvm::ArrayRef<StringRef> change = { "reshape","cast"};
//          unsigned int r = genUtils.genRandomN(0,1);
//          selectedOp = change[r].str();
//          flag=0;
//        }
//        cout <<"==="<<i <<"  testing opName:" << selectedOp << endl;
//
//        //实例化info结构体，用于创建op
//        auto opConstraint = genUtils.getConstraint(selectedOp);
//        infogen.setNextOpInfo(b,loc,opConstraint,newOp);
//      }
//      constraint_info = {info.resultType,info.resultShape};
//
//      newOp = create.createOp(b,loc);
//      lastOp = newOp;
//      //用于检测当前op与上一个op的shape或者type是否相同
//      if(!pre_resultShape.empty()){
//        if (pre_resultShape==info.resultShape || pre_resultType == info.resultType)
//          flag++;
//      }
//      pre_resultShape = info.resultShape;
//      pre_resultType = info.resultType;
//    }