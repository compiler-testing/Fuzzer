//
// Created by Administrator on 2023/3/4.
//

#ifndef LLVM_CREATE_H
#define LLVM_CREATE_H

#endif // LLVM_CREATE_H
#include "mlir/Dialect/Vector/Utils/VectorUtils.h"
#include <mlir/IR/ImplicitLocOpBuilder.h>
#include "mlir/Conversion/ArmNeon2dToIntr/ArmNeon2dToIntr.h"
#include "mlir/Dialect/ArmNeon/ArmNeonDialect.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "llvm/IR/Attributes.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/IR/FunctionInterfaces.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/IR/Builders.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/Tosa/IR/TosaOps.h"
#include "mlir/IR/Attributes.h"
#include "llvm/ADT/DenseMap.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Types.h"
#include "mlir/IR/Value.h"

#include <ctime>
#include <cstdlib>
#include <random>
#include <iostream>
#include "mlir/json/json.h"
#include<iostream>
#include<fstream>
#include<assert.h>
#include<random>
#include <string>
using namespace mlir;
using namespace std;

class Create {
public:
  void insertFuncArg(ImplicitLocOpBuilder b, func::FuncOp funcOp,SmallVector<Type, 8> argTypes);
  Value createNewBranch(ImplicitLocOpBuilder b,Location loc,func::FuncOp funcOp,string selectedOp);

  Value createOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc);
  Value createOpWithNoAttr(ImplicitLocOpBuilder b, Location loc);
  Value createOpWithOneAttr(ImplicitLocOpBuilder b, Location loc);
  Value createOpWithMulAttrs(ImplicitLocOpBuilder b, Location loc);

};