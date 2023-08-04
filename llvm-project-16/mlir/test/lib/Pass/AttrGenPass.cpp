#include "TestDialect.h"
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
#include "llvm/Support/Debug.h"
#include <iostream>
#include <mlir/IR/ImplicitLocOpBuilder.h>
#include "mlir/Dialect/Tosa/IR/TosaOps.h"
#include "mlir/Dialect/Tosa/Transforms/Passes.h"
#include "mlir/Dialect/Tosa/Utils/ShapeUtils.h"
#include "mlir/Pass/Pass.h"
#include <random>
#include "cstdlib"

using namespace mlir;
using namespace std;

#define DEBUG_TYPE "user-define-pass"
std::random_device rdNum;  //Will be used to obtain a seed for the random number engine
std::mt19937 Getgen(rdNum());


llvm::SmallVector<int64_t> ChangeAttrElem(llvm::SmallVector<int64_t> attrElem, string attrName){
  llvm::SmallVector<int64_t> MutaAttrElem;
  std::uniform_int_distribution<unsigned> randomBig(0, 126549185);
  std::uniform_int_distribution<unsigned> randomSmall(1, 5);

  if(attrName=="pad" || attrName=="out_pad"){
    for (int i = 0; i < attrElem.size(); i++) {
      MutaAttrElem.push_back(randomBig(Getgen));
    }
  }else if(attrName=="stride" || attrName=="kernel" || attrName=="dilation"){
    for (int i = 0; i < attrElem.size(); i++) {
      MutaAttrElem.push_back(randomSmall(Getgen));
    }
  }else if (attrName=="out_shape"){
    std::uniform_int_distribution<unsigned> randomOutShape(-1, 10);
    for (int i = 0; i < attrElem.size(); i++) {
      MutaAttrElem.push_back(randomSmall(Getgen));
    }
  }
  return MutaAttrElem;
}


