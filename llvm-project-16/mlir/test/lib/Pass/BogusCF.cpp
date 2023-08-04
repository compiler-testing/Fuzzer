
//
// Created by nisl888 on 2022/2/15.
//
#include "mlir/Conversion/ArmNeon2dToIntr/ArmNeon2dToIntr.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ArmNeon/ArmNeonDialect.h"
#include <mlir/IR/ImplicitLocOpBuilder.h>

#include "TestDialect.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/Tosa/IR/TosaOps.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Block.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/FunctionInterfaces.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/MatrixBuilder.h"
#include "llvm/IR/Operator.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
using namespace std;
using namespace mlir;

#define DEBUG_TYPE "user-define-pass"


namespace {

struct BcfPass : public PassWrapper<BcfPass, OperationPass<func::FuncOp>> {
 MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(BcfPass)
 StringRef getArgument() const final { return "BCF"; }
 StringRef getDescription() const final { return "Bogus Control Flow"; }

 void getDependentDialects(DialectRegistry &registry) const override {
   registry.insert<scf::SCFDialect, memref::MemRefDialect>();
 }
 // random split block
 Block *randomSplit(Block *bb, int flag);
 void copyBb(Block *bb, Block *clonebb);
 Value CreateTrueCond(ImplicitLocOpBuilder builder,Location loc );
 void runOnOperation() override;
};
} // namespace
std::random_device rd2;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen2(rd2());


void BcfPass::runOnOperation() {
 func::FuncOp f = getOperation();
 func::FuncOp::iterator fi = f.getBlocks().begin();
 func::FuncOp::iterator fe = f.getBlocks().end();

 SmallVector<Block *,16> blockStack;

 while(fi != fe) {
   blockStack.push_back(&(*fi));
   fi++;
 }

 std::uniform_int_distribution<unsigned> randomNum(0, blockStack.size()-1);
 // iterator blocks

     Block * bb;
    do{
    bb = blockStack[randomNum(gen2)];
    }while(&(*bb->begin())==bb->getTerminator());

    int times = 2;

//  for (Block *b : blockStack) {
  while (times--) {
   LLVM_DEBUG(llvm::dbgs() << "========Block exec Start===========\n");
   LLVM_DEBUG(bb->dump());

   if (bb->getOperations().size() <= 2)
     continue;
   Location loc = bb->begin()->getLoc();
   ImplicitLocOpBuilder builder = ImplicitLocOpBuilder::atBlockBegin(loc, bb);

   builder.setInsertionPointToStart(bb);
   auto oneType = builder.getI1Type();

   f.dump();
    Value cond =
        builder.create<arith::ConstantOp>(loc, IntegerAttr::get(oneType, 1));

   // split bb
   randomSplit(bb, 1);

   // create clonebb
   Block *bb1 = bb->getNextNode();
   Block *clonebb = builder.createBlock(bb1);

   builder.setInsertionPointToEnd(bb);
   bb->getTerminator()->erase();
   builder.create<cf::CondBranchOp>(cond, bb1, clonebb);

   builder.setInsertionPointToEnd(clonebb);
   copyBb(bb1, clonebb);
   builder.create<cf::BranchOp>(bb1);

   builder.setInsertionPointToStart(bb);
   Value cond1 = CreateTrueCond(builder,loc);
   cond.replaceAllUsesWith(cond1);
   //          bb
   //        /    \
   //     bb1 <-> clonebb
   //     /
   //   bb2
   if (bb1->getOperations().size() >= 3) {
     randomSplit(bb1, 0);
     bb1->back().erase();
     builder.setInsertionPointToEnd(bb1);
     auto oneType = builder.getI1Type();
     Value cond =
         builder.create<arith::ConstantOp>(loc, IntegerAttr::get(oneType, 1));
     builder.create<cf::CondBranchOp>(cond, bb1->getNextNode(), clonebb);

     builder.setInsertionPointToStart(bb1);
     Value cond1 = CreateTrueCond(builder,loc);
     cond.replaceAllUsesWith(cond1);
   }
   do{
     bb = blockStack[randomNum(gen2)];
   }while(&(*bb->begin())==bb->getTerminator());
 }
}

