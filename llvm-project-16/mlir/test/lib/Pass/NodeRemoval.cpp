////
//// Created by Administrator on 2023/2/21.
////
////
//// Created by Administrator on 2023/2/21.
////
//#include "TestDialect.h"
//#include "mlir/TosaGen/create.h"
//#include "mlir/TosaGen/opinfo.h"
//#include "mlir/TosaGen/transfer.h"
//#include "mlir/TosaGen/utils.h"
//
//using namespace mlir;
//using namespace std;
//using namespace test;
//
//
//Utils genUtils;
//InfoGen infogen;
//Create create;
//opInfo info;
//Transfer transfer;
//
//
//namespace {
//struct NRPass
//    : public PassWrapper<NRPass, OperationPass<func::FuncOp>> {
//  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(NRPass)
//  StringRef getArgument() const final { return "NR"; }
//  StringRef getDescription() const final { return "Node Removal."; }
//  void getDependentDialects(DialectRegistry &registry) const override {
//    registry.insert<tosa::TosaDialect>();
//  }
//  void runOnOperation() override {
////    cout << "NR" << endl;
//    auto funcOp = getOperation();
//
//    SmallVector<Operation *> rawops;  //所有op
//    SmallVector<Operation *> saveops;  //type相同的op
//    SmallVector<Value> operands;  //保存与输出相同的操作数
//
//    funcOp.walk([&](Operation *op) { rawops.push_back(op); });
//
//    //寻找输入输入type相同的op
//    //随即删除，并更新使用者
//    for (auto op : rawops){
//      if(op->getNumOperands()>0 && op->hasOneUse()){  //操作数大于0，并且有后继节点
//        auto result_type = op->getResult(0).getType();
//        for (auto operand : op->getOperands()){
//          auto operand_type = operand.getType();
//          if(result_type == operand_type)
//          {
//            operands.push_back(operand);
////            cout<<"the input and output type are the same!"<<endl;
//            saveops.push_back(op);
//            break;
//          }
//        }
//      }
//    }
//    int i = genUtils.genRandomN(0, saveops.size()-1);
//    Operation* removeOp = saveops[i];
////    cout<<"remove this op:  ";
////    removeOp->dump();
//    Value subOp = operands[i];
//    removeOp->getResult(0).replaceAllUsesWith(subOp);
//    removeOp->erase();
//  }
//};
//}
//namespace mlir {
//namespace test {
//void registerNRPass() { PassRegistration<NRPass>();
//}
//}}
