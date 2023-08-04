//
// Created by Administrator on 2023/3/4.
//

#ifndef LLVM_UTILS_H
#define LLVM_UTILS_H

#endif // LLVM_UTILS_H
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

class Utils {
public:
  SmallVector<int64_t, 8> GenRandomShape(int Dim);
  unsigned int genRandomN(unsigned int begin, unsigned int end);
  string getRandomOp(llvm::ArrayRef<StringRef> opPool);
  int getElementNum(llvm::SmallVector<int64_t , 8> shape);
  int getElementNum(llvm::ArrayRef<int64_t> shape);

  Json::Value getConstraint(StringRef selectedOp);
  llvm::SmallVector<int64_t, 8> genShape(int index, int Dim, SmallVector<SmallVector<int64_t, 8>, 8> inputshapes);
  llvm::SmallVector<int64_t , 8> genConstrainedShape(SmallVector<int64_t , 8> shape1);
  string getTypestr(string typeStr);
  string type2str(mlir::Type type);
  SmallVector<int64_t , 8> getPrimeFactors(int x) ;
  llvm::SmallVector<int64_t, 8> genBroadcastShape();
  string getTensorType(Type tensor);
  SmallVector<int64_t, 8> getTensorShape(Type tensor);
  Type str2type(ImplicitLocOpBuilder b, SmallVector<int64_t,8> inputShape, string elementType);
  Type str2type(ImplicitLocOpBuilder b, llvm::ArrayRef<int64_t> inputShape, string elementType);

  //重载函数，根据输入的shape和tensor，返回tensor type
  Type genTensorType(ImplicitLocOpBuilder b, SmallVector<int64_t,8> inputShape, string elementType);
  Type genTensorType(ImplicitLocOpBuilder b, llvm::ArrayRef<int64_t> inputShape, string elementType);


  bool typeMatch(Value preOp);
  // Value typeMatch(func::FuncOp funcOp);
  SmallVector<Value> typeMatch(SmallVector<Value> valuePool);
  Value skipMatch(SmallVector<Value> valuePool);
  bool addSkips(Value v);

  SmallVector<SmallVector<int64_t,8>,8> getDimFromopConstraint();
  SmallVector<string,8> getTypeFromopConstraint();
  SmallVector<Value> collectInsertPoint(func::FuncOp funcOp);
  void printTypes(SmallVector<Type,8> tensors);
  void printValues(SmallVector<Value,8> tensors);

  void getOpDim( llvm::ArrayRef<StringRef> opPool);
  void getOpType( llvm::ArrayRef<StringRef> opPool);
  void analyseAllOp(llvm::ArrayRef<StringRef> opPool);

  string getNextOp(mlir::Value curOp) ;
  llvm::SmallVector<int64_t, 8> getShapeVector(llvm::ArrayRef<int64_t> shape);

  DenseIntOrFPElementsAttr getDenseAttr(ImplicitLocOpBuilder b,string typeStr,llvm::SmallVector<int64_t , 8> shape1);

  };