Value BcfPass::CreateTrueCond(ImplicitLocOpBuilder builder,Location loc){
 auto IntType = builder.getIntegerType(32);
 auto zero =  builder.create<arith::ConstantOp>(loc, IntegerAttr::get(IntType, 0));
 auto x =  builder.create<arith::ConstantOp>(loc, IntegerAttr::get(IntType, 0));
 auto y =  builder.create<arith::ConstantOp>(loc, IntegerAttr::get(IntType, 0));

 auto c10 =  builder.create<arith::ConstantOp>(loc, IntegerAttr::get(IntType, 10));
 auto c2 =  builder.create<arith::ConstantOp>(loc, IntegerAttr::get(IntType, 2));

 auto x_1 = builder.create<arith::SubIOp>(loc, x, c10);
 auto x_x_1 = builder.create<arith::MulIOp>(loc, x, x_1);
 auto x_x_1_mod_2 = builder.create<arith::RemUIOp>(loc, x_x_1, c2);

 auto y_lt_c10 = builder.create<arith::CmpIOp>(loc, arith::CmpIPredicate::slt, y, c10);
 auto x_x_1_mod_2_eq_0 = builder.create<arith::CmpIOp>(loc, arith::CmpIPredicate::eq, x_x_1_mod_2,  zero);

 Value cond1 = builder.create<arith::OrIOp>(loc, y_lt_c10, x_x_1_mod_2_eq_0);
 return cond1;
}

Block *BcfPass::randomSplit(Block *bb, int flag) {

 SmallVector<Operation *, 16> ops;
 Operation *lastOp = bb->getTerminator();
 for (auto &op : *bb) {
   if (&op != lastOp)
     ops.push_back(&op);
 }

 Block *clonebb = nullptr;

 Location loc = bb->begin()->getLoc();
 ImplicitLocOpBuilder b = ImplicitLocOpBuilder::atBlockBegin(loc, bb);

 // clone bb
 if (&(bb->getParent()->back()) == bb) {
   clonebb = b.createBlock(bb->getParent(), bb->getParent()->end());
 } else {
   clonebb = b.createBlock(bb->getNextNode());
 }

 unsigned seed = time(0);
 srand(seed);
 int scope1 = 0;
 int scope2 = ops.size();
 int splitPoint = (rand() % (scope2 - scope1)) + scope1;

 if (flag == 1)
   splitPoint = 1;

 LLVM_DEBUG(llvm::dbgs() << "========Block splitPoint===========\n");
 LLVM_DEBUG(ops[splitPoint]->dump());

 SmallVector<Operation *, 16>
     moveOps; //{&ops[split_point], &ops[ops.size()-1]};

 auto *oi = ops.begin();
 auto *oe = ops.end();
 for (int n = 0; n < splitPoint; n++) {
   oi++;
 }
 moveOps.assign(oi, oe);

 for (auto *op : moveOps) {
   op->moveBefore(clonebb, clonebb->end());
 }

 b.setInsertionPointToEnd(bb);
 b.create<cf::BranchOp>(clonebb);

 lastOp->moveBefore(clonebb, clonebb->end());

 return clonebb;
}

void BcfPass::copyBb(Block *bb, Block *clonebb) {
 IRMapping mapper;

 mapper.map(bb, clonebb);
 for (auto arg : bb->getArguments())
   if (!mapper.contains(arg))
     mapper.map(arg, clonebb->addArgument(arg.getType(), arg.getLoc()));

 for (auto &op : *bb) {
   if (&op != bb->getTerminator())
     clonebb->push_back(op.clone(mapper));
 }
}

namespace mlir {
namespace test {
void registerBcfPass() { PassRegistration<BcfPass>(); }
} // namespace test
} // namespace mlir