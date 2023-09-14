//
// Created by Administrator on 2023/3/4.
//
#include "TosaGen/utils.h"
#include "TosaGen/opinfo.h"
#include <unistd.h>
#include "stdlib.h"
#include <stdio.h>
#include <string.h>
#include <vector>

#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/Support/raw_ostream.h"
#include <numeric>

using namespace std;
extern Utils genUtils;
extern InfoGen infogen;
extern opInfo info;
int concat_axis;

extern DenseMap<int, SmallVector<string,8>>  opsDim;
extern DenseMap<int, SmallVector<string,8>>  opsType;

std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd());
unsigned int Utils::genRandomN(unsigned int begin,unsigned int end)
{
  std::uniform_int_distribution<unsigned> randomNum(begin, end);
  return randomNum(gen);
}


string Utils::getRandomOp(llvm::ArrayRef<StringRef> opPool) {
  // 从op池中随机选择op
  unsigned int r =  genRandomN(0, opPool.size()-1);
  return opPool[r].str();
}

llvm::SmallVector<int64_t, 8> Utils::genShape(int i, int Dim, SmallVector<SmallVector<int64_t, 8>, 8> inputShapes) {
  //根据维度随机生成形状
  Json::Value inputs = info.Constraint["INPUT"];
  SmallVector<int64_t, 8> shape = GenRandomShape(Dim);


    if (inputs[i]["NAME"]=="input" && info.opName=="depthwise_conv2d"){
      shape[3]= genUtils.genRandomN(1, 5);
    }
    //随机创建weight，但是weight的参数和input参数有关联，且KH，KW两个参数不宜取大值
    if (inputs[i]["NAME"]=="weight" ){

      if (info.opName=="conv2d" || info.opName=="transpose_con2d" ){
        shape[3]=inputShapes[0][3];
        shape[1]= genUtils.genRandomN(1, 5);
        shape[2]= genUtils.genRandomN(1, 5);
      }
      else if (info.opName=="conv3d"){
        shape[4]=inputShapes[0][4];
        shape[1]=genUtils.genRandomN(1, 5);
        shape[2]=genUtils.genRandomN(1, 5);
        shape[3]=genUtils.genRandomN(1, 5);
      }
      else if (info.opName=="depthwise_conv2d"){
        shape[2]=inputShapes[0][3];
        shape[0]=genUtils.genRandomN(1, 5);
        shape[1]=genUtils.genRandomN(1, 5);
        shape[3]=genUtils.genRandomN(1, 5);
      }
    }
    else if (inputs[i]["NAME"]=="bias"){
      if (info.opName=="conv2d" || info.opName=="conv3d" || info.opName=="transpose_conv2d"){
        shape[0]=inputShapes[1][0];
      }
      else if (info.opName=="depthwise_conv2d"){
        shape[0]=inputShapes[1][2]*inputShapes[1][3];
      }
    }
  return shape;
}

int Utils::getElementNum(llvm::SmallVector<int64_t , 8> shape){
  int ElementNum = 1;
  auto SI = shape.begin();
  auto SE = shape.end();
  while(SI!=SE){
    ElementNum *= *SI;
    SI++;
  }
  return ElementNum;
}

int Utils::getElementNum(llvm::ArrayRef<int64_t> shape){
  int ElementNum = 1;
  auto SI = shape.begin();
  auto SE = shape.end();
  while(SI!=SE){
    ElementNum *= *SI;
    SI++;
  }
  return ElementNum;
}


Json::Value Utils::getConstraint(StringRef selectedOp) {
  // 从tosaOps.json中获取op创建的约束
  //string filePath = "/home/ty/llvm15/mlir/test/tosa_ops.json";

  char szPath[128];
  memset( szPath, 0x00, sizeof(szPath));
  // int ret =  readlink("/proc/self/exe", szPath, sizeof(szPath)-1 );
  int ret =  readlink("/proc/self/exe", szPath, 256);
  // printf("ret:%d\n", ret);
  // printf("path:%s\n", szPath);

  char *token= NULL;
  token = strtok(szPath, "b");

  string path = token;

  string filePath = path + "utils/tosaOps.json";

  // string filePath = path + "test/lib/TosaGen/tosaOps.json";
  std::ifstream is;
  is.open (filePath, std::ios::binary );

  Json::Reader reader;
  Json::Value value;
  Json::Value opConstraint;
  if (reader.parse(is, value)) // json字符串转为json对象
  {
    opConstraint = value[selectedOp.str()];
  }
  return opConstraint;
}

