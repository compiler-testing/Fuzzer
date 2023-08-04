////
//// Created by Administrator on 2023/2/21.
////
//#include "TestDialect.h"
//#include "mlir/TosaGen/TosaGenUtils.h"
//#include "mlir/TosaGen/createOp.h"
//#include "mlir/TosaGen/opInfo.h"
//
//using namespace mlir;
//using namespace std;
//using namespace test;
//
//
//extern TosaGenUtils genUtils;
//extern InputsGen inputgen;
//extern opInfo info;
//extern InfoGen infogen;
//extern CreateOp create;
//
//bool isNeedBroadcast (StringRef opName){
//  bool isBroadcast;
//  SmallVector<StringRef , 8> opConNames ={"select","concat","add","bitwise_and","bitwise_or","bitwise_xor","div","equal","greater","greater_equal","logical_and","logical_left_shift","logical_or","logical_right_shift","logical_xor","maximum","minimum","pow","sub","arithmetic_right_shift","mul","select"}; //,"cond_if"
//  auto isConOp = std::find(opConNames.begin(), opConNames.end(), opName.str());
//  if(isConOp != opConNames.end())
//    isBroadcast=1;
//  else
//    isBroadcast=0;
//  return isBroadcast;
//}
//
//pair<int,int> getTwoRandom_Num(SmallVector<Operation*> opsList){
//  default_random_engine int_engine; //定义随机数引擎
//  uniform_int_distribution<int> distribution(0, opsList.size()-1);
//  int N1 = distribution(int_engine);
//  int N2 = distribution(int_engine);
//  while(N1 == N2)
//    N2 = distribution(int_engine);
//  pair<int,int> random_num;
//  random_num.first = N1;
//  random_num.second = N2;
//  return random_num;
//}
//
//template <typename OpTy>
//Value createOpImplement(ImplicitLocOpBuilder b, Location loc, SmallVector<Value,8> inputs, SmallVector<NamedAttribute> newAttrs) {
//  Type outType;
//  Value newOp;
//  SmallVector<int64_t,8> InOut_Shape;
//  string InOut_Type;
//
//  InOut_Shape = infogen.getResultInfo(inputs[0]).second;
//  InOut_Type = infogen.getResultInfo(inputs[0]).first;
//
//  Type resultType = genUtils.genElementType(b, InOut_Type);
//  outType = RankedTensorType::get(InOut_Shape, resultType);
//  //outType.dump();
//
//  //outType = RankedTensorType::get(info.resultShape, resultType);
//  if(newAttrs.size()==0)
//    newOp  =b.create<OpTy>(loc,outType,inputs);
//  else if(newAttrs.size()==1)
//    newOp  =b.create<OpTy>(loc,outType,inputs,newAttrs);
//  else if (newAttrs.size()>1)
//    newOp = b.create<OpTy>(loc,outType,inputs,newAttrs);
//  //newOp.dump();
//  return newOp;
//}
//
//
//Value CreateOpWithNoAttr(ImplicitLocOpBuilder b, Location loc, StringRef op_name, SmallVector<Value,8> inputs, SmallVector<NamedAttribute> newAttrs) {
//  Value newop;
//  string selectedOp = op_name.str();
//  newAttrs.clear();
//  newAttrs.push_back(b.getNamedAttr("axis",b.getI64IntegerAttr(0))); //随便写一个属性，创建时不使用
//  if(selectedOp=="tosa.abs")
//    newop = createOpImplement<tosa::AbsOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.add")
//    newop = createOpImplement<tosa::AddOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.bitwise_and")
//    newop = createOpImplement<tosa::BitwiseAndOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.bitwise_not")
//    newop = createOpImplement<tosa::BitwiseNotOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.bitwise_or")
//    newop = createOpImplement<tosa::BitwiseOrOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.bitwise_xor")
//    newop = createOpImplement<tosa::BitwiseXorOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.cast")
//    newop = createOpImplement<tosa::CastOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.ceil")
//    newop = createOpImplement<tosa::CeilOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.clz")
//    newop = createOpImplement<tosa::ClzOp>(b,loc,inputs,newAttrs);
//  /*else if(selectedOp=="cond_if")  //cond_if----if
//    newop = createOpImplement<tosa::IfOp>(b,loc,selectedOp,inputs,{});*/
//  else if(selectedOp=="tosa.div")
//    newop = createOpImplement<tosa::DivOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.equal")
//    newop = createOpImplement<tosa::EqualOp>(b,loc,inputs,newAttrs);
//  if(selectedOp=="tosa.exp")
//    newop = createOpImplement<tosa::ExpOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.floor")
//    newop = createOpImplement<tosa::FloorOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.gather")
//    newop = createOpImplement<tosa::GatherOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.greater")
//    newop = createOpImplement<tosa::GreaterOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.greater_equal")
//    newop = createOpImplement<tosa::GreaterEqualOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.identity")
//    newop = createOpImplement<tosa::IdentityOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.log")
//    newop = createOpImplement<tosa::LogOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.logical_and")
//    newop = createOpImplement<tosa::LogicalAndOp>(b,loc,inputs,newAttrs);
//  if(selectedOp=="tosa.logical_left_shift")
//    newop = createOpImplement<tosa::LogicalLeftShiftOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.logical_not")
//    newop = createOpImplement<tosa::LogicalNotOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.logical_or")
//    newop = createOpImplement<tosa::LogicalOrOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.logical_right_shift")
//    newop = createOpImplement<tosa::LogicalRightShiftOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.logical_xor")
//    newop = createOpImplement<tosa::LogicalXorOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.maximum")
//    newop = createOpImplement<tosa::MaximumOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.minimum")
//    newop = createOpImplement<tosa::MinimumOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.pow")
//    newop = createOpImplement<tosa::PowOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.reciprocal")
//    newop = createOpImplement<tosa::ReciprocalOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.rsqrt")
//    newop = createOpImplement<tosa::RsqrtOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.scatter")
//    newop = createOpImplement<tosa::ScatterOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.select")
//    newop = createOpImplement<tosa::SelectOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.sigmoid")
//    newop = createOpImplement<tosa::SigmoidOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.sub")
//    newop = createOpImplement<tosa::SubOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.table")
//    newop = createOpImplement<tosa::TableOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.tanh")
//    newop = createOpImplement<tosa::TanhOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.transpose")
//    newop = createOpImplement<tosa::TransposeOp>(b,loc,inputs,newAttrs);
//  /*else if(selectedOp=="while_loop")
//    newop = createOpImplement<tosa::WhileOp>(b,loc,selectedOp,inputs,{});*/
//  return newop;
//}
//
//Value CreateOpWithOneAttr(ImplicitLocOpBuilder b, Location loc, StringRef op_name, SmallVector<Value,8> inputs, SmallVector<NamedAttribute> newAttrs){
//  Value newop;
//  string selectedOp = op_name.str();
//  if(selectedOp=="tosa.educe_max")
//    newop = createOpImplement<tosa::ReduceMaxOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.reduce_any")
//    newop = createOpImplement<tosa::ReduceAnyOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp == "tosa.reduce_all")
//    newop = createOpImplement<tosa::ReduceAllOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp == "tosa.reduce_prod")
//    newop = createOpImplement<tosa::ReduceProdOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp == "tosa.reduce_min")
//    newop = createOpImplement<tosa::ReduceMinOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp == "tosa.reduce_sum")
//    newop = createOpImplement<tosa::ReduceSumOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp == "tosa.argmax")
//    newop = createOpImplement<tosa::ArgMaxOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp == "tosa.reverse")
//    newop = createOpImplement<tosa::ReverseOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp == "tosa.concat")
//    newop = createOpImplement<tosa::ConcatOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp == "tosa.reshape")
//    newop = createOpImplement<tosa::ReshapeOp>(b,loc,inputs,newAttrs);
//  return newop;
//}
//
//Value CreateOpWithMulAttrs(ImplicitLocOpBuilder b, Location loc, StringRef op_name, SmallVector<Value,8> inputs, SmallVector<NamedAttribute> newAttrs){
//  Value newop;
//  string selectedOp = op_name.str();
//  if(selectedOp=="tosa.conv2d")
//    newop = createOpImplement<tosa::Conv2DOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.conv3d")
//    newop = createOpImplement<tosa::Conv3DOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.transpose_conv2d")
//    newop = createOpImplement<tosa::TransposeConv2DOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.depthwise_conv2d")
//    newop = createOpImplement<tosa::DepthwiseConv2DOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.avg_pool2d")
//    newop = createOpImplement<tosa::AvgPool2dOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.max_pool2d")
//    newop = createOpImplement<tosa::MaxPool2dOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.slice")
//    newop = createOpImplement<tosa::SliceOp>(b,loc,inputs,newAttrs);
////  else if(selectedOp=="tosa.reluN")
////    newop = createOpImplement<tosa::ReluNOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.clamp")
//    newop = createOpImplement<tosa::ClampOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.resize")
//    newop = createOpImplement<tosa::ResizeOp>(b,loc,inputs,newAttrs);
//  else if(selectedOp=="tosa.rescale")
//    newop = createOpImplement<tosa::RescaleOp>(b,loc,inputs,newAttrs);
//  return newop;
//}
//
//
//Value createOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc, StringRef op_name, SmallVector<Value,8> inputs, SmallVector<NamedAttribute> newAttrs) {
//  Value newOp;
//  if (newAttrs.size() == 0)
//    newOp = CreateOpWithNoAttr(b, loc, op_name, inputs, newAttrs);
//  else if (newAttrs.size() == 1)
//    newOp = CreateOpWithOneAttr(b, loc, op_name, inputs, newAttrs);
//  else if (newAttrs.size() > 1)
//    newOp = CreateOpWithMulAttrs(b, loc, op_name,inputs, newAttrs);
//  return newOp;
//}
//
//pair<Value,Value> insertOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc, Operation* upOp, Operation* downOp){
//  SmallVector<Value,8> upOp_input ;
//  SmallVector<Value,8> downOp_input ;
//  SmallVector<NamedAttribute> upOp_attrs;
//  SmallVector<NamedAttribute> downOp_attrs;
//  //获取 upOp,downOp 的inputs,attrs,opName
//  cout<<"=====upOp"<<endl;
//  upOp->dump();
//  cout<<"=====downOp"<<endl;
//  downOp->dump();
////  cout<<"=====upOp many inputs"<<endl;
//  for (int i=0;i<upOp->getNumOperands();i++) {
//    upOp_input.push_back(upOp->getOperand(i));
////    input.dump();
//  }
////  cout<<"=====downOp many inputs"<<endl;
//  for (int i=0;i<downOp->getNumOperands();i++) {
//    downOp_input.push_back(downOp->getOperand(i));
////    input.dump();
//  }
//  for(auto attr:upOp->getAttrs())
//    upOp_attrs.push_back(attr);
//  for(auto attr:downOp->getAttrs())
//    downOp_attrs.push_back(attr);
//  StringRef upOp_name = upOp->getName().getStringRef();
//  StringRef downOp_name = downOp->getName().getStringRef();
//
//  Value newOp1;
//  Value newOp2;
//  Operation* insertPoint;
//
//  //先将下面的算子插入上面，即downOp->upOp
//  //将downOp所需的输入个数与upOp输入个数进行匹配
//  Value input = upOp_input[0];
//  upOp_input.clear();
//  for (int i=0;i<downOp_input.size();i++) {
//    upOp_input.push_back(input);
//  }
//  insertPoint = upOp;
//  b.setInsertionPoint(insertPoint);
//  newOp1 = createOp(b,loc,downOp_name,upOp_input,downOp_attrs);
////  cout<<"=====newOp1"<<endl;
////  newOp1.dump();
//
//  //将上面的算子插入下面，即upOp->downOp
//  //将upOp所需的输入个数与downOp输入个数进行匹配
//  input = downOp_input[0];
//  downOp_input.clear();
//  for (int i=0;i<upOp_input.size();i++) {
//    downOp_input.push_back(input);
//  }
//  insertPoint = downOp;
//  b.setInsertionPoint(insertPoint);
//  newOp2 = createOp(b,loc,upOp_name,downOp_input,upOp_attrs);
////  cout<<"=====newOp2"<<endl;
////  newOp2.dump();
//  pair<Value,Value> newOp1_newOp2;
//  newOp1_newOp2.first = newOp1;
//  newOp1_newOp2.second = newOp2;
//  return newOp1_newOp2;
//}
//
//namespace {
//struct NSPass
//    : public PassWrapper<NSPass, OperationPass<func::FuncOp>> {
//  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(NSPass)
//  StringRef getArgument() const final { return "NS"; }
//  StringRef getDescription() const final { return "Node Switch."; }
//  void getDependentDialects(DialectRegistry &registry) const override {
//    registry.insert<tosa::TosaDialect>();
//  }
//  void runOnOperation() override {
//    cout << "NS" << endl;
//    auto funcOp = getOperation();
//    SmallVector<Operation *> ops;
//
//    Block *firstbb = &(*funcOp.begin());
//    Location loc = firstbb->begin()->getLoc();
//    ImplicitLocOpBuilder b = ImplicitLocOpBuilder::atBlockBegin(loc, firstbb);
//    b.setInsertionPointToStart(firstbb);
//
//    SmallVector<Value, 8> OPinput;
//    Value constOp;
//    Type elementType;
//    // cout << "===========create ConstOp:" << info.inputNum <<"============" << endl;
//    SmallVector<int64_t,8> inputShape;
//    SmallVector<string,8> inputTypes;
//    inputShape = {1,2};
//    //inputShapes[0].push_back(2);
//    inputTypes.push_back("i32");
//    DenseIntOrFPElementsAttr inputAttr = genUtils.getDenseAttr(b, inputTypes[0], inputShape);
//    constOp = b.create<tosa::ConstOp>(loc, inputAttr.getType(), inputAttr);
//    constOp.dump();
//
//
////
////    funcOp.walk([&](Operation *op) {
////      if (!mlir::isa<func::ReturnOp>(op)){
////        if (!mlir::isa<func::FuncOp>(op))
////          if (op->getNumOperands() > 0)
////            ops.push_back(op);
////      }
////    });
////
////    //获取输入输出相同的节点
////    SmallVector<Operation*> ops_sameInOut;
////    for (auto op : ops){
////      //取输入输出相同的节点
////      if (op->getOperand(0).getType() == op->getResult(0).getType()){
////        ops_sameInOut.push_back(op);
////      }
////    }
////
////
////    Operation* upOp;  //上方待交换的节点
////    Operation* downOp; //下方待交换的节点
////    SmallVector<Operation*> ops_NoBroadcast;
////
////    //获取不需要广播的算子
////    for (auto op:ops_sameInOut) {
////      StringRef opName = op->getName().getStringRef();
////      if (!isNeedBroadcast(opName)){ //不需要广播
////        ops_NoBroadcast.push_back(op);
////      }
////    }
////
////    //在不需要广播的算子中随机获取 upOp,downOp
////
////    pair<int,int> n1_n2 = getTwoRandom_Num(ops_NoBroadcast);
////    upOp = ops_NoBroadcast[n1_n2.first];
////    downOp = ops_NoBroadcast[n1_n2.second];
////
////    //downOp插入upOp算子前一行；upOp插入downOp算子前一行，返回两个新创建的算子newOp1,newOp2
////    pair<Value,Value> newOp1_newOp2 = insertOp(b,loc,upOp,downOp);
////    Value newOp1 = newOp1_newOp2.first;
////    Value newOp2 = newOp1_newOp2.second;
////
////    //进行操作数替换
////    for(auto op:ops){
////      for(int i=0;i<op->getNumOperands();i++){
////        if (op->getOperand(i) == upOp->getResult(0))
////          op->setOperand(i,newOp1);
////        else if (op->getOperand(i) == downOp->getResult(0))
////          op->setOperand(i,newOp2);
////      }
////    }
////    cout<<"=====delete before"<<endl;
////    funcOp->dump();
////    cout<<"=====delete upOp"<<endl;
////    upOp->dump();
////    upOp->erase();
////    cout<<"=====delete downOp"<<endl;
////    downOp->dump();
////    downOp->erase();
////    cout<<"=====delete after"<<endl;
////    funcOp->dump();
//
//
//  }
//};
//}
//namespace mlir {
//namespace test {
//void registerNSPass() { PassRegistration<NSPass>();
//}
//}}
