//
//#include "mlir/Dialect/Arith/IR/Arith.h"
//#include "mlir/Dialect/Vector/IR/VectorOps.h"
//#include "mlir/IR/Builders.h"
//#include "mlir/IR/PatternMatch.h"
//#include "mlir/Pass/Pass.h"
//#include "mlir/Pass/PassRegistry.h"
//#include "llvm/Support/Debug.h"
//#include "llvm/Support/raw_ostream.h"
//#include <mlir/IR/ImplicitLocOpBuilder.h>
//#include "TestDialect.h"
//#include <iostream>
//
//#define DEBUG_TYPE "user-define-pass"
//using namespace mlir;
//using namespace std;
//using namespace llvm;
////=====================================================================
//
//
//namespace {
//struct SubPass
//    : public PassWrapper<SubPass, OperationPass<func::FuncOp>> {
//  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(SubPass)
//  void runOnOperation() override;
//
//  void getDependentDialects(DialectRegistry &registry) const override {
//    registry.insert<arith::ArithDialect>();
//  }
//  StringRef getArgument() const final { return "ES"; }
//  StringRef getDescription() const final { return "Equivalent substitution pass"; }
//};
//} // namespace
//
////======================================================================================
//
//int getRandomSeed(Type elementTy,int scope1,int scope2) {
//
//  if (elementTy.isa<TensorType>()) {
//    scope2 = 3;
//  }
//  int subNum = (rand() % (scope2 - scope1)) + scope1;
//  return subNum;
//}
//
//Attribute getAttributeConst(Type elementTy, double a) {
//  if (elementTy.isIntOrFloat()) {
//    if (elementTy.isF16() || elementTy.isF32() || elementTy.isF64() ||
//        elementTy.isF128())
//      return FloatAttr::get(elementTy, a);
//    else
//      return IntegerAttr::get(elementTy, a);
//  }
//  return 0;
//}
//
//
//template <typename BinaryArithOp, typename BinaryReplacementOp>
//void instructionReplacement(const SmallVector<BinaryArithOp> &vector);
//
//template <typename BinaryArithOp, typename BinaryReplacementOp>
//void subInstructionReplacement(const SmallVector<BinaryArithOp> &vector);
//
//void LogicalOperationExcute(SmallVector<arith::AndIOp> &andIOps,SmallVector<arith::OrIOp> &orIOps,SmallVector<arith::XOrIOp> &xorIOps);
//
//  void SubPass::runOnOperation() {
//  LLVM_DEBUG(llvm::dbgs() << "================pass excute start===========\n");
//  func::FuncOp f = OperationPass<func::FuncOp>::getOperation();
//
//  SmallVector<arith::AddIOp> addIOps;
//  SmallVector<arith::AddFOp> addFOps;
//
//  SmallVector<arith::SubIOp> subIOps;
//  SmallVector<arith::SubFOp> subFOps;
//
//  SmallVector<arith::AndIOp> andIOps;
//  SmallVector<arith::OrIOp> orIOps;
//  SmallVector<arith::XOrIOp> xorIOps;
//
//  f.walk([&](mlir::arith::AddIOp addIOp) { addIOps.push_back(addIOp); });
//  f.walk([&](mlir::arith::AddFOp addFOp) { addFOps.push_back(addFOp); });
//  f.walk([&](mlir::arith::SubIOp subIOp) { subIOps.push_back(subIOp); });
//  f.walk([&](mlir::arith::SubFOp subFOp) { subFOps.push_back(subFOp); });
//  f.walk([&](mlir::arith::AndIOp andIOp) { andIOps.push_back(andIOp); });
//  f.walk([&](mlir::arith::OrIOp orIOp) { orIOps.push_back(orIOp); });
//  f.walk([&](mlir::arith::XOrIOp xorIOp) { xorIOps.push_back(xorIOp); });
//
//  instructionReplacement<arith::AddIOp, arith::SubIOp>(addIOps);
//  instructionReplacement<arith::AddFOp, arith::SubFOp>(addFOps);
//
//  subInstructionReplacement<arith::SubIOp, arith::AddIOp>(subIOps);
//  subInstructionReplacement<arith::SubFOp, arith::AddFOp>(subFOps);
//
//  LogicalOperationExcute(andIOps,orIOps,xorIOps);
//
//
//
//  LLVM_DEBUG(llvm::dbgs() << "================pass excute end===========\n");
//}
//
//template <typename BinaryArithOp, typename BinaryReplacementOp>
//void instructionReplacement(const SmallVector<BinaryArithOp> &vector) {
//  for (auto op : vector) {
//    LLVM_DEBUG(llvm::dbgs() << "================add replace op start: ===========\n");
//    OpBuilder builder(op.getContext());
//    // Sets the insertion position of the instruction generation class at the
//    // instruction to be replaced
//    builder.setInsertionPoint(op);
//    // Gets the right-hand/left-hand operand of the instruction, eg. a=b+c
//    Value rsh = op.getRhs(); // c
//    Value lsh = op.getLhs(); // b
//    // Gets type of operand
//    auto elementTy = rsh.getType();
//    // new mlirOP
//    Value newResult;
//
//    int subNum = getRandomSeed(elementTy,1,5);
//    LLVM_DEBUG(llvm::dbgs() << "            " << subNum << "\n");
//    // a =  b - (-c)
//
//    if (subNum == 1) {
//      Value zeroExpr = builder.create<arith::ConstantOp>(
//          op->getLoc(), getAttributeConst(elementTy, 0));
//      // Create an arith sub op to get 0-c
//      Value negRsh =
//          builder.create<BinaryReplacementOp>(op->getLoc(), zeroExpr, rsh);
//      // Create an arith sub op to get b-(-c)
//      newResult =
//          builder.create<BinaryReplacementOp>(op->getLoc(), lsh, negRsh);
//    } else if (subNum == 2) {
//      // a = -(-b + (-c)) output-test11
//      Value zeroExpr = builder.create<arith::ConstantOp>(
//          op->getLoc(), getAttributeConst(elementTy, 0));
//      Value negRsh = builder.create<BinaryReplacementOp>(op->getLoc(), zeroExpr,
//                                                         rsh); //  -c
//      Value temp = builder.create<BinaryReplacementOp>(op->getLoc(), zeroExpr,
//                                                       lsh); // -b
//      Value temp1 = builder.create<BinaryArithOp>(op->getLoc(), negRsh,
//                                                  temp); //  (-b)+(-c)
//      newResult = builder.create<BinaryReplacementOp>(op->getLoc(), zeroExpr,
//                                                      temp1); //  -(-b+(-c))
//    } else if (subNum == 3) {
//      // r = rand();a = b + r;a = a + c;a = a - r; output-test12
//      int t = rand();
//      Value r = builder.create<arith::ConstantOp>(
//          op->getLoc(), getAttributeConst(elementTy, t));
//      Value temp1 =
//          builder.create<BinaryArithOp>(op->getLoc(), r, lsh); //  a = b + r
//      Value temp2 =
//          builder.create<BinaryArithOp>(op->getLoc(), temp1, rsh); //  a = a + c
//      newResult = builder.create<BinaryReplacementOp>(op->getLoc(), temp2,
//                                                      r); // a = a - r
//    } else if (subNum == 4) {
//      // r = rand();a = b - r;a = a + c;a = a + r; output-test13
//      // a + b  (a-r)+b+r
//      int t = rand() % 1000;
//      Value r = builder.create<arith::ConstantOp>(
//          op->getLoc(), getAttributeConst(elementTy, t));
//      Value temp1 = builder.create<BinaryReplacementOp>(op->getLoc(), lsh,
//                                                        r); //  a = b - r
//      Value temp2 =
//          builder.create<BinaryArithOp>(op->getLoc(), temp1, rsh); //  a = a + c
//      newResult =
//          builder.create<BinaryArithOp>(op->getLoc(), temp2, r); // a = a + r
//    }
//
//    Value result = op.getResult();
//    result.replaceAllUsesWith(newResult);
//    op.erase();
//    LLVM_DEBUG(llvm::dbgs() << "================add replace op end: ===========\n");
//  }
//}
//
//template <typename BinaryArithOp, typename BinaryReplacementOp>
//void subInstructionReplacement(const SmallVector<BinaryArithOp> &vector) {
//
//  for (auto op : vector) {
//    LLVM_DEBUG(llvm::dbgs() << "================sub replace op start: ===========\n");
//    OpBuilder builder(op.getContext());
//    // Sets the insertion position of the instruction generation class at the
//    // instruction to be replaced
//    builder.setInsertionPoint(op);
//    // Gets the right-hand/left-hand operand of the instruction, eg. a=b+c
//    Value rsh = op.getRhs(); // c
//    Value lsh = op.getLhs(); // b
//    // Gets type of operand
//    auto elementTy = rsh.getType();
//    // new mlirOP
//    Value newResult;
//
//    int subNum = getRandomSeed(elementTy,1,4);
//    LLVM_DEBUG(llvm::dbgs() << "            " << subNum << "\n");
//    if (subNum == 1) {
//      // a = b + (-c)  output-test14
//      Value zeroExpr = builder.create<arith::ConstantOp>(op->getLoc(), getAttributeConst(elementTy, 0));
//      Value temp1 =builder.create<BinaryArithOp>(op->getLoc(), zeroExpr, rsh); //  -c
//      newResult = builder.create<BinaryReplacementOp>(op->getLoc(), temp1,lsh); //  b + (-c)
//    } else if (subNum == 2) {
//      // r = rand();a = b + r;a = a - c;a = a - r; output-test15
//      int t = rand();
//      Value r = builder.create<arith::ConstantOp>(op->getLoc(), getAttributeConst(elementTy, t));
//      Value temp1 = builder.create<BinaryReplacementOp>(op->getLoc(), lsh, r); //  a = b + r;
//      Value temp2 =builder.create<BinaryArithOp>(op->getLoc(), temp1, rsh); //  a = a - c
//      newResult =builder.create<BinaryArithOp>(op->getLoc(), temp2, r); //  a = a - r
//    } else if (subNum == 3) {
//      // r = rand();a = b - r;a = a - c;a = a + r; output-test16
//      int t = rand();
//      Value r = builder.create<arith::ConstantOp>(op->getLoc(), getAttributeConst(elementTy, t));
//
//      Value temp1 =builder.create<BinaryArithOp>(op->getLoc(), lsh, r); //  a = b - r;
//
//      Value temp2 =builder.create<BinaryArithOp>(op->getLoc(), temp1, rsh); //  a = a - c
//      newResult =builder.create<BinaryReplacementOp>(op->getLoc(), temp2, r); //  a = a + r
//    }
//
//    Value result = op.getResult();
//    result.replaceAllUsesWith(newResult);
//    op.erase();
//  }
//}
//
//
//void LogicalOperationExcute(SmallVector<arith::AndIOp> &andIOps,SmallVector<arith::OrIOp> &orIOps,SmallVector<arith::XOrIOp> &xorIOps){
//  //and
//  for(auto andIOp : andIOps){
//
//    OpBuilder builder(andIOp->getContext());
//    ImplicitLocOpBuilder b = ImplicitLocOpBuilder::atBlockBegin(andIOp->getLoc(),andIOp->getBlock());
//    b.setInsertionPoint(andIOp);
//    Value rsh = andIOp.getRhs();  // c
//    Value lsh = andIOp.getLhs();  // b
//
//    //a=b&c->a=(b^~c)&b  output-test17
//    //      auto elementTy = builder.getI32Type();
//    auto elementTy= rsh.getType();
//    Value num = b.create<arith::ConstantOp>(andIOp->getLoc(),IntegerAttr::get(elementTy, -1));   // get -1
//    Value temp1 = b.create<arith::XOrIOp>(andIOp->getLoc(),rsh,num);   //  get ~c
//    Value temp2 = b.create<arith::XOrIOp>(andIOp->getLoc(),lsh,temp1);   // get (b^~c)
//    Value newResult = b.create<arith::AndIOp>(andIOp->getLoc(),temp2,lsh);   //get (b^~c)&b
//
//    Value result = andIOp.getResult();
//    result.replaceAllUsesWith(newResult);
//    andIOp.erase();
//  }
//  // or
//  for(auto orIOp : orIOps) {
//    LLVM_DEBUG(llvm::dbgs() << orIOp->getName().getStringRef());
//    OpBuilder builder(orIOp->getContext());
//    ImplicitLocOpBuilder b = ImplicitLocOpBuilder::atBlockBegin(orIOp->getLoc(), orIOp->getBlock());
//    b.setInsertionPoint(orIOp);
//    Value rsh = orIOp.getRhs(); // c
//    Value lsh = orIOp.getLhs(); // b
//
//    //a=b|c->a=(b&c)|(b^c) output->test18
//    Value temp1 = b.create<arith::AddIOp>(orIOp->getLoc(),lsh,rsh);  // get b&c
//    Value temp2 = b.create<arith::XOrIOp>(orIOp->getLoc(),lsh,rsh);   // get b^c
//    Value newResult = b.create<arith::OrIOp>(orIOp->getLoc(),temp1,temp2);   // get (b&c)|(b^c)
//
//    Value result = orIOp.getResult();
//    result.replaceAllUsesWith(newResult);
//    orIOp.erase();
//  }
//  // xor
//  for(auto xorIOp : xorIOps) {
//    OpBuilder builder(xorIOp->getContext());
//    ImplicitLocOpBuilder b = ImplicitLocOpBuilder::atBlockBegin(xorIOp->getLoc(), xorIOp->getBlock());
//    b.setInsertionPoint(xorIOp);
//    Value rsh = xorIOp.getRhs(); // c
//    Value lsh = xorIOp.getLhs(); // b
//                                 //      auto elementTy = builder.getI32Type();
//    auto elementTy= rsh.getType();
//    Value oneExpr = b.create<arith::ConstantOp>(xorIOp->getLoc(),IntegerAttr::get(elementTy, 1));
//
//    //a=b^c->a=(!c&b)|(c&!b) output-test19
//    Value temp1 = b.create<arith::XOrIOp>(xorIOp->getLoc(),oneExpr,rsh);   //get !c
//    Value temp2 = b.create<arith::AndIOp>(xorIOp->getLoc(),temp1,lsh);   //get !c&b
//    Value temp3 = b.create<arith::XOrIOp>(xorIOp->getLoc(),oneExpr,lsh);   // get !b
//    Value temp4 = b.create<arith::AndIOp>(xorIOp->getLoc(),rsh,temp3);   // get c&!b
//    Value newResult = b.create<arith::OrIOp>(xorIOp->getLoc(),temp2,temp4);   // get (!c&b)|(c&!b)
//
//    Value result = xorIOp.getResult();
//    result.replaceAllUsesWith(newResult);
//    xorIOp.erase();
//  }
//}
//
//
//
//namespace mlir {
//namespace test {
//void registerSubPass() { PassRegistration<SubPass>(); }
//} // namespace test
//} // namespace mlir