llvm::SmallVector<int64_t , 8> Utils::genConstrainedShape(SmallVector<int64_t , 8> shape1) {
  SmallVector<SmallVector<int64_t ,8>,8> shapes;
  llvm::SmallVector<int64_t, 8>  allOneShape;
  llvm::SmallVector<int64_t, 8>  candidateShape;
  //  cout<<"=======all shapes======"<<endl;

  if (info.opName=="concat"){
    std::uniform_int_distribution<unsigned> rDim(0,shape1.size()-1);
    std::uniform_int_distribution<unsigned> rDimVal(1,100);
    candidateShape = shape1;
    concat_axis = rDim(gen);
    candidateShape[concat_axis] = rDimVal(gen);
    //testGen.printArray(candidateShape);
    return candidateShape;
  }

  for (int i = 0; i < shape1.size(); i++)
    allOneShape.push_back(1); //[1,1,1,1]

  //  testGen.printArray(allOneShape);
  shapes.push_back(allOneShape);

  for (int i = 0; i < shape1.size(); i++)
  {
    int ans = shapes.size();
    if(shape1[i]==1)
      continue ;
    for (int j = 0; j < ans; j++)
    {
      llvm::SmallVector<int64_t , 8> candidateShape = shapes[j];
      candidateShape[i]=shape1[i];
      //      testGen.printArray(candidateShape);
      shapes.push_back(candidateShape);
    }
  }
  //  cout<<"=======select shapes======"<<endl;
  std::uniform_int_distribution<unsigned> randomRange(0,shapes.size()-1);  //取随机数1代表取"1"，取随机数2代表取shape1中的值
  SmallVector<int64_t ,8> selectShape = shapes[randomRange(gen)];
  //  testGen.printArray(selectShape);
  return selectShape;
}


string normalizeType(string typeStr){
  string elementType;
  if (typeStr=="i1" || typeStr=="I1")
    elementType = "i1";
  else if (typeStr=="I8" || typeStr=="i8" )
    elementType = "i8";
  else if (typeStr=="I16" || typeStr=="i16" )
    elementType = "i16";
  else if (typeStr=="I32" || typeStr=="i32" || typeStr=="Int32" || typeStr=="int32" )
    elementType = "i32";
  else if (typeStr=="I64" || typeStr=="i64" || typeStr=="Int64" || typeStr=="int64" )
    elementType = "i64";
  else if (typeStr=="f32" || typeStr=="F32" || typeStr=="float")
    elementType = "f32";
  else
    elementType = typeStr;
  return elementType;
}

string Utils::type2str(mlir::Type type) {
  string typeStr;
  if (type.isIntOrFloat()){
    if (type.isF32())
      typeStr="f32";
    else if (type.isF64())
      typeStr="f64";
    else if (type.isInteger(1))
      typeStr = "i1";
    else if (type.isInteger(8))
      typeStr = "i8";
    else if (type.isInteger(16))
      typeStr = "i16";
    else if (type.isInteger(32))
      typeStr = "i32";
    else if (type.isInteger(64))
      typeStr = "i64";
  }
  return typeStr;
}

//分解因式
SmallVector<int64_t , 8> Utils::getPrimeFactors(int x) {
  SmallVector<int64_t , 8> primeFactors;
  long p = 2;
  while (x != 1) {
    for (p = 2; p <= x; p++) {
      if (x % p == 0)
        break;
    }
    primeFactors.push_back(p);
    x /= p;
  }
  return primeFactors;
}

