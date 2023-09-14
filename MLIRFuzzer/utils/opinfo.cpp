//
// Created by Administrator on 2023/3/4.
//
#include "TosaGen/opinfo.h"
#include "TosaGen/utils.h"
#include "TosaGen/transfer.h"
#include "TosaGen/create.h"

extern Utils genUtils;
extern InfoGen infogen;
extern opInfo info;
extern Transfer transfer;
extern Create create;

opInfo initStruct(opInfo info){
  info.opName = "";
  info.inputNum = 0;
  info.attsNum = 0;
  info.needBroadcast  = 0;

  info.inputType= {};
  info.attrs = {};
  info.resultType = nullptr;
  info.inputs = {};
  info.Constraint = {};
  return info;
}


void InfoGen::initInfo(string opname){
  /*根据opnamde初始化opinfo*/
//  cout<<"initializing"<<endl;
  auto opConstraint = genUtils.getConstraint(opname);
  info = initStruct(info);
  info.opName = opConstraint["OP_NAME"].asString();
  info.Constraint = opConstraint;
  SmallVector<StringRef , 8> opConNames ={"select","concat","add","bitwise_and","bitwise_or","bitwise_xor","div","equal","greater","greater_equal","logical_and","logical_left_shift","logical_or","logical_right_shift","logical_xor","maximum","minimum","pow","sub","arithmetic_right_shift","mul","select"}; //,"cond_if"
  auto isConOp = std::find(opConNames.begin(), opConNames.end(), info.opName);
  if(isConOp != opConNames.end())
    info.needBroadcast=1;
  else
    info.needBroadcast=0;

  info.attsNum = opConstraint["ATTR_NUM"].asInt();
  info.inputNum = opConstraint["INPUT_NUM"].asInt();

  if(info.opName=="select")
    info.inputNum = 2;
}

// std::vector<int> randperm(int Num)
// {
// 	std::vector<int> temp;
// 	for (int i = 0; i < Num; ++i)
// 	{
// 		temp.push_back(i);
// 	}
// 	random_shuffle(temp.begin(), temp.end());
//   return temp;
// }

SmallVector<SmallVector<int64_t,8>,8> genInputShape() {
//  Json::Value inputs = info.Constraint["INPUT"];
  int inputNum = info.inputNum;
  int Dim;
  llvm::SmallVector<int64_t, 8> shape1;
  llvm::SmallVector<int64_t, 8> shape2;
  SmallVector<SmallVector<int64_t, 8>, 8> inputShapes;

  SmallVector<SmallVector<int64_t,8>,8> curOpDim = genUtils.getDimFromopConstraint();

  SmallVector<int64_t,8> cShape; //ConstraintShape
  for (int i = 0; i < curOpDim.size();
       i++) { // 循环遍历“INPUT”中的多个对象，获取每个对象中的维度大小
    cShape = curOpDim[i];

    if (cShape.size() > 1) {
      int rDim = genUtils.genRandomN(0, cShape.size() - 1);
      Dim = cShape[rDim];
    } else if (cShape.size() == 1) {
      if (cShape[0] == 100)
        Dim = genUtils.genRandomN(0, 6);
      else
        Dim = cShape[0];
    }
    //    cout << "Dim : " << Dim << endl;
    if (i == 1 && info.needBroadcast == 1) {
      shape2 = genUtils.genConstrainedShape(shape1);
      inputShapes.push_back(shape2);
    } else {
      if(i==0 && !info.inputType.empty()) {
        shape1 = genUtils.getTensorShape(info.inputType[0]);
      }else{
        shape1 = genUtils.genShape(i,Dim,inputShapes); // 生成形状
      }
      inputShapes.push_back(shape1);
    }
  }

  // if(info.needBroadcast == 1){
  //   int v = genUtils.genRandomN(0,inputShapes[0].size()-1);
  //   std::vector<int> vec = randperm(inputShapes[0].size());

  //   for(int i=0;i<inputShapes.size();i++){
  //     for(int j =0;j<v;j++){
  //       inputShapes[i][vec[j]]=mlir::ShapedType::kDynamic;
  //     }
  //   }
  // }
  return inputShapes;
}

SmallVector<string,8> genInputType(){
  Json::Value inputs = info.Constraint["INPUT"];
  int inputNum = info.inputNum;
  string typeStr;
  string elementType;
  SmallVector<string, 8> ElementTypes;
  for (int i = 0;i < inputNum; i++) {
    if(i==0 && !info.inputType.empty()){
      elementType = genUtils.getTensorType(info.inputType[0]);
      ElementTypes.push_back(elementType);
    }else{
      typeStr = inputs[i]["TYPE"].asString();
      if (info.opName == "conv2d" || info.opName == "conv3d" ||
          info.opName == "transpose_conv2d" ||
          info.opName == "depthwise_conv2d") {
        typeStr = "conv";
      }
      if (info.opName == "avg_pool2d" || info.opName == "max_pool2d") {
        typeStr = "anyNoI1";
      }
      // 操作数元素相同的情况
      string firstType;
      if (typeStr == "any" || typeStr == "float" || typeStr == "int" ||
          typeStr == "floatOruint" || typeStr == "anyNoI1" || typeStr == "conv") {
        if (i == 0)
          elementType = genUtils.getTypestr(typeStr);
        ElementTypes.push_back(elementType);
      } else {
        elementType = genUtils.getTypestr(typeStr);
        ElementTypes.push_back(elementType);
      }
    }
  }
  return ElementTypes;
}

void InfoGen::addInputType(ImplicitLocOpBuilder b){
//  cout<<"addInputType     "<<info.inputNum<<endl;

  SmallVector<SmallVector<int64_t,8>,8> inputShapes = genInputShape();
  SmallVector<string,8> elementTypes = genInputType();

  Type tensorType;
  for(int i = 0;i<inputShapes.size();i++) {
    tensorType = genUtils.genTensorType(b, inputShapes[i], elementTypes[i]);
    info.inputType.push_back(tensorType);
  }
//  genUtils.printTypes(info.inputType);
}

void InfoGen::addInputType(ImplicitLocOpBuilder b,Value preOp){
//  cout << "addInputType" << endl;
  info.inputType.push_back(preOp.getType());
  if(info.inputNum>1){
    Json::Value inputs = info.Constraint["INPUT"];
    SmallVector<SmallVector<int64_t, 8>, 8> inputShapes = genInputShape();
    SmallVector<string,8> elementTypes = genInputType();

    Type tensorType;
    for(int i = 1;i<inputShapes.size();i++) {
      tensorType = genUtils.genTensorType(b, inputShapes[i], elementTypes[i]);
      info.inputType.push_back(tensorType);
    }
  }
//  genUtils.printTypes(info.inputType);
}

extern int concat_axis;
void InfoGen::addAttrs(ImplicitLocOpBuilder b, Location loc){
//  cout<<"addAttrs"<<endl;
  int attsNum = info.attsNum;
  auto opConstraint = info.Constraint;
  Json::Value attrs = opConstraint["ATTR"];
  SmallVector<NamedAttribute> namedAttrs;

  if(attsNum==1) {
    if (attrs[0]["NAME"].asString() == "axis") {
      int Dim = info.inputType[0].cast<ShapedType>().getRank();
      int axis = genUtils.genRandomN(0, Dim - 1);

      if (info.opName == "concat"){
//        int axis=0;
        SmallVector<int> index;
        if(info.inputType.size()==2){
          auto i1= info.inputType[0].cast<ShapedType>().getShape();
          auto i2= info.inputType[1].cast<ShapedType>().getShape();
          if(i1.size()==i2.size()){
            for(int i = 0;i<i2.size();i++){
              if(i1[i]!=i2[i]){
                index.push_back(i);
              }
            }
          }
        }
        if(!index.empty()){
        if(index.size()==1)
          axis=index[0];
        else
          axis = index[genUtils.genRandomN(0,index.size()-1)];
        }
      }
//      cout<<"axis  : "<< axis<<endl;
      namedAttrs.push_back(b.getNamedAttr("axis", b.getI64IntegerAttr(axis)));
    } else if (attrs[0]["NAME"].asString() == "new_shape") {
      SmallVector<int64_t, 8> newShape;
      int en = genUtils.getElementNum(
          info.inputType[0].cast<ShapedType>().getShape());
      // 判断是不是素数
      int prime = 0;
      for (int i = 2; i < en; i++) {
        if (en % i == 0)
          prime++;
      }
      // 如果元素个数是素数，包含1
      if (prime == 0) {
        newShape.push_back(en);
        int size = genUtils.genRandomN(1, 3);
        while (size > 0) {
          newShape.push_back(1);
          size--;
        }
      } else {
        int size = genUtils.genRandomN(1, 4);

        SmallVector<int64_t, 8> factors = genUtils.getPrimeFactors(en);
        random_shuffle(factors.begin(), factors.end());
        if (size == 1) {
          newShape.push_back(en);
        } else {
          int temp_size = 1;
          int product = 1;

          while (temp_size <= size) {
            if (factors.size() == 0) {
              newShape.push_back(1);
            } else if (temp_size == size) {
              product = 1;
              while (factors.size() > 0) {
                product = product * factors.back();
                factors.pop_back();
              }
              newShape.push_back(product);
            } else {
              int slice = genUtils.genRandomN(0, factors.size() - 1);
              SmallVector<int64_t, 8> slice_vec;
              product = 1;
              for (; slice > 0; slice--) {
                product = product * factors.back();
                factors.pop_back();
              }
              newShape.push_back(product);
            }
            temp_size++;
          }
        }
      }
      random_shuffle(newShape.begin(), newShape.end());
      namedAttrs.push_back(b.getNamedAttr("new_shape",b.getDenseI64ArrayAttr(newShape)));
//      namedAttrs.push_back(
//          b.getNamedAttr("new_shape", b.getI64ArrayAttr(newShape)));
    }
  }else{
    int attrArrayCount;
    string  attrName;

    //std::uniform_int_distribution<unsigned> randomBig(0, 126549185);
    //  std::uniform_int_distribution<unsigned> randomSmall(1, 2);
    if (info.opName=="conv2d" || info.opName=="conv3d" || info.opName=="depthwise_conv2d"
        || info.opName=="max_pool2d" || info.opName=="avg_pool2d" || info.opName=="transpose_conv2d"){
      for (int i = 0; i < attsNum; i++) {
        SmallVector<int64_t , 8> newShape ;
        attrName = attrs[i]["NAME"].asString();
        attrArrayCount = attrs[i]["ArrayCount"][0].asInt();
        if (attrName == "quantization_info"){  //对于属性 "quantization_info"，先暂时不处理
          break;
        }
        if (attrName == "out_shape"){
          attrArrayCount = 4;
        }
        for (int j = 0; j < attrArrayCount; j++) {
          unsigned int r =  genUtils.genRandomN(1,2);
          newShape.push_back(r);

        }
        namedAttrs.push_back(b.getNamedAttr(attrName,b.getDenseI64ArrayAttr(newShape)));
      }
    }

    else if (info.opName=="slice"){

    }
    else if (info.opName=="reluN"){}
    else if (info.opName=="clamp"){}
    else if (info.opName=="resize"){}
    else if (info.opName=="rescale"){}
  }
  info.attrs = namedAttrs;
}


void InfoGen::addInputs(ImplicitLocOpBuilder b, Location loc,func::FuncOp funcOp) {
//  cout<<"addInputs"<<endl;
  auto opConstraint = info.Constraint;
  SmallVector<Value, 8> argsValue;
  int startindex =  funcOp.getNumArguments()-info.inputNum;
  for (int j = startindex; j < funcOp.getNumArguments(); j++) {
    argsValue.push_back(funcOp.getArgument(j));
  }
  info.inputs =argsValue;
//  genUtils.printValues(info.inputs);
}

void InfoGen::addInputs(ImplicitLocOpBuilder b, Location loc,func::FuncOp funcOp,Value preOp) {
//  cout<<"addInputs"<<endl;
  auto opConstraint = info.Constraint;
  SmallVector<Value, 8> argsValue;
  SmallVector<Type, 8> argTypes;
  info.inputs.push_back(preOp);
  if(info.inputNum>1){//创建输入
    int startIndex = funcOp.getNumArguments();
    argTypes = info.inputType;
    argTypes.erase(argTypes.begin());
    create.insertFuncArg(b,funcOp,argTypes); //插入index

    for (int j = startIndex; j < funcOp.getNumArguments(); j++) {
      info.inputs.push_back(funcOp.getArgument(j));
    }
  }
//  genUtils.printValues(info.inputs);
}


void InfoGen::addResult(mlir::ImplicitLocOpBuilder b) {
  cout<<"addResult:   ";
  pair<string, SmallVector<int64_t, 8>> result =  transfer.transferResult(b);
  Type resultType = genUtils.genTensorType(b, result.second, result.first);
  info.resultType = resultType;
  info.resultType.dump();
}


pair<string, SmallVector<int64_t, 8>> InfoGen::getResultInfo(mlir::Value op) {

  auto type = op.getType().cast<ShapedType>().getElementType();
  auto shape = op.getType().cast<ShapedType>().getShape();
  pair<string, SmallVector<int64_t, 8>> resultInfo = {genUtils.type2str(type), genUtils.getShapeVector(shape)};
  return  resultInfo;
}