namespace {
struct AttrGenPass
    : public PassWrapper<AttrGenPass, OperationPass<func::FuncOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(AttrGenPass)
  void runOnOperation() override {
    auto funcOp = getOperation();



    for (auto &bb : funcOp) {

      Location loc = bb.begin()->getLoc();
      ImplicitLocOpBuilder b = ImplicitLocOpBuilder::atBlockBegin(loc, &bb);
      b.setInsertionPointToStart(&bb);

      for (auto &op:bb) {
        auto opName = op.getName().getStringRef().str();
        cout << op.getName().getStringRef().str()<<endl;  //op名
        cout << op.getNumOperands()<<endl;  //操作数个数

        //OpResult LastOpResult;

        if(op.getName().getStringRef().str()=="func.return"){
          op.getOperand(0).dump();
          auto result = op.getResult(0);
          //result.replaceAllUsesWith(LastOpResult);

          break;
        }

        //ValueRange inputs;
        llvm::SmallVector<Value> inputs;
        llvm::SmallVector<ShapedType> inputsTy;
        llvm::SmallVector<Type> inputsETy;
        llvm::SmallVector<llvm::ArrayRef<int64_t>> inputsElem;
        //1.通过这个for循环，入类型，输入元素类型，输入元素数值存入数组中，输入包括input1，input2，...
        for (int i = 0; i < op.getNumOperands(); i++) {
          //          Value input;
          //          ShapedType inputTy;   //tensor<1x45x40x28xf32>
          //          Type inputETy;        //f32
          //          llvm::ArrayRef<int64_t> inputElem;  //[1,45,40,28]
          Value input = op.getOperand(i);
          ShapedType inputTy = input.getType().cast<ShapedType>();
          Type inputETy = inputTy.getElementType();
          llvm::ArrayRef<int64_t> inputElem = inputTy.getShape();

          inputs.push_back(input);
          inputsTy.push_back(inputTy);
          inputsETy.push_back(inputETy);
          inputsElem.push_back(inputElem);
        }

        ShapedType resultTy = op.getResult(0).getType().cast<ShapedType>();
        Type resultETy = resultTy.getElementType();
        llvm::SmallVector<int64_t> resultElem;
        //LastOpResult = op.getResult(0).getType().cast<ShapedType>();


        llvm::ArrayRef<mlir::NamedAttribute> attrList = op.getAttrs();
        llvm::SmallVector<StringAttr> attrsName;
        //llvm::SmallVector<Type> OneAttrsElem;
        llvm::SmallVector<llvm::SmallVector<int64_t>> attrsElem;
        llvm::SmallVector<Attribute> attrsValue;
        llvm::SmallVector<Type> attrsETy;

        if (op.getAttrs().size() == 1){
          StringAttr attrName = op.getAttrs()[0].getName();
          Attribute attrValue = op.getAttrs()[0].getValue();
          if(attrName == "axis"){   //属性是单个值，不是数组
            //op.getAttr("axis").cast<IntegerAttr>().getValue().getSExtValue();
            auto attrElem = attrValue.cast<IntegerAttr>().getValue().getSExtValue();
            int inputDim = inputsTy[0].getRank();
            std::uniform_int_distribution<unsigned> randomAxis(1, inputDim-1);
            int MutaAxis = randomAxis(Getgen);
            if(opName == "tosa.reduce_sum"||opName == "tosa.reduce_prod"||opName == "tosa.reduce_min"
                ||opName == "tosa.reduce_max" ||opName == "tosa.reduce_all" ||opName == "tosa.reduce_any" ){
              for (int i = 0; i < resultTy.getShape().size(); i++) {
                resultElem.push_back(inputsElem[0][i]);
              }
              resultElem[MutaAxis] = 1;
              op.setAttr("axis",b.getI64IntegerAttr(MutaAxis));
              op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));
              //b.create<tosa::ReduceSumOp>(loc,RankedTensorType::get(resultElem,resultETy),inputs[0],b.getI64IntegerAttr(MutaAxis));
            }

            else if (opName == "tosa.concat"){
              //如果aixs=
              resultElem[MutaAxis] = inputsElem[0][MutaAxis] + inputsElem[1][MutaAxis];
              op.setAttr("axis",b.getI64IntegerAttr(MutaAxis));
              op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));
              //b.create<tosa::ConcatOp>(loc,RankedTensorType::get(resultElem,resultETy),ValueRange{inputs[0],inputs[1]},b.getI64IntegerAttr(MutaAxis));
            }
            else if (opName == "tosa.argmax"){
              for (int i = 0; i < resultTy.getShape().size(); i++) {
                resultElem.push_back(inputsElem[0][i]);
              }
              SmallVector<int, 8> newShape;
              auto SI = resultElem.begin();
              for (int j = 0; j < resultElem.size(); j++) {
                if (j == MutaAxis)
                  break;
                else
                  SI++;
              }
              resultElem.erase(SI);
              op.setAttr("axis",b.getI64IntegerAttr(MutaAxis));
              op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));
              //b.create<tosa::ArgMaxOp>(loc,RankedTensorType::get(resultElem,resultETy),inputs[0],b.getI64IntegerAttr(MutaAxis));
            }
            else if (opName == "tosa.reverse"){
              op.setAttr("axis",b.getI64IntegerAttr(MutaAxis));
            }
          }
          else if(attrName == "round"){}
          else if (attrName == "shift"){
            std::uniform_int_distribution<> randomShift(-128,127);
            int MutaShift = randomShift(Getgen);
            op.setAttr("shift",b.getI32IntegerAttr(MutaShift));
            //op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));
          }
          else if (attrName == "new_shape"){}
          else if (attrName == "multiples"){
            std::uniform_int_distribution<uint64_t> randomMultiple(0, 100);
            llvm::SmallVector<int64_t> multipleElem = extractFromI64ArrayAttr(op.getAttr("multiples"));
            multipleElem[0] = randomMultiple(Getgen);
            multipleElem[1] = randomMultiple(Getgen);
            resultElem.push_back( (multipleElem[0]) * (inputsElem[0][0]) ) ;
            resultElem.push_back( (multipleElem[1]) * (inputsElem[0][1]) ) ;
            op.setAttr("multiple",b.getI64ArrayAttr(multipleElem));
            op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));
          }
//          else if (attrName == "perm"){
//            //处理permOp前，有一个arith的常量Op，对这个Op也要进行变异
//            llvm::SmallVector<int64_t> permElem ;
//            for (int i = inputsElem[0].size()-1; i >= 0; i--) {
//              permElem.push_back(i);
//            }
//            for (int i = inputsElem[0].size()-1; i >= 0; i--) {
//              resultElem.push_back(inputsElem[0][i]);
//            }
//
//          }
          else if (opName == "tosa.table"){}
          else if (opName == "tosa.select"){}
        }


        else if (op.getAttrs().size() > 1){
          for (int i = 0; i < op.getAttrs().size(); i++) {
            StringAttr attrName = attrList[i].getName();
            Attribute attrValue = attrList[i].getValue();
          //  Type attrETy = attrValue.

            attrsName.push_back(attrName);
            attrsValue.push_back(attrValue);
//            attrsETy.push_back(attrETy);
          }

          if (opName == "tosa.conv2d"){
            llvm::SmallVector<int64_t> dilationElem = extractFromI64ArrayAttr(op.getAttr("dilation"));
            llvm::SmallVector<int64_t> padElem = extractFromI64ArrayAttr(op.getAttr("pad"));
            llvm::SmallVector<int64_t> strideElem = extractFromI64ArrayAttr(op.getAttr("stride"));

            llvm::SmallVector<int64_t> MutaPadElem;
            llvm::SmallVector<int64_t> MutaStrideElem;
            llvm::SmallVector<int64_t> MutaDilationElem;

            MutaPadElem = ChangeAttrElem(padElem,"pad");
            MutaStrideElem = ChangeAttrElem(strideElem,"stride");
            MutaDilationElem = ChangeAttrElem(dilationElem,"dilation");


            // OH == idiv_check(IH - 1 + pad_top + pad_bottom - (KH - 1) * dilation_y, stride_y) + 1
            // OW == idiv_check(IW - 1 + pad_left + pad_right - (KW - 1) * dilation_x, stride_x) + 1

            //3.2.计算输出的shape
            auto OH = div(inputsElem[0][1]-1+MutaPadElem[0]+MutaPadElem[1]-(inputsElem[1][1]-1)*MutaDilationElem[0],MutaStrideElem[0]).quot+1 ;
            auto OW = div(inputsElem[0][2]-1+MutaPadElem[2]+MutaPadElem[3]-(inputsElem[1][2]-1)*MutaDilationElem[1],MutaStrideElem[1]).quot+1 ;
            auto N = inputsElem[0][0];
            auto OC = inputsElem[2][0];

            resultElem.push_back(N);
            resultElem.push_back(OH);
            resultElem.push_back(OW);
            resultElem.push_back(OC);

            //3.3.创建变异后的Op
            op.setAttr("dilation",b.getI64ArrayAttr(MutaDilationElem));
            op.setAttr("pad",b.getI64ArrayAttr(MutaPadElem));
            op.setAttr("stride",b.getI64ArrayAttr(MutaStrideElem));
            op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));

          }
          else if (opName == "tosa.slice") {
            llvm::SmallVector<int64_t> startElem = extractFromI64ArrayAttr(op.getAttr("start"));
            llvm::SmallVector<int64_t> sizeElem = extractFromI64ArrayAttr(op.getAttr("size"));

            for (int i = 0; i < inputsTy[0].getRank(); i++) {
              std::uniform_int_distribution<int64_t> randomStart(0, inputsElem[0][i]-1);
              int EveryDimStart = randomStart(Getgen);
              startElem[i] = EveryDimStart;
              std::uniform_int_distribution<int64_t> randomSize(1,inputsElem[0][i]-EveryDimStart);
              int EveryDimSize = randomSize(Getgen);
              sizeElem[i] = EveryDimSize;
            }

            for (int i = 0; i < sizeElem.size(); i++) {
              resultElem.push_back(sizeElem[i]);
            }

            op.setAttr("start",b.getI64ArrayAttr(startElem));
            op.setAttr("size",b.getI64ArrayAttr(sizeElem));
            op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));

          }
          else if (opName == "tosa.reluN") {
            std::uniform_real_distribution<> randomMaxFp(0, 100);
            std::uniform_int_distribution<> randomMaxInt(0, 100);
            auto maxFp = randomMaxFp(Getgen);
            auto maxInt = randomMaxInt(Getgen);

            for (int i = 0; i < resultTy.getRank(); i++) {
              resultElem.push_back(inputsElem[0][i]);
            }

            if (inputsETy[0].isa<FloatType>()){
              for (int i = 0; i < resultElem.size(); i++) {
                if (resultElem[i] < 0){
                  resultElem[i] = 0;
                }
                else if (resultElem[i] > maxFp){
                  resultElem[i] = maxFp;
                }
              }
            }
            if (inputsETy[0].isa<IntegerType>()){
              for (int i = 0; i < resultElem.size(); i++) {
                if (resultElem[i] < 0){
                  resultElem[i] = 0;
                }
                else if (resultElem[i] > maxInt){
                  resultElem[i] = maxInt;
                }
              }
            }

            auto max_fpETy = getElementTypeOrSelf(op.getAttr("max_fp"));
            auto max_intETy = getElementTypeOrSelf(op.getAttr("max_int"));
            op.setAttr("max_fp",b.getFloatAttr(max_fpETy,maxFp));
            op.setAttr("max_int",b.getIntegerAttr(max_intETy,maxInt));
            op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));

          }
          else if (opName == "tosa.max_pool2d") {
            llvm::SmallVector<int64_t> padElem = extractFromI64ArrayAttr(op.getAttr("pad"));
            llvm::SmallVector<int64_t> kernelElem = extractFromI64ArrayAttr(op.getAttr("kernel"));
            llvm::SmallVector<int64_t> strideElem = extractFromI64ArrayAttr(op.getAttr("stride"));

            llvm::SmallVector<int64_t> MutaPadElem;
            llvm::SmallVector<int64_t> MutaKernelElem;
            llvm::SmallVector<int64_t> MutaStrideElem;

            MutaPadElem = ChangeAttrElem(padElem,"pad");
            MutaKernelElem = ChangeAttrElem(kernelElem,"kernel");
            MutaStrideElem = ChangeAttrElem(strideElem,"stride");

            // OH = idiv_check(IH + pad_top + pad_bottom - kernel_y, stride_y) + 1;
            // OW = idiv_check(IW + pad_left + pad_right - kernel_x, stride_x) + 1;

            auto OH = div(inputsElem[0][1]-1+MutaPadElem[0]+MutaPadElem[1]-MutaKernelElem[0],MutaStrideElem[0]).quot+1 ;
            auto OW = div(inputsElem[0][2]-1+MutaPadElem[2]+MutaPadElem[3]-MutaKernelElem[1],MutaStrideElem[1]).quot+1 ;

            resultElem.push_back(inputsElem[0][0]);
            resultElem.push_back(OH);
            resultElem.push_back(OW);
            resultElem.push_back(inputsElem[0][3]);

            op.setAttr("kernel",b.getI64ArrayAttr(MutaKernelElem));
            op.setAttr("pad",b.getI64ArrayAttr(MutaPadElem));
            op.setAttr("stride",b.getI64ArrayAttr(MutaStrideElem));
            op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));

          }
          else if (opName == "tosa.depthwise_conv2d") {
            llvm::SmallVector<int64_t> dilationElem = extractFromI64ArrayAttr(op.getAttr("dilation"));
            llvm::SmallVector<int64_t> padElem = extractFromI64ArrayAttr(op.getAttr("pad"));
            llvm::SmallVector<int64_t> strideElem = extractFromI64ArrayAttr(op.getAttr("stride"));

            llvm::SmallVector<int64_t> MutaPadElem;
            llvm::SmallVector<int64_t> MutaStrideElem;
            llvm::SmallVector<int64_t> MutaDilationElem;

            MutaPadElem = ChangeAttrElem(padElem,"pad");
            MutaStrideElem = ChangeAttrElem(strideElem,"stride");
            MutaDilationElem = ChangeAttrElem(dilationElem,"dilation");

            //OH == idiv_check(IH - 1 + pad_top + pad_bottom - (KH - 1) * dilation_y, stride_y) + 1
            //OW == idiv_check(IW - 1 + pad_left + pad_right - (KW - 1) * dilation_x, stride_x) + 1

            //3.2.计算输出的shape
            auto OH = div(inputsElem[0][1]-1+MutaPadElem[0]+MutaPadElem[1]-(inputsElem[1][0]-1)*MutaDilationElem[0],MutaStrideElem[0]).quot+1 ;
            auto OW = div(inputsElem[0][2]-1+MutaPadElem[2]+MutaPadElem[3]-(inputsElem[1][1]-1)*MutaDilationElem[1],MutaStrideElem[1]).quot+1 ;
            auto N = inputsElem[0][0];
            auto OC = inputsElem[2][0];

            resultElem.push_back(N);
            resultElem.push_back(OH);
            resultElem.push_back(OW);
            resultElem.push_back(OC);

            //3.3.创建变异后的Op
            op.setAttr("dilation",b.getI64ArrayAttr(MutaDilationElem));
            op.setAttr("pad",b.getI64ArrayAttr(MutaPadElem));
            op.setAttr("stride",b.getI64ArrayAttr(MutaStrideElem));
            op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));

          }
          else if (opName == "tosa.avg_pool2d") {
            llvm::SmallVector<int64_t> padElem = extractFromI64ArrayAttr(op.getAttr("pad"));
            llvm::SmallVector<int64_t> kernelElem = extractFromI64ArrayAttr(op.getAttr("kernel"));
            llvm::SmallVector<int64_t> strideElem = extractFromI64ArrayAttr(op.getAttr("stride"));

            llvm::SmallVector<int64_t> MutaPadElem;
            llvm::SmallVector<int64_t> MutaKernelElem;
            llvm::SmallVector<int64_t> MutaStrideElem;

            MutaPadElem = ChangeAttrElem(padElem,"pad");
            MutaKernelElem = ChangeAttrElem(kernelElem,"kernel");
            MutaStrideElem = ChangeAttrElem(strideElem,"stride");

            // OH = idiv_check(IH + pad_top + pad_bottom - kernel_y, stride_y) + 1;
            // OW = idiv_check(IW + pad_left + pad_right - kernel_x, stride_x) + 1;

            auto OH = div(inputsElem[0][1]-1+MutaPadElem[0]+MutaPadElem[1]-MutaKernelElem[0],MutaStrideElem[0]).quot+1 ;
            auto OW = div(inputsElem[0][2]-1+MutaPadElem[2]+MutaPadElem[3]-MutaKernelElem[1],MutaStrideElem[1]).quot+1 ;

            resultElem.push_back(inputsElem[0][0]);
            resultElem.push_back(OH);
            resultElem.push_back(OW);
            resultElem.push_back(inputsElem[0][3]);

            op.setAttr("kernel",b.getI64ArrayAttr(MutaKernelElem));
            op.setAttr("pad",b.getI64ArrayAttr(MutaPadElem));
            op.setAttr("stride",b.getI64ArrayAttr(MutaStrideElem));
            op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));

          }
          else if (opName == "tosa.conv3d") {
            llvm::SmallVector<int64_t> dilationElem = extractFromI64ArrayAttr(op.getAttr("dilation"));
            llvm::SmallVector<int64_t> padElem = extractFromI64ArrayAttr(op.getAttr("pad"));
            llvm::SmallVector<int64_t> strideElem = extractFromI64ArrayAttr(op.getAttr("stride"));

            llvm::SmallVector<int64_t> MutaPadElem;
            llvm::SmallVector<int64_t> MutaStrideElem;
            llvm::SmallVector<int64_t> MutaDilationElem;

            MutaPadElem = ChangeAttrElem(padElem,"pad");
            MutaStrideElem = ChangeAttrElem(strideElem,"stride");
            MutaDilationElem = ChangeAttrElem(dilationElem,"dilation");

            // OD = idiv_check(ID - 1 + pad_d0 + pad_d1 - (KD - 1) * dilation_d, stride_d) + 1
            // OH = idiv_check(IH - 1 + pad_top + pad_bottom - (KH - 1) * dilation_y, stride_y) + 1
            // OW = idiv_check(IW - 1 + pad_left + pad_right - (KW - 1) * dilation_x, stride_x) + 1

            //3.2.计算输出的shape
            auto OD = div(inputsElem[0][1]-1+MutaPadElem[0]+MutaPadElem[1]-(inputsElem[1][1]-1)*MutaDilationElem[0],MutaStrideElem[0]).quot+1;
            auto OH = div(inputsElem[0][2]-1+MutaPadElem[2]+MutaPadElem[3]-(inputsElem[1][2]-1)*MutaDilationElem[1],MutaStrideElem[1]).quot+1 ;
            auto OW = div(inputsElem[0][3]-1+MutaPadElem[4]+MutaPadElem[5]-(inputsElem[1][3]-1)*MutaDilationElem[2],MutaStrideElem[2]).quot+1 ;
            auto N = inputsElem[0][0];
            auto OC = inputsElem[2][0];

            resultElem.push_back(N);
            resultElem.push_back(OD);
            resultElem.push_back(OH);
            resultElem.push_back(OW);
            resultElem.push_back(OC);

            //3.3.创建变异后的Op
            op.setAttr("dilation",b.getI64ArrayAttr(MutaDilationElem));
            op.setAttr("pad",b.getI64ArrayAttr(MutaPadElem));
            op.setAttr("stride",b.getI64ArrayAttr(MutaStrideElem));
            op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));
          }
          else if (opName == "tosa.transpose_conv2d") {
            llvm::SmallVector<int64_t> outPadElem = extractFromI64ArrayAttr(op.getAttr("out_pad"));
            llvm::SmallVector<int64_t> outShapeElem = extractFromI64ArrayAttr(op.getAttr("out_shape"));
            llvm::SmallVector<int64_t> strideElem = extractFromI64ArrayAttr(op.getAttr("stride"));

            llvm::SmallVector<int64_t> MutaOutPadElem;
            llvm::SmallVector<int64_t> MutaStrideElem;
            //llvm::SmallVector<int64_t> MutaOutShapeElem;

            MutaOutPadElem = ChangeAttrElem(outPadElem,"out_pad");
            MutaStrideElem = ChangeAttrElem(strideElem,"stride");
            //MutaOutShapeElem = ChangeAttrElem(outShapeElem,"out_shape");

            // OH = (IH - 1) * stride_y + out_pad_top + out_pad_bottom + KH);
            // OW = (IW - 1) * stride_x + out_pad_left + out_pad_right + KW);
            outShapeElem[0] = inputsElem[0][0];  //N
            outShapeElem[1] = (inputsElem[0][1]-1)*MutaStrideElem[0]+MutaOutPadElem[0]+MutaOutPadElem[1]+inputsElem[1][1];  //OH
            outShapeElem[2] = (inputsElem[0][2]-1)*MutaStrideElem[1]+MutaOutPadElem[2]+MutaOutPadElem[3]+inputsElem[1][2];  //OW
            outShapeElem[3] = inputsElem[2][0];  //OC

            for (int i = 0; i < outShapeElem.size(); i++) {
              resultElem.push_back(outShapeElem[i]);
            }

            op.setAttr("out_shape",b.getI64ArrayAttr(outShapeElem));
            op.setAttr("out_pad",b.getI64ArrayAttr(MutaOutPadElem));
            op.setAttr("stride",b.getI64ArrayAttr(MutaStrideElem));
            op.getResult(0).setType(RankedTensorType::get(resultElem,resultETy));
            //LastOpResult.setType(RankedTensorType::get(resultElem,resultETy));



          }

          else if (opName == "tosa.resize") {}
          else if (opName == "tosa.rescale") {}


        }


        //LastOpResult.setType(RankedTensorType::get(resultElem,resultETy));

      }
    }
  };




  //  void getDependentDialects(DialectRegistry & registry) const override {
  //    registry.insert<arith::ArithmeticDialect, scf::SCFDialect,
  //                    memref::MemRefDialect>();
  //  }
  StringRef getArgument() const final { return "AttrGenPass"; }
  StringRef getDescription() const final { return "Attr pass"; }

};
}  // namespace
namespace mlir {
namespace test {
void registerAttrGenPass() { PassRegistration<AttrGenPass>(); }
} // namespace test
} // namespace mlir