string Utils::getTensorType(Type tensor){
  string typeStr = type2str(
      tensor.cast<ShapedType>().getElementType());
  return typeStr;
}


SmallVector<int64_t, 8> Utils::getTensorShape(Type tensor){
  SmallVector<int64_t, 8> shapes;
  auto inputshape = tensor.cast<ShapedType>().getShape();
  for (auto s : inputshape) {
    shapes.push_back(s);
  }
  return shapes;
}



llvm::SmallVector<int64_t, 8> Utils::GenRandomShape(int Dim) {
  llvm::SmallVector<int64_t , 8> shape;
  std::uniform_int_distribution<unsigned> randomLen(1, 100);
  int Len;
  for(int i = 0;i < Dim; i++){
    Len = randomLen(gen);
    shape.push_back(Len);
    //    llvm::errs()<<" "<<Len;
  }
  return shape;
}

Type genElementType(ImplicitLocOpBuilder b,string typeStr){
  Type elementType;
  if (typeStr=="i1")
    elementType = b.getI1Type();
  else if (typeStr=="i8" || typeStr=="ui8" )
    elementType = b.getI8Type();
  else if (typeStr=="i16" || typeStr=="ui16" )
    elementType = b.getIntegerType(16);
  else if (typeStr=="i32" || typeStr=="ui32")
    elementType = b.getI32Type();
  else if (typeStr=="i64" || typeStr=="ui64")
    elementType = b.getI64Type();
  else if (typeStr=="f32")
    elementType = b.getF32Type();
  return elementType;
}

template <typename OpTy>
llvm::SmallVector<OpTy , 8> GenRandomTensor(int ElementNum,string typrStr){
  std::uniform_int_distribution<int> randomValue(-127, 127);
  if(typrStr=="i1")
    std::uniform_int_distribution<int> randomValue(0, 1);
  else if(typrStr=="f32")
    std::uniform_real_distribution<float> randomValue(-128.0, 127.0);
  else if(typrStr=="ui8" || typrStr=="ui32" || typrStr=="ui64")
    std::uniform_int_distribution<unsigned> randomValue(0, 127);
  else if(typrStr=="ui32" || typrStr=="ui64")
    std::uniform_int_distribution<unsigned> randomValue(0, 127);
  llvm::SmallVector<OpTy, 8> inputValue;
  int val;
  //  llvm::errs()<<"Tensor: [";
  for(int j = 0;j < ElementNum; j++){
    //    Float16Type x;
    //    float_t  y = 1.0;
    val = randomValue(gen);
    inputValue.push_back(val);
    //    llvm::errs()<<val<<',';
  }
  //  llvm::errs()<<']'<<'\n';
  return inputValue;
}
//
DenseIntOrFPElementsAttr Utils::getDenseAttr(ImplicitLocOpBuilder b,string typeStr,llvm::SmallVector<int64_t , 8> shape1){
  DenseIntOrFPElementsAttr inputAttr;
  int ElementNum = getElementNum(shape1);

//  Type elementType = genElementType(b,typeStr);
  if (typeStr=="i1"){
    auto inputValue = GenRandomTensor<bool>(ElementNum, typeStr);
    inputAttr = DenseIntElementsAttr::get(RankedTensorType::get(shape1, b.getI1Type()), inputValue);
  }
  else if (typeStr=="i8" || typeStr=="ui8" ){
    auto inputValue = GenRandomTensor<int8_t>(ElementNum, typeStr);
    inputAttr = DenseIntElementsAttr::get(RankedTensorType::get(shape1, b.getI8Type()), inputValue);
  }
  else if (typeStr=="i16" || typeStr=="ui16" ){
    auto inputValue = GenRandomTensor<int16_t>(ElementNum, typeStr);
    inputAttr = DenseIntElementsAttr::get(RankedTensorType::get(shape1, b.getIntegerType(16)), inputValue);
  }
  else if (typeStr=="i32" || typeStr=="ui32"){
    auto inputValue = GenRandomTensor<int32_t>(ElementNum, typeStr);
    inputAttr = DenseIntElementsAttr::get(RankedTensorType::get(shape1, b.getI32Type()), inputValue);
  }
  else if (typeStr=="i64" || typeStr=="ui64"){
    auto inputValue = GenRandomTensor<int64_t>(ElementNum, typeStr);
    inputAttr = DenseIntElementsAttr::get(RankedTensorType::get(shape1, b.getI64Type()), inputValue);
  }
  else if (typeStr=="f32"){
    auto inputValue = GenRandomTensor<float_t>(ElementNum, typeStr);
    inputAttr = DenseFPElementsAttr::get(
        RankedTensorType::get(shape1, b.getF32Type()), inputValue);
  }

  return inputAttr;
}

