//
// Created by Administrator on 2023/3/4.
//
#include "mlir/TosaGen/transfer.h"

#include "mlir/TosaGen/opinfo.h"
#include "mlir/TosaGen/utils.h"

extern Utils genUtils;
extern InfoGen infogen;
extern opInfo info;
extern Transfer transfer;

pair<string, SmallVector<int64_t, 8>> Transfer::transferResult(ImplicitLocOpBuilder b) {
  auto opConstraint = info.Constraint;
  Json::Value result = opConstraint["RESULT"];

  string elementType;
  string typeStr = result[0]["TYPE"].asString();
  if(typeStr=="any" || typeStr=="float" || typeStr=="int" || typeStr=="floatOruint")
      elementType = genUtils.getTensorType(info.inputType[0]);   //result的shape类型和input1的类型相同，不需要更改
  else
      elementType = genUtils.getTypestr(typeStr);


  SmallVector<int64_t, 8> resultShape ;
  if(info.needBroadcast==1) {
    resultShape = genUtils.genBroadcastShape();
  }else{
    resultShape = genUtils.getTensorShape(info.inputType[0]);
  }

  //转换类型
  if(info.opName=="slice") {
    SmallVector<int64_t, 8> newShape;
    auto array =
        info.attrs[1].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    //    auto x =  b.getDenseI64ArrayAttr(newShape);
    for (auto a : array) {
      newShape.push_back(a);
    }
    resultShape = newShape;
  }


  //转换类型
  if(info.opName=="cast") {
    llvm::ArrayRef<string> types= {"i1", "i8", "i16", "i32", "i64", "f32"};
    //找到不一样的类型
    SmallVector<string,8> candidates;
    for(auto type:types) {
      if(type!=elementType)
        candidates.push_back(type);
    }
    elementType = types[genUtils.genRandomN(0, candidates.size() - 1)];
  }

  //转换shape
  if (info.opName == "reshape") {
    SmallVector<int64_t,8> newShape;
    auto array = info.attrs[0].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
//    auto x =  b.getDenseI64ArrayAttr(newShape);
    for(auto a:array){
      newShape.push_back(a);
    }

//    auto y = newShapeAttr.getRawData().data();
//
//    for(auto x : newShapeAttr.getRawData())
//    {
//      newShape.push_back(x);
//    }
//
//    info.attrs[0].getValue().cast<ArrayAttr>();
//    for(auto shapeAttr : newShapeAttr){
//      auto v = shapeAttr.cast<IntegerAttr>().getValue().getSExtValue();
//      newShape.push_back(v);
//    }
    resultShape = newShape;
  }

  //根据axis确定result
  if (info.opName == "reduce_all") {
    auto axis =
        info.attrs[0].getValue().cast<IntegerAttr>().getValue().getSExtValue();
    resultShape = genUtils.getTensorShape(info.inputType[0]);
    resultShape[axis] = 1;
  }

  if (info.opName == "reduce_any") {
    auto axis =
        info.attrs[0].getValue().cast<IntegerAttr>().getValue().getSExtValue();
    resultShape = genUtils.getTensorShape(info.inputType[0]);
    resultShape[axis] = 1;
  }
  if (info.opName == "reduce_max") {
    auto axis =
        info.attrs[0].getValue().cast<IntegerAttr>().getValue().getSExtValue();
    resultShape = genUtils.getTensorShape(info.inputType[0]);
    resultShape[axis] = 1;
  }
  if (info.opName == "reduce_min") {
    auto axis =
        info.attrs[0].getValue().cast<IntegerAttr>().getValue().getSExtValue();
    resultShape = genUtils.getTensorShape(info.inputType[0]);
    resultShape[axis] = 1;
  }
  if (info.opName == "reduce_prod") {
    auto axis =
        info.attrs[0].getValue().cast<IntegerAttr>().getValue().getSExtValue();
    resultShape = genUtils.getTensorShape(info.inputType[0]);
    resultShape[axis] = 1;
  }

  if (info.opName == "reduce_sum") {
    auto axis =
        info.attrs[0].getValue().cast<IntegerAttr>().getValue().getSExtValue();
    resultShape = genUtils.getTensorShape(info.inputType[0]);
    resultShape[axis] = 1;
  }

  if (info.opName == "concat") {
    auto axis =
        info.attrs[0].getValue().cast<IntegerAttr>().getValue().getSExtValue();
    int Dim = info.inputType[0].cast<ShapedType>().getRank();
    if (axis >= Dim) { // 更新属性值
      axis = genUtils.genRandomN(0, Dim - 1);
      info.attrs[0].setValue(b.getI64IntegerAttr(axis));
    }
    SmallVector<int64_t, 8> shape1 =
        genUtils.getTensorShape(info.inputType[0]);
    SmallVector<int64_t, 8> shape2 =
        genUtils.getTensorShape(info.inputType[1]);
    resultShape[axis] = shape1[axis] + shape2[axis];
  }

  if (info.opName == "argmax") {
    auto axis =
        info.attrs[0].getValue().cast<IntegerAttr>().getValue().getSExtValue();
    SmallVector<int64_t, 8> shape =
        genUtils.getTensorShape(info.inputType[0]);
    auto SI = shape.begin();
    for (int j = 0; j < shape.size(); j++) {
      if (j == axis)
        break;
      else
        SI++;
    }
    shape.erase(SI);
    resultShape = shape;
  }

  if (info.opName == "conv2d"){
    // OH == idiv_check(IH - 1 + pad_top + pad_bottom - (KH - 1) * dilation_y, stride_y) + 1
    // OW == idiv_check(IW - 1 + pad_left + pad_right - (KW - 1) * dilation_x, stride_x) + 1

    SmallVector<int64_t,8> input = genUtils.getTensorShape(info.inputType[0]) ;
    SmallVector<int64_t,8> weight = genUtils.getTensorShape(info.inputType[1]) ;
    SmallVector<int64_t,8> bias = genUtils.getTensorShape(info.inputType[2]) ;

    auto padElem = info.attrs[0].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto strideElem = info.attrs[1].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto dilationElem = info.attrs[2].getValue().cast<DenseI64ArrayAttr>().asArrayRef();

    SmallVector<int64_t, 8> shape;
    shape.push_back(input[0]);
    shape.push_back(div(input[1]-1+padElem[0]+padElem[1]-(weight[1]-1)*dilationElem[0],strideElem[0]).quot+1);
    shape.push_back(div(input[2]-1+padElem[2]+padElem[3]-(weight[2]-1)*dilationElem[1],strideElem[1]).quot+1);
    shape.push_back(bias[0]);
    resultShape = shape;
  }

  if (info.opName=="conv3d"){
    //OD = idiv_check(ID - 1 + pad_d0 + pad_d1      - (KD - 1) * dilation_d, stride_d) + 1
    //OH = idiv_check(IH - 1 + pad_top + pad_bottom - (KH - 1) * dilation_y, stride_y) + 1
    //OW = idiv_check(IW - 1 + pad_left + pad_right - (KW - 1) * dilation_x, stride_x) + 1
    SmallVector<int64_t,8> input = genUtils.getTensorShape(info.inputType[0]) ;
    SmallVector<int64_t,8> weight = genUtils.getTensorShape(info.inputType[1]) ;
    SmallVector<int64_t,8> bias = genUtils.getTensorShape(info.inputType[2]) ;

    auto padElem = info.attrs[0].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto strideElem = info.attrs[1].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto dilationElem = info.attrs[2].getValue().cast<DenseI64ArrayAttr>().asArrayRef();

    SmallVector<int64_t, 8> shape;
    shape.push_back(input[0]);
    shape.push_back(div(input[1]-1+padElem[0]+padElem[1]-(weight[1]-1)*dilationElem[0],strideElem[0]).quot+1);
    shape.push_back(div(input[2]-1+padElem[2]+padElem[3]-(weight[2]-1)*dilationElem[1],strideElem[1]).quot+1);
    shape.push_back(div(input[3]-1+padElem[4]+padElem[5]-(weight[3]-1)*dilationElem[2],strideElem[2]).quot+1);
    shape.push_back(bias[0]);
    resultShape = shape;
  }

  else if (info.opName=="transpose_conv2d"){
    // OH = (IH - 1) * stride_y + out_pad_top + out_pad_bottom + KH);
    // OW = (IW - 1) * stride_x + out_pad_left + out_pad_right + KW);
    SmallVector<int64_t,8> input = genUtils.getTensorShape(info.inputType[0]) ;
    SmallVector<int64_t,8> weight = genUtils.getTensorShape(info.inputType[1]) ;
    SmallVector<int64_t,8> bias = genUtils.getTensorShape(info.inputType[2]) ;


    auto out_pad = info.attrs[0].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto strideElem = info.attrs[1].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto dilationElem = info.attrs[2].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto out_shape = info.attrs[3].getValue().cast<DenseI64ArrayAttr>().asArrayRef();

    SmallVector<int64_t, 8> shape;
    shape.push_back(input[0]);
    shape.push_back((input[1]-1)*strideElem[0]+out_pad[0]+out_pad[1]+weight[1]);
    shape.push_back((input[2]-1)*strideElem[1]+out_pad[2]+out_pad[3]+weight[2]);
    shape.push_back(bias[0]);
    resultShape = shape;

  }

  if (info.opName=="depthwise_conv2d"){
    SmallVector<int64_t,8> input = genUtils.getTensorShape(info.inputType[0]) ;
    SmallVector<int64_t,8> weight = genUtils.getTensorShape(info.inputType[1]) ;
    SmallVector<int64_t,8> bias = genUtils.getTensorShape(info.inputType[2]) ;
    // OH = idiv_check(IH - 1 + pad_top + pad_bottom - (KH - 1) * dilation_y, stride_y) + 1
    // OW = idiv_check(IW - 1 + pad_left + pad_right - (KW - 1) * dilation_x, stride_x) + 1
    auto padElem = info.attrs[0].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto strideElem = info.attrs[1].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto dilationElem = info.attrs[2].getValue().cast<DenseI64ArrayAttr>().asArrayRef();

    SmallVector<int64_t, 8> shape;
    shape.push_back(input[0]);
    shape.push_back(div(input[1]-1+padElem[0]+padElem[1]-(weight[0]-1)*dilationElem[0],strideElem[0]).quot+1);
    shape.push_back(div(input[2]-1+padElem[2]+padElem[3]-(weight[1]-1)*dilationElem[1],strideElem[1]).quot+1);
    shape.push_back(bias[0]);
    resultShape = shape;
  }
  if (info.opName=="max_pool2d" || info.opName=="avg_pool2d"){
    SmallVector<int64_t,8> input = genUtils.getTensorShape(info.inputType[0]) ;
// OH = idiv_check(IH + pad_top + pad_bottom - kernel_y, stride_y) + 1
// OW = idiv_check(IW + pad_left + pad_right - kernel_x, stride_x) + 1
    auto kernelElem = info.attrs[0].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto strideElem = info.attrs[1].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto padElem = info.attrs[2].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    SmallVector<int64_t, 8> shape;
    shape.push_back(input[0]);
    shape.push_back(div(input[1]+padElem[0]+padElem[1]-kernelElem[0],strideElem[0]).quot+1);
    shape.push_back(div(input[2]+padElem[2]+padElem[3]-kernelElem[1],strideElem[1]).quot+1);
    shape.push_back(input[3]);
    resultShape = shape;
  }
  else if (info.opName=="slice"){}
  else if (info.opName=="clamp"){}
  else if (info.opName=="resize"){
    SmallVector<int64_t, 8> inputShape = genUtils.getTensorShape(info.inputType[0]);
    SmallVector<int64_t, 8> outputShape;
    outputShape.push_back(inputShape[0]) ; //output[0]
    int64_t inputHeight = inputShape[1];
    int64_t inputWidth = inputShape[2];
    auto scaleInt = info.attrs[0].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto offsetInt = info.attrs[1].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    auto borderInt = info.attrs[2].getValue().cast<DenseI64ArrayAttr>().asArrayRef();
    outputShape.push_back((((inputHeight - 1) * scaleInt[0] - offsetInt[0] + borderInt[0]) / scaleInt[1]) + 1); //output[1]
    outputShape.push_back((((inputWidth - 1) * scaleInt[2] - offsetInt[1] + borderInt[1]) / scaleInt[3]) + 1);  //output[2]
    outputShape.push_back(inputShape[3]) ; //output[3]
    resultShape = outputShape;
  }
  else if (info.opName=="rescale"){}

  return {elementType,resultShape};
}

pair<string, SmallVector<SmallVector<int64_t, 8>, 8>> Transfer::transferResult(ImplicitLocOpBuilder b, string opName) {
  SmallVector<SmallVector<int64_t, 8>, 8> resultShape ;
  string elementType = genUtils.getTensorType(info.inputType[0]);
  if (opName == "rfft2d"){
    SmallVector<int64_t, 8> inputShape = genUtils.getTensorShape(info.inputType[0]);
    SmallVector<int64_t, 8> outputShape;
    outputShape[0] = inputShape[0];
    outputShape[1] = inputShape[1];
    int64_t inWidth = inputShape[2];
    outputShape[2] = inWidth / 2 + 1;
    resultShape.push_back(outputShape);
    resultShape.push_back(outputShape);
  }
  return {elementType,resultShape};
}