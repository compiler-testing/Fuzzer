//
// Created by Administrator on 2023/3/4.
//

#ifndef LLVM_OPINFO_H
#define LLVM_OPINFO_H

#endif // LLVM_OPINFO_H
#include "mlir/Dialect/Vector/Utils/VectorUtils.h"
#include <mlir/IR/ImplicitLocOpBuilder.h>
#include "mlir/Conversion/ArmNeon2dToIntr/ArmNeon2dToIntr.h"
#include "mlir/Dialect/ArmNeon/ArmNeonDialect.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
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
#include "json/json.h"


#include <ctime>
#include <cstdlib>
#include <random>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <string>

using namespace mlir;
using namespace std;
//Standard mersenne_twister_engine seeded with rd()

class InfoGen{
public:
  void initInfo(string opname);
  string getRandomOp(llvm::ArrayRef<StringRef> opPool);
  void addInputType(ImplicitLocOpBuilder b);
  void addInputType(ImplicitLocOpBuilder b,Value preOp);

  void addInputs(ImplicitLocOpBuilder b, Location loc,func::FuncOp funcOp);
  void addInputs(ImplicitLocOpBuilder b, Location loc,func::FuncOp funcOp,Value preOp);

  void addAttrs(ImplicitLocOpBuilder b, Location loc);
  void addResult(ImplicitLocOpBuilder b) ;

  string getNextOp(Value curOp);
  void setInputNoMatched(ImplicitLocOpBuilder b, Location loc, Json::Value opConstraint);
  void initFirstOpInfo(ImplicitLocOpBuilder b, Location loc, string first);
  void setFirstOpInput(ImplicitLocOpBuilder b, Location loc,func::FuncOp funcOp);
  void setNextOpInfo(ImplicitLocOpBuilder b, Location loc, Json::Value opConstraint,Value lastOp);
  void addExtraConstrain(ImplicitLocOpBuilder b, Location loc);
  void setInput(Json::Value inputs);
  void setAttr(ImplicitLocOpBuilder b, Location loc, Json::Value attrs);
  void setResult(ImplicitLocOpBuilder b, Json::Value inputs) ;
  SmallVector<string,8> getInputType(Json::Value inputInfo);
  SmallVector<SmallVector<int64_t,8>,8> getInputShapeDIM(Json::Value inputInfo);
  pair<string, SmallVector<int64_t, 8>> getResultInfo(Value op);
  SmallVector<Type,4> Input_Tensor(ImplicitLocOpBuilder b, SmallVector<SmallVector<int64_t,8>,8> inputShapes, SmallVector<string,8> inputTypes);


  void getOpDim( llvm::ArrayRef<StringRef> opPool);
  void getOpType( llvm::ArrayRef<StringRef> opPool);
  void analyseAllOp(llvm::ArrayRef<StringRef> opPool);
};

//声明
struct opInfo{
  string opName;
  int inputNum;
  int attsNum;
  bool needBroadcast;

  SmallVector<Type,8> inputType;
  SmallVector<NamedAttribute> attrs;
  Type resultType;
  SmallVector<Value,8> inputs;
  Json::Value Constraint;

  int concat_axis;
};



//extern pair<string, SmallVector<int64_t, 8>> constraint_info;