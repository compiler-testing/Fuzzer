//
// Created by Administrator on 2023/3/4.
//
#include "mlir/TosaGen/create.h"
#include "mlir/TosaGen/opinfo.h"
#include "mlir/TosaGen/utils.h"

extern Utils genUtils;
extern InfoGen infogen;
extern opInfo info;
extern Create create;


Value Create::createNewBranch(ImplicitLocOpBuilder b, Location loc, func::FuncOp funcOp,string selectedOp){
        infogen.initInfo(selectedOp);
        infogen.addInputType(b);
        // if(info.inputType.size()==0)
        //   continue;
        // else
        create.insertFuncArg(b,funcOp,info.inputType); //插入index
        infogen.addInputs(b,loc,funcOp);
        infogen.addAttrs(b,loc);
        infogen.addResult(b);
        Value newOp = create.createOp(b,loc);
        return newOp;
}

void Create::insertFuncArg(ImplicitLocOpBuilder b, func::FuncOp funcOp, SmallVector<Type, 8> argTypes){
  //插入位置
  int startIndex = funcOp.getNumArguments();
//  cout<<"insertFuncArg"<<endl;
  //将input插入funcOp的参数中
  int begin = startIndex;
  int end = startIndex + argTypes.size();
  for (int i = begin;i < end;i++) {
    funcOp.insertArgument(i,argTypes[i-begin],
                          {},UnknownLoc::get(funcOp.getContext()));
//      funcOp.insertArgument(i,argTypes[i-begin],DictionaryAttr::get(funcOp.getContext()),UnknownLoc::get(funcOp.getContext()));
  }
}

template <typename OpTy>
Value createOpImplement(ImplicitLocOpBuilder b, Location loc) {
  Value newOp;
  if(info.attsNum==0)
    newOp  =b.create<OpTy>(loc,info.resultType,info.inputs);
  else if(info.attsNum>=1)
    newOp  =b.create<OpTy>(loc,info.resultType,info.inputs,info.attrs);
  return newOp;
}

//根据属性个数创建op
Value Create::createOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc) {
  Value newOp;
  if (info.attsNum == 0)
    newOp = createOpWithNoAttr(b, loc);
  else if (info.attsNum == 1)
    newOp = createOpWithOneAttr(b, loc);
  else if (info.attsNum>1)
    newOp = createOpWithMulAttrs(b, loc);
  newOp.dump();
  return newOp;
}

Value Create::createOpWithNoAttr(ImplicitLocOpBuilder b, Location loc) {
  Value newop;
  string selectedOp = info.opName;
  if(selectedOp=="abs")
    newop = createOpImplement<tosa::AbsOp>(b,loc);
  else if(selectedOp=="add")
    newop = createOpImplement<tosa::AddOp>(b,loc);
  else if(selectedOp=="bitwise_and")
    newop = createOpImplement<tosa::BitwiseAndOp>(b,loc);
  else if(selectedOp=="bitwise_not")
    newop = createOpImplement<tosa::BitwiseNotOp>(b,loc);
  else if(selectedOp=="bitwise_or")
    newop = createOpImplement<tosa::BitwiseOrOp>(b,loc);
  else if(selectedOp=="bitwise_xor")
    newop = createOpImplement<tosa::BitwiseXorOp>(b,loc);
  else if(selectedOp=="cast")
    newop = createOpImplement<tosa::CastOp>(b,loc);
  else if(selectedOp=="ceil")
    newop = createOpImplement<tosa::CeilOp>(b,loc);
  else if(selectedOp=="clz")
    newop = createOpImplement<tosa::ClzOp>(b,loc);
    /*else if(selectedOp=="cond_if")  //cond_if----if
      newop = createOpImplement<tosa::IfOp>(b,loc,selectedOp,inputs,{});*/
  else if(selectedOp=="div")
    newop = createOpImplement<tosa::DivOp>(b,loc);
  else if(selectedOp=="equal")
    newop = createOpImplement<tosa::EqualOp>(b,loc);
  if(selectedOp=="exp")
    newop = createOpImplement<tosa::ExpOp>(b,loc);
  else if(selectedOp=="floor")
    newop = createOpImplement<tosa::FloorOp>(b,loc);
  else if(selectedOp=="gather")
    newop = createOpImplement<tosa::GatherOp>(b,loc);
  else if(selectedOp=="greater")
    newop = createOpImplement<tosa::GreaterOp>(b,loc);
  else if(selectedOp=="greater_equal")
    newop = createOpImplement<tosa::GreaterEqualOp>(b,loc);
  else if(selectedOp=="identity")
    newop = createOpImplement<tosa::IdentityOp>(b,loc);
  else if(selectedOp=="log")
    newop = createOpImplement<tosa::LogOp>(b,loc);
  else if(selectedOp=="logical_and")
    newop = createOpImplement<tosa::LogicalAndOp>(b,loc);
  if(selectedOp=="logical_left_shift")
    newop = createOpImplement<tosa::LogicalLeftShiftOp>(b,loc);
  else if(selectedOp=="logical_not")
    newop = createOpImplement<tosa::LogicalNotOp>(b,loc);
  else if(selectedOp=="logical_or")
    newop = createOpImplement<tosa::LogicalOrOp>(b,loc);
  else if(selectedOp=="logical_right_shift")
    newop = createOpImplement<tosa::LogicalRightShiftOp>(b,loc);
  else if(selectedOp=="logical_xor")
    newop = createOpImplement<tosa::LogicalXorOp>(b,loc);
  else if(selectedOp=="maximum")
    newop = createOpImplement<tosa::MaximumOp>(b,loc);
  else if(selectedOp=="minimum")
    newop = createOpImplement<tosa::MinimumOp>(b,loc);
  else if(selectedOp=="pow")
    newop = createOpImplement<tosa::PowOp>(b,loc);
  else if(selectedOp=="reciprocal")
    newop = createOpImplement<tosa::ReciprocalOp>(b,loc);
  else if(selectedOp=="rsqrt")
    newop = createOpImplement<tosa::RsqrtOp>(b,loc);
  else if(selectedOp=="scatter")
    newop = createOpImplement<tosa::ScatterOp>(b,loc);
  else if(selectedOp=="select")
    newop = createOpImplement<tosa::SelectOp>(b,loc);
  else if(selectedOp=="sigmoid")
    newop = createOpImplement<tosa::SigmoidOp>(b,loc);
  else if(selectedOp=="sub")
    newop = createOpImplement<tosa::SubOp>(b,loc);
  else if(selectedOp=="table")
    newop = createOpImplement<tosa::TableOp>(b,loc);
  else if(selectedOp=="tanh")
    newop = createOpImplement<tosa::TanhOp>(b,loc);
  else if(selectedOp=="transpose")
    newop = createOpImplement<tosa::TransposeOp>(b,loc);
  /*else if(selectedOp=="while_loop")
    newop = createOpImplement<tosa::WhileOp>(b,loc,selectedOp,inputs,{});*/
  return newop;
}