string Utils::getTypestr(string typeStr){
  string elementType;
  if (typeStr=="Int32Or64"){
    llvm::ArrayRef<string> types = {"i32","i64"};
    std::uniform_int_distribution<unsigned> randomType(0, types.size()-1);
    elementType=types[randomType(gen)];
  }
  else if(typeStr=="any"){
    llvm::ArrayRef<string> types = {"i1", "i8", "i16", "i32","i64","f32"};
    std::uniform_int_distribution<unsigned> randomType(0, types.size()-1);
    elementType=types[randomType(gen)];
  }else if(typeStr=="floatOruint"){
    llvm::ArrayRef<string> types = {"f32", "i8","i32","i64",};  // , "ui16",
    std::uniform_int_distribution<unsigned> randomType(0, types.size()-1);
    elementType=types[randomType(gen)];
  }else if(typeStr=="int"){
    llvm::ArrayRef<string> types = {"i1", "i8", "i16", "i32","i64"};
    std::uniform_int_distribution<unsigned> randomType(0, types.size()-1);
    elementType=types[randomType(gen)];
  }else if(typeStr=="i8ori16"){
    llvm::ArrayRef<string> types = {"i8", "i16"};
    std::uniform_int_distribution<unsigned> randomType(0, types.size()-1);
    elementType=types[randomType(gen)];
  }else if (typeStr=="anyNoI1"){
    llvm::ArrayRef<string> types = {"i8", "i16", "f32"};
    std::uniform_int_distribution<unsigned> randomType(0, types.size()-1);
    elementType=types[randomType(gen)];
  }else if (typeStr=="conv"){
    llvm::ArrayRef<string> types = {"f32"};
    std::uniform_int_distribution<unsigned> randomType(0, types.size()-1);
    elementType=types[randomType(gen)];
  }
  else
    elementType = normalizeType(typeStr);
  return elementType;
}

llvm::SmallVector<int64_t, 8> Utils::genBroadcastShape(){
  SmallVector<int64_t ,8> shape1 = getTensorShape(info.inputType[0]);
  SmallVector<int64_t,8> shape2 = getTensorShape(info.inputType[1]);

  SmallVector<int64_t ,8> BroadcastShape;
  if(shape1 == shape2)
    BroadcastShape.assign(shape1.begin(),shape1.end());
  else {
    if(shape1.size() == shape2.size()) {
      for (int i = 0; i < shape1.size(); i++) {
        BroadcastShape.push_back(shape1[i] > shape2[i] ? shape1[i]
                                                       : shape2[i]);
      }
    } else {
      auto longShape = shape1.size() > shape2.size() ? shape1 : shape2;
      auto shortShape = shape1.size() < shape2.size() ? shape1 : shape2;
      int diff = longShape.size() - shortShape.size();
      // int convertShape[] = {};
      for (int i = 0; i < longShape.size(); i++) {
        BroadcastShape.push_back(longShape[i]);
        if (i >= diff)
          BroadcastShape.push_back(longShape[i] > shortShape[i - diff]
                                   ? longShape[i]
                                   : shortShape[i - diff]);
      }
    }
  }
  return BroadcastShape;
}

Type Utils::str2type(ImplicitLocOpBuilder b, SmallVector<int64_t,8> inputShape, string elementType){
  Type tensorType;
  if (elementType == "i1")
    tensorType = mlir::RankedTensorType::get(inputShape, b.getI1Type());
  else if (elementType == "i8")
    tensorType = mlir::RankedTensorType::get(inputShape, b.getI8Type());
  else if (elementType == "i16")
    tensorType = mlir::RankedTensorType::get(inputShape, b.getIntegerType(16));
  else if (elementType == "i32")
    tensorType = mlir::RankedTensorType::get(inputShape, b.getI32Type());
  else if (elementType == "i64")
    tensorType = mlir::RankedTensorType::get(inputShape, b.getI64Type());
  else if (elementType == "f32")
    tensorType = mlir::RankedTensorType::get(inputShape, b.getF32Type());
  return  tensorType;
}

Type Utils::str2type(ImplicitLocOpBuilder b, llvm::ArrayRef<int64_t> inputShape, string elementType){
  SmallVector<int64_t, 8> shapes;
  for (auto s : inputShape) {
    shapes.push_back(s);
  }
  return str2type(b,shapes,elementType);
}


//
//SmallVector<Type,8> Utils::genTensorsType(ImplicitLocOpBuilder b, SmallVector<SmallVector<int64_t,8>,8> inputShapes, SmallVector<string,8> ElementTypes){
//  SmallVector<Type,8> Tensors;
//  Type type;
//  for (int i=0;i<ElementTypes.size();i++) {
//    string type_str = ElementTypes[i];
//    type = genUtils.str2type(b,inputShapes[i],ElementTypes[i]);
//    Tensors.push_back(type);
//  }
//  return Tensors;
//}
//
//SmallVector<Type,8> Utils::genTensorsType(ImplicitLocOpBuilder b, SmallVector<llvm::ArrayRef<int64_t>,8> inputShapes, SmallVector<string,8> ElementTypes){
//  SmallVector<Type,8> Tensors;
//  Type type;
//  for (int i=0;i<ElementTypes.size();i++) {
//    string type_str = ElementTypes[i];
//    type = genUtils.str2type(b,inputShapes[i],ElementTypes[i]);
//    Tensors.push_back(type);
//  }
//  return Tensors;
//}

Type Utils::genTensorType(ImplicitLocOpBuilder b, llvm::ArrayRef<int64_t> inputShape, string elementType){
  Type type = genUtils.str2type(b,inputShape,elementType);
  return type;
}

Type Utils::genTensorType(ImplicitLocOpBuilder b, SmallVector<int64_t,8> inputShape, string elementType){
  Type type = genUtils.str2type(b,inputShape,elementType);
  return type;
}



bool Utils::typeMatch(Value preOp){
//  auto elementType = pre_result.cast<ShapedType>().getElementType();
  //当前待插入节点的输入信息

  Type pre_result = preOp.getType();
//  cout<<"tensor type of prenode: ";
//  pre_result.dump();
  SmallVector<string,8> curOpType = getTypeFromopConstraint();
  SmallVector<SmallVector<int64_t,8>,8> curOpDim = getDimFromopConstraint();
//  cout<<"Constraint of curnode: "<<curOpType[0];

//  cout<<"Constraint of curnode:  [";
//  for (auto i : curOpDim[0]){
//    cout<<i;
//  }
//  cout<<" ]"<<endl;

  //pre节点的输出信息
//  string preOpType = getTensorType(pre_result);
  auto preOpType = pre_result.cast<ShapedType>().getElementType();
  SmallVector<int64_t,8> preOpShape = getTensorShape(pre_result);


  bool flag_type = false;
  bool flag_dim = false;
  SmallVector<int64_t,8> indexes = {};
  //对类型进行匹配
  for (int i = 0; i < curOpType.size(); i++) {
    string inputType = curOpType[i];
    if (inputType == "any"){
      flag_type = true;
    }
    else if (inputType == "Int32Or64"){
      if(preOpType.isInteger(32) || preOpType.isInteger(64))
        flag_type = true;
    }
    else if (inputType == "floatOruint"){
//      SmallVector<StringRef , 8> types ={"f32","ui8","ui32","ui64"};
      if(preOpType.isF32() ||preOpType.isInteger(8) || preOpType.isInteger(32) || preOpType.isInteger(64))
        flag_type = true;
    }
    else if (inputType == "int"){
//      SmallVector<StringRef , 8> types ={"i1","i8","i16","i32","i64"};
      if (preOpType.isIntOrIndex())
        flag_type = true;
    }
    else if (inputType == "i8ori16"){
      if(preOpType.isInteger(8) || preOpType.isInteger(16))
        flag_type = true;
    }
    else if (inputType == "anyNoI1"){
      if(preOpType.isInteger(8) || preOpType.isInteger(16) || preOpType.isF32())
        flag_type = true;
    }
    else if (inputType==type2str(preOpType)) {
      flag_type = true;
    }

    int preOpDim = preOpShape.size();

    //对shape的维度进行匹配
    SmallVector<int64_t,8> inputShapeDIM;
    for (int i = 0; i < curOpDim.size(); ++i) {
      if (curOpDim[i].size() == 1){
        if(curOpDim[i][0] == 100 || curOpDim[i][0]==preOpDim)
          flag_dim = true;
      }
      else if (preOpDim>=curOpDim[i].front() && preOpDim <= curOpDim[i].back())
        flag_dim = true;
      
      break;
    }
    break;
  }

  return (flag_type && flag_dim);
}


bool Utils::addSkips(Value preOp){
  auto preOpType = preOp.getType().cast<ShapedType>();
//  SmallVector<int64_t,8> preOpShape = getTensorShape(pre_result);

  bool flag = false;
  //对类型进行匹配
  auto arg1Ety= info.inputType[0].cast<ShapedType>().getElementType();
  auto preEty= preOpType.getElementType();

  auto arg1Shape = info.inputType[0].cast<ShapedType>().getShape();
  auto preShape = preOpType.getShape();

  if(arg1Ety==preEty){  //type matching
    if(arg1Shape==preShape) //shape matching
       flag = true;
  }
  return flag;
}


Value Utils::skipMatch(SmallVector<Value> valuePool){
  SmallVector<Value> valuePicked;
  if(info.inputNum==2){
    for(auto v:valuePool){
       if(genUtils.addSkips(v)){
        valuePicked.push_back(v);
       }
    }
  }
  Value v = nullptr;
  if(!valuePicked.empty())
     v = valuePicked[genUtils.genRandomN(0,valuePicked.size()-1)];
  return v;
}

SmallVector<SmallVector<int64_t,8>,8> Utils::getDimFromopConstraint(){
  auto inputInfo = info.Constraint["INPUT"];
  int inputNum = info.inputNum;
  int Dim;
  SmallVector<SmallVector<int64_t, 8>, 8> inputShapesDIM;

  for (int i = 0;i < inputNum; i++) { // 循环遍历“INPUT”中的多个对象，获取每个对象中的维度大小
    SmallVector<int64_t, 8> inputDIM;
    int DimNum = inputInfo[i]["DIM"].size();

    for (int j = 0; j < DimNum; j++) {
      Dim = inputInfo[i]["DIM"][j].asInt();
      inputDIM.push_back(Dim);
    }
    inputShapesDIM.push_back(inputDIM);
  }
  return inputShapesDIM;
}

SmallVector<string,8> Utils::getTypeFromopConstraint(){
  auto inputInfo = info.Constraint["INPUT"];
  int inputNum = info.inputNum;
  SmallVector<string, 8> ElementTypeVector;
  string typeStr;

  for (int i = 0;i < inputNum; i++) { // 循环遍历“INPUT”中的多个对象，获取每个对象中的形状
    typeStr = inputInfo[i]["TYPE"].asString();
    ElementTypeVector.push_back(normalizeType(typeStr));
  }
  return ElementTypeVector;
}

SmallVector<Value> Utils::collectInsertPoint(func::FuncOp funcOp){
  SmallVector<Value> allOpsResultValue;
  funcOp.walk([&](Operation *op) {
    if (!mlir::isa<func::ReturnOp>(op)){
      if (!mlir::isa<func::FuncOp>(op))
        allOpsResultValue.push_back(op->getResult(0));
    }
  });
//  SmallVector< pair<string, SmallVector<int64_t, 8>> > allOpsResult_Type_Shape;
//  for (int j = 0; j < allOpsResultValue.size(); j++) {
//    allOpsResult_Type_Shape.push_back(infogen.getResultInfo(allOpsResultValue[j]));
//  }
  return allOpsResultValue;
}


SmallVector<Value> Utils::typeMatch(SmallVector<Value> valuePool){
  SmallVector<Value> candidates = {};
  for(Value op : valuePool){
    if(typeMatch(op)){
      candidates.push_back(op);
      op.dump();
    }

  }

  return candidates;
  // if(candidates.empty())
  //   return nullptr;
  // else
  //   return candidates[genUtils.genRandomN(0,candidates.size()-1)];
}






void Utils::printTypes(SmallVector<Type,8> tensors){
  for(auto t :tensors ){
    t.dump();
  }
}

void Utils::printValues(SmallVector<Value,8> values){
  for(auto t :values ){
    t.dump();
  }
}

void Utils::getOpType( llvm::ArrayRef<StringRef> opPool) {
  SmallVector<string,8> any_type;
  SmallVector<string,8> int_type;
  SmallVector<string,8> i32Or64_type;
  SmallVector<string,8> i8ori16_type;
  SmallVector<string,8> float_type;
  SmallVector<string,8> floatOruint_type;
  SmallVector<string,8> i1_type;
  SmallVector<string,8> i8_type;
  SmallVector<string,8> i16_type;
  SmallVector<string,8> i32_type;
  SmallVector<string,8> i64_type;

  for(auto opname : opPool){
    string op = opname.str();
    auto opConstraint = genUtils.getConstraint(op);
    for (auto input : opConstraint["INPUT"]) {
      int len = input["TYPE"].size();
      string typeStr = input["TYPE"].asString();
      if (typeStr=="i1" || typeStr=="I1")
        i1_type.push_back(op);
      else if (typeStr=="I8" || typeStr=="i8" )
        i8_type.push_back(op);
      else if (typeStr=="I16" || typeStr=="i16" )
        i16_type.push_back(op);
      else if (typeStr=="I32" || typeStr=="i32" || typeStr=="Int32" || typeStr=="int32" )
        i32_type.push_back(op);
      else if (typeStr=="I64" || typeStr=="i64" || typeStr=="Int64" || typeStr=="int64" )
        i64_type.push_back(op);
      else if(typeStr=="any" )
        any_type.push_back(op);
      else if (typeStr=="Int32Or64"){
        i32Or64_type.push_back(op);
      }else if(typeStr=="floatOruint"){
        floatOruint_type.push_back(op);
      }else if(typeStr=="int"){
        int_type.push_back(op);
      }else if(typeStr=="i8ori16"){
        i8ori16_type.push_back(op);
      }else if(typeStr=="float"){
        float_type.push_back(op);
      }
      break;
    }
  }

  opsType.insert({100,any_type});
  opsType.insert({1,int_type});
  opsType.insert({2,i1_type});
  opsType.insert({3,i8_type});
  opsType.insert({4,i16_type});
  opsType.insert({5,i32_type});
  opsType.insert({6,i64_type});
  opsType.insert({7,i8ori16_type});
  opsType.insert({8,i32Or64_type});
  opsType.insert({9,float_type});
  opsType.insert({10,floatOruint_type});

  //    return opContent;
}

void Utils::getOpDim( llvm::ArrayRef<StringRef> opPool) {
  SmallVector<string,8> anydim;
  SmallVector<string,8> maxdim6;
  SmallVector<string,8> maxdim4;
  SmallVector<string,8> dim5;
  SmallVector<string,8> dim4;
  SmallVector<string,8> dim3;
  SmallVector<string,8> dim2;
  SmallVector<string,8> dim1;


  for(auto opname : opPool){
    string op = opname.str();
    auto opConstraint = genUtils.getConstraint(op);
    for (auto input : opConstraint["INPUT"]) {
      int len = input["DIM"].size();
      if(len == 1){
        if (input["DIM"][len-1].asInt()==100)
          anydim.push_back(op);
        else if (input["DIM"][len-1].asInt()==5)
          dim5.push_back(op);
        else if (input["DIM"][len-1].asInt()==4)
          dim4.push_back(op);
        else if (input["DIM"][len-1].asInt()==3)
          dim3.push_back(op);
        else if (input["DIM"][len-1].asInt()==2)
          dim2.push_back(op);
        else if (input["DIM"][len-1].asInt()==1)
          dim1.push_back(op);
      }else{
        if (input["DIM"][len-1].asInt()==6)
          maxdim6.push_back(op);
        else if (input["DIM"][len-1].asInt()==4)
          maxdim4.push_back(op);
      }
      break;
    }
  }

  opsDim.insert({100,anydim});
  opsDim.insert({16,maxdim6});
  opsDim.insert({14,maxdim4});
  opsDim.insert({5,dim5});
  opsDim.insert({4,dim4});
  opsDim.insert({3,dim3});
  opsDim.insert({2,dim2});
  opsDim.insert({1,dim1});

  //    return opContent;

}
void Utils::analyseAllOp(llvm::ArrayRef<StringRef> opPool) {
  getOpDim(opPool);
  getOpType(opPool);
}


// 根据上一个op,选择满足约束的下一个opcode
string Utils::getNextOp(mlir::Value curOp) {
  string selectedOp = " ";

  pair<string, SmallVector<int64_t, 8>> constraint_info = infogen.getResultInfo(curOp);
  int dim = constraint_info.second.size();
  string type = constraint_info.first;
  SmallVector<string, 8> opname1;
  SmallVector<string, 8> opname2;

  if(dim==0){
    selectedOp = " ";
    return selectedOp;
  }

  if (dim == 5)
    opname1.append(opsDim[dim]);
  else if (dim < 5){
    opname1.append(opsDim[dim]);
    opname1.append(opsDim[14]);
  }
  opname1.append(opsDim[16]);
  opname1.append(opsDim[100]);

  auto isUint = std::find(type.begin(),type.end(),'u');
  auto isInt = std::find(type.begin(),type.end(),'i');
  if (isUint!=type.end())
    opname2.append(opsType[10]);
  else if (isInt!=type.end()){
    if (type == "i1"){
      opname2.append(opsType[2]);
    }else  if (type == "i8"){
      opname2.append(opsType[3]);
      opname2.append(opsType[7]);
    }else  if (type == "i16"){
      opname2.append(opsType[4]);
      opname2.append(opsType[7]);
    }else  if (type == "i32"){
      opname2.append(opsType[5]);
      opname2.append(opsType[8]);
    }else  if (type == "i64"){
      opname2.append(opsType[6]);
      opname2.append(opsType[8]);
    }
    opname2.append(opsType[1]);
  }else{
    opname2.append(opsType[9]);
    opname2.append(opsType[10]);
  }
  opname2.append(opsType[100]);

  SmallVector<string, 8> optionalOp;
  //  cout<< "optionalOp : ";
  for(string name : opname1){
    auto isExist = std::find(opname2.begin(),opname2.end(),name);

    if(isExist != opname2.end()){
      optionalOp.push_back(name);
      //      cout<< name<< ",";
    }
  }
  unsigned int r = genUtils.genRandomN(0,optionalOp.size()-1);
  return optionalOp[r];
}


llvm::SmallVector<int64_t, 8> Utils::getShapeVector(llvm::ArrayRef<int64_t> shape){
  llvm::SmallVector<int64_t, 8> shapeVec = {};
  for (auto s : shape){
    shapeVec.push_back(s);
  }
  return shapeVec;
}