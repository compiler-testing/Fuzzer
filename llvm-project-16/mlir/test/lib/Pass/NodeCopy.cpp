////
//// Created by Administrator on 2023/2/21.
////
////
//// Created by Administrator on 2023/2/21.
////
//#include "TestDialect.h"
////#include "mlir/TosaGen/utils.h"
////#include "mlir/TosaGen/create.h"
////#include "mlir/TosaGen/opinfo.h"
////#include "mlir/TosaGen/transfer.h"
//
//using namespace mlir;
//using namespace std;
//using namespace test;
//
////Utils genUtils;
////InfoGen infogen;
////Create create;
////opInfo info;
////Transfer transfer;
//
//namespace {
//struct NCPass
//    : public PassWrapper<NCPass, OperationPass<func::FuncOp>> {
//  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(NCPass)
//  StringRef getArgument() const final { return "NC"; }
//  StringRef getDescription() const final { return "Node Copy."; }
//  void getDependentDialects(DialectRegistry &registry) const override {
//    registry.insert<tosa::TosaDialect>();
//  }
//  void runOnOperation() override {
//    cout << "NC" << endl;
//    auto funcOp = getOperation();
//
//    Block *firstbb = &(*funcOp.begin());
//    Location loc = firstbb->begin()->getLoc();
//    ImplicitLocOpBuilder b = ImplicitLocOpBuilder::atBlockBegin(loc, firstbb);
//    b.setInsertionPointToEnd(firstbb);
//
//    //在return前面设置插入点
//    SmallVector<Operation *> rawops;  //所有op
//    Operation *lastOp = nullptr;
//    Operation *curOp = nullptr;
//    funcOp.walk([&](Operation *op) {
//      if (op->getName().getStringRef() != "func.func"){
//      if (op->getName().getStringRef() == "func.return"){
//        lastOp = curOp;
//      }
//       curOp = op;
////       curOp->dump();
//      }
//      rawops.push_back(op);
//    });
//
////    lastOp->dump();
//
//    b.setInsertionPointAfter(lastOp);
////   b.setInsertionPoint(lastOp);
//    SmallVector<Operation *> saveops;  //type相同的op
//    SmallVector<Operation *> candidateops;  //与copy type相同的op
//    SmallVector<int> operandN;  //保存与输出相同的操作数下标
//
//    //寻找输入输出type相同的op
//    int m = 0;//操作数下标
//    for (auto op : rawops){
//      m = 0;
//      if(op->getNumOperands()>0 && op->hasOneUse()){  //操作数大于0，并且有后继节点
//        auto result_type = op->getResult(0).getType();
//        for (auto operand : op->getOperands()){
//          auto operand_type = operand.getType();
//          if(result_type == operand_type)
//          {
//            m++;
//            operandN.push_back(m);
////            cout<<"the input and output type are the same!"<<endl;
//            saveops.push_back(op);
//            break;
//          }
//        }
//      }
//    }
//    //随机选择copy op
//    int i = genUtils.genRandomN(0, saveops.size()-1);
//    Operation* selectedOp = saveops[i];
//    Operation * copyOp = nullptr;
//    copyOp = b.clone(*selectedOp);
//    Operation * copyOp1 = b.cloneWithoutRegions(*selectedOp);
//    cout<<"copy:  ";
//    copyOp->dump();
//
//
//    //寻找合适的插入点，也就是另一个与拷贝op type相同的op
//    for(auto op : saveops){
//      if (op->getResult(0).getType()==copyOp->getResult(0).getType()){
//        candidateops.push_back(op);
//        cout<<"candidate:  ";
//        op->dump();
//      }
//    }
//
//    //随机选择插入点
//    int j = genUtils.genRandomN(0, candidateops.size()-1);
//    Operation* selectedOp1 = candidateops[j];
//    cout<<"insert:  ";
//    selectedOp1->dump();
//
//
//    //更新使用者
//    copyOp->setOperand(operandN[i],selectedOp1->getResult(0));
//    selectedOp1->getResult(0).replaceAllUsesWith(copyOp->getResult(0));
//
//
//  }
//};
//}
//namespace mlir {
//namespace test {
//void registerNCPass() { PassRegistration<NCPass>();
//}
//}}