Value Create::createOpWithOneAttr(ImplicitLocOpBuilder b, Location loc){
  Value newop;
  string selectedOp = info.opName;
  if(selectedOp=="reduce_max")
    newop = createOpImplement<tosa::ReduceMaxOp>(b,loc);
  else if(selectedOp=="reduce_any")
    newop = createOpImplement<tosa::ReduceAnyOp>(b,loc);
  else if(selectedOp == "reduce_all")
    newop = createOpImplement<tosa::ReduceAllOp>(b,loc);
  else if(selectedOp == "reduce_prod")
    newop = createOpImplement<tosa::ReduceProdOp>(b,loc);
  else if(selectedOp == "reduce_min")
    newop = createOpImplement<tosa::ReduceMinOp>(b,loc);
  else if(selectedOp == "reduce_sum")
    newop = createOpImplement<tosa::ReduceSumOp>(b,loc);
  else if(selectedOp == "argmax")
    newop = createOpImplement<tosa::ArgMaxOp>(b,loc);
  else if(selectedOp == "reverse")
    newop = createOpImplement<tosa::ReverseOp>(b,loc);
  else if(selectedOp == "concat")
    newop = createOpImplement<tosa::ConcatOp>(b,loc);
  else if(selectedOp == "reshape")
    newop = createOpImplement<tosa::ReshapeOp>(b,loc);
  return newop;
}

Value Create::createOpWithMulAttrs(ImplicitLocOpBuilder b, Location loc){
  Value newop;
  string selectedOp = info.opName;
  if(selectedOp=="conv2d")
    newop = createOpImplement<tosa::Conv2DOp>(b,loc);
  else if(selectedOp=="conv3d")
    newop = createOpImplement<tosa::Conv3DOp>(b,loc);
  else if(selectedOp=="transpose_conv2d")
    newop = createOpImplement<tosa::TransposeConv2DOp>(b,loc);
  else if(selectedOp=="depthwise_conv2d")
    newop = createOpImplement<tosa::DepthwiseConv2DOp>(b,loc);
  else if(selectedOp=="avg_pool2d")
    newop = createOpImplement<tosa::AvgPool2dOp>(b,loc);
  else if(selectedOp=="max_pool2d")
    newop = createOpImplement<tosa::MaxPool2dOp>(b,loc);
  else if(selectedOp=="slice")
    newop = createOpImplement<tosa::SliceOp>(b,loc);
  else if(selectedOp=="clamp")
    newop = createOpImplement<tosa::ClampOp>(b,loc);
  else if(selectedOp=="resize")
    newop = createOpImplement<tosa::ResizeOp>(b,loc);
  else if(selectedOp=="rescale")
    newop = createOpImplement<tosa::RescaleOp>(b,loc);
  return newop;
}
