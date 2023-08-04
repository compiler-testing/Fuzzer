#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/FunctionInterfaces.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"
#include "TestDialect.h"
#include <iostream>
#include <mlir/IR/ImplicitLocOpBuilder.h>
#include "llvm/Support/Debug.h"
#include <random>

using namespace mlir;
using namespace std;

std::random_device rd1;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen1(rd1());

#define DEBUG_TYPE "user-define-pass"

namespace {
struct TGPass : public PassWrapper<TGPass, OperationPass<func::FuncOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(TGPass)
  void copyBb(Block *bb, Block *clonebb);
  Value CreateTrueCond(ImplicitLocOpBuilder builder,Location loc );
  void runOnOperation() override {

    auto f = getOperation();

    func::FuncOp::iterator fi = f.getBlocks().begin();
    func::FuncOp::iterator fe = f.getBlocks().end();

    SmallVector<Block *,16> blockStack;

    while(fi != fe) {
      blockStack.push_back(&(*fi));
      fi++;
    }

    std::uniform_int_distribution<unsigned> randomNum(0, blockStack.size()-1);

    Block * bb;
    do{
    bb = blockStack[randomNum(gen1)];
    }while(&(*bb->begin())==bb->getTerminator());

    int times = 2;
    while (times--) {
      LLVM_DEBUG(llvm::dbgs()
                 << "===============block dump======================\n");
//      bb->dump();

      SmallVector<Operation *, 16> allOps;
      SmallVector<Operation *, 16> rawOps;
      SmallVector<Value> hasReturnValueOps;
      SmallVector<Type> returnOpsTypes;
      Location loc = bb->begin()->getLoc();
      ImplicitLocOpBuilder b = ImplicitLocOpBuilder::atBlockBegin(loc, bb);

      Operation *lastOp = bb->getTerminator();

      for (auto &op : *bb) {
        if (&op != lastOp) {
          if(op.getDialect()->getNamespace().str()=="scf" || op.getDialect()->getNamespace().str()=="affine")
            break;
          allOps.push_back(&op);
          if ((&op)->getNumResults() != 0) {
            //TODO think of some operation which has more than one return values
            hasReturnValueOps.push_back(op.getResult(0));
            returnOpsTypes.push_back(op.getResult(0).getType());
          }
        }
      }

      auto bi = allOps.begin();
      if (allOps.size()>1){
        std::uniform_int_distribution<unsigned> randomNum(1, allOps.size()-1);
        int n = randomNum(gen1);
        while(n>0){
          bi++;
          n--;
        }
        rawOps.assign(bi,allOps.end());
      }else{
        rawOps = allOps;
      }


      if (rawOps.empty()){
        do{
          bb = blockStack[randomNum(gen1)];
        }while(&(*bb->begin())==bb->getTerminator());
        continue;
      }


      SmallVector<Type> values;
      for (auto op : rawOps) {
        if ((op)->getNumResults() != 0) { //&& (&op)->getNumOperands()!=0
          values.push_back(op->getResult(0).getType());
        }
      }

      b.setInsertionPoint(rawOps[0]);
      Value cond = b.create<arith::ConstantOp>(loc, b.getI1Type(), b.getBoolAttr(true));

      scf::IfOp ifOP  = b.create<scf::IfOp>(loc, values, cond, true);
      for (Operation* op : rawOps) {
        op->moveBefore(ifOP.thenBlock(), ifOP.thenBlock()->end());
      }

      SmallVector<Value> values1; // return value #ThenBlock
      SmallVector<Value> values2; // return value #ElseBolck

      for (auto &op : *ifOP.thenBlock()) {
        if ((&op)->getNumResults() != 0) { //&& (&op)->getNumOperands()!=0
          values1.push_back(op.getResult(0));
        }
      }
      b.setInsertionPointToEnd(ifOP.thenBlock());
      b.create<scf::YieldOp>(loc, ValueRange(values1));

      b.setInsertionPointToEnd(ifOP.elseBlock());

      // create clonebb

      copyBb(ifOP.thenBlock(), ifOP.elseBlock());

      for (auto &op : *ifOP.elseBlock()) {
        if ((&op)->getNumResults() != 0) { //&& (&op)->getNumOperands()!=0
          values2.push_back(op.getResult(0));
        }
      }
      b.create<scf::YieldOp>(loc, ValueRange(values2));

      b.setInsertionPointToStart(cond.getParentBlock());
      Value cond1 = CreateTrueCond(b,loc);
      cond.replaceAllUsesWith(cond1);
      SmallPtrSet<Operation *, 16> opsUsers;

      Block *ifbb = ifOP.thenBlock();

      ifbb->walk([&](Operation *op) { opsUsers.insert(op); });

      int n = 0;
      for (auto op : rawOps) {
        Value v1 = ifOP.getResult(n);
        op->getResult(0).replaceAllUsesExcept(v1, opsUsers);
        //        op->getResult(0).replaceAllUsesWith(v1);
        n++;
      }
      do{
        bb = blockStack[randomNum(gen1)];
      }while(&(*bb->begin())==bb->getTerminator());
    }
  };

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<arith::ArithDialect, scf::SCFDialect,
                    memref::MemRefDialect>();
  }
  StringRef getArgument() const final { return "TG"; }
  StringRef getDescription() const final { return "Always true guard pass"; }
};

} // namespace

void TGPass::copyBb(Block *bb, Block *clonebb) {
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

Value TGPass::CreateTrueCond(ImplicitLocOpBuilder builder,Location loc){
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
namespace mlir {
namespace test {
void registerTGPass() { PassRegistration<TGPass>(); }
} // namespace test
} // namespace mlir
