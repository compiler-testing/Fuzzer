//
// Created by Administrator on 2023/2/7.
//
#include "mlir/TosaGen/utils.h"
#include "mlir/TosaGen/create.h"
#include "mlir/TosaGen/opinfo.h"
#include "mlir/TosaGen/transfer.h"
#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h"
#include "mlir/Dialect/Bufferization/IR/Bufferization.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/Bufferization/Transforms/Bufferize.h"
#include <map>
#include <random>

using namespace mlir;
using namespace std;

extern Utils genUtils;
extern InfoGen infogen;
extern Create create;
extern opInfo info;
extern Transfer transfer;

DenseMap<int, SmallVector<string,8>>  opsDim;
DenseMap<int, SmallVector<string,8>>  opsType;

std::random_device rd3;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen3(rd3());
#define DEBUG_TYPE "user-define-pass"

namespace {
struct MIXPass : public PassWrapper<MIXPass, OperationPass<func::FuncOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(MIXPass)
  SmallVector<Value> insertTosaOp(ImplicitLocOpBuilder b,Location loc,Value conOp,func::FuncOp func, SmallVector<Value> valuePool);
  Value Conversion(ImplicitLocOpBuilder b,Location loc,Value insOp,Value conOp,func::FuncOp f);
  Value shapeConversion(ImplicitLocOpBuilder b,Location loc,Value insOp,Value conOp,func::FuncOp f);
  Value createReshapeOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc, mlir::Value insOp,
                        llvm::SmallVector<int64_t , 8> newShape);
  Value createConstOp(ImplicitLocOpBuilder b, Location loc,  string et, llvm::SmallVector<int64_t , 8> concatShape,func::FuncOp f);
  Value createConcatOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc, mlir::Value insOp,
                       Value constOp);
  Value createConstOpForMixConvert(ImplicitLocOpBuilder b, Location loc,string type, llvm::SmallVector<int64_t , 8> newShape,func::FuncOp f);
  Value createCastOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc, mlir::Value insOp,
                     Value constOp);
  Value createSliceOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc, mlir::Value insOp,
                      llvm::SmallVector<int64_t , 8> con_Shape);
  Value createReduceOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc, mlir::Value insOp,
                       int axis);
  void runOnOperation() override {



    cout << "Mix"<< endl;

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
      bb = blockStack[randomNum(gen3)];
    }while(&(*bb->begin())==bb->getTerminator());

    int times = 1;
    while (times--) {
    //auto &bbs = f.getBody().getBlocks();
   // for (auto &bb : f) {
      Location loc = bb->begin()->getLoc();
      ImplicitLocOpBuilder b = ImplicitLocOpBuilder::atBlockBegin(loc, bb);
      b.setInsertionPointToStart(bb);

      Operation * returnOp;
      // 获取所有可能的插入点
      SmallVector<Operation *> rawOps;
      llvm::SmallVector<string> specialOp = {"memref.load","tensor.extract","memref.store"};  //取第一个操作数
      getOperation().walk([&](Operation *op) {
        if (!mlir::isa<func::ReturnOp>(op)){
//          if ( (op->getNumOperands()>0) && (op->getName().getStringRef() != "scf.for") ){
          if (op->getNumResults()==1){
//                if(op->getResult(0).getType().isa<TensorType, MemRefType>()){
            if(op->getResult(0).getType().isa<TensorType>()){
              rawOps.push_back(op);
            }
          }
        } else{
          returnOp = op;
        }

      });

      //打印rawOps
//      for (int i = 0; i < rawOps.size(); ++i) {
////        cout<<"rawOps 序号："<<i<<endl;
//        rawOps[i]->dump();
//      }

      //类型判断：如果是memref，需要准换成tensor,
      //插入完成后，再把tensor转成memref

      //选择插入点,对特殊op的操作数选取进行限制
      unsigned int r ;
      unsigned int n ;
      SmallVector<Value> valuePool;
      if(rawOps.empty())
        break;
      else{
        if (rawOps.size() == 1){
          r = 0;
          valuePool.push_back(rawOps[r]->getResult(0));
        }
        else{
          r = genUtils.genRandomN(1,rawOps.size()-1);
          for(auto op : rawOps){
            valuePool.push_back(rawOps[r]->getResult(0));
            if(op==rawOps[r])
              break;
          }
        }
      }

      auto curOp = rawOps[r];  //currentOp 当前插入点
      cout<<"当前插入点："<<endl;
      curOp->dump();
      b.setInsertionPointAfter(curOp);
      Value conOp = curOp->getResult(0);
//      if (conOp.getType().isa<MemRefType>()) cout<<"memref type"<<endl;
//      else cout<<"no memref"<<endl;
      //对当前插入点的操作数进行类型判断，如果是memref，需要转为tensor。插入完成后，再把tensor转成memref
      if (conOp.getType().isa<MemRefType>()){
        cout<<"============ memref to tensor"<<endl;
        conOp = b.create<bufferization::ToTensorOp>(loc, conOp);
//        conOp.dump();
      }

      cout<<"=============new tosa insOp: "<<endl;
      SmallVector<Value> allInsOps = insertTosaOp(b,loc,conOp,f, valuePool);
      auto new_tosa_insOp = allInsOps.back();
      new_tosa_insOp.dump();
      if(allInsOps.empty())
        break;
//
//      //转换算法实现 Conversion();
      Value newInsOp = Conversion(b,loc,new_tosa_insOp,conOp,f);
      cout<<"=============converted op"<<endl;
      newInsOp.dump();
//      f.dump();

      SmallPtrSet<Operation *,4> exceptUser;
      for(auto op : allInsOps){
        exceptUser.insert(op.getDefiningOp());
      }


      conOp.replaceAllUsesExcept(newInsOp,exceptUser);


      f.dump();
//
//      //如果当前插入点是memref。插入tosa完成后，再把tensor转成memref
//      insOp = curOp->getOperand(n);
//      if (insOp.getType().isa<MemRefType>()){
//        cout<<"============ tensor to memref"<<endl;
//        Type type = insOp.getType();
//        newInsOp = b.create<mlir::bufferization::ToMemrefOp>(loc,type,newInsOp);
//        newInsOp.dump();
//      }
//      curOp->setOperand(n,newInsOp);
      do{
        bb = blockStack[randomNum(gen3)];
      }while(&(*bb->begin())==bb->getTerminator());
    }

  };



  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<tosa::TosaDialect,scf::SCFDialect,arith::ArithDialect,mlir::bufferization::BufferizationDialect>();
  }
  StringRef getArgument() const final { return "Mix"; }
  StringRef getDescription() const final { return "Mix pass"; }
};

} // namespace

///constraint_Info,ins_Info,result_Info,insOp,->return newInsOp
Value MIXPass::shapeConversion(ImplicitLocOpBuilder b,Location loc,Value insOp,Value conOp,func::FuncOp func){
  pair<string, SmallVector<int64_t, 8>> con_Info = infogen.getResultInfo(conOp);
  pair<string, SmallVector<int64_t, 8>> ins_Info = infogen.getResultInfo(insOp);
  string con_type = con_Info.first;
  string ins_type = ins_Info.first;
  llvm::SmallVector<int64_t , 8> con_shape = con_Info.second;
  llvm::SmallVector<int64_t , 8> ins_shape = ins_Info.second;

  Value newInsOp;
  pair<string, SmallVector<int64_t,8>> result_Info;
  result_Info = con_Info;

  int e1 = insOp.getType().cast<ShapedType>().getNumElements();  //10
  int e2 = conOp.getType().cast<ShapedType>().getNumElements();  //24
  float i = (float)e1 / e2;
  if (i==1) { // shape不同，但元素个数相同   {2,12}
    newInsOp = createReshapeOp(b,loc,insOp,con_shape);
  }
  else if (i<1){
    SmallVector<int> index; //
    int pro = 1;
    llvm::SmallVector<int64_t , 8> concatShape;
    if(ins_shape.size()==con_shape.size()){
      for(int i = 0;i<ins_shape.size();i++){
        if(ins_shape[i]==con_shape[i]){
          index.push_back(i);
          concatShape.push_back(ins_shape[i]);
        }
        else{
          pro = (con_shape[i]/ins_shape[i])*pro;
          concatShape.push_back(con_shape[i]-ins_shape[i]);
        }
      }
    }

    if(!index.empty() && con_shape.size()-index.size()==1){
      float en = e2/e1;
      if(e2/e1 - (int)e2/e1==0){
        if ((int)en==pro){
          cout<<"expand shape"<<endl;
        }

        Value constOp = createConstOp( b,  loc,  ins_type,  concatShape,func);
        newInsOp = createConcatOp(b,loc,insOp,constOp);
      }
    }else{
      llvm::SmallVector<int64_t , 8> newShape;
      for(auto x :con_shape){
        if(*con_shape.begin()==x)
          newShape.push_back(e2-e1);
        else
          newShape.push_back(1);
      }

      Value padOp = createConstOpForMixConvert(b,loc,ins_type,newShape,func);

      //reshape [2,5]->[10]
      newShape[0]=e1;
      Value flattenOp = createReshapeOp(b,loc,insOp,newShape);
      flattenOp.dump();
      //concat (10,n)->[24]
      Value concatOp = createConcatOp(b,loc,padOp,flattenOp);
      //reshape [24]->[1,2,3,4]
      newInsOp = createReshapeOp(b,loc,concatOp,con_shape);
    }
  }
  else if (i>1){  //是输入形状的倍数,此时需要压缩shape  {48}
    if(i - (int)e1/e2==0){ //倍数是整数
      int index=-1;
      for(int i =0;i<ins_shape.size();i++){
        if(ins_shape[i]==(int)e1/e2*con_shape[i]) {
          index = i;
          break;
        }
      }
      if(index!=-1) {
        Value reduceOP= createSliceOp(b,loc,insOp,con_shape);
        newInsOp = createReshapeOp(b,loc,reduceOP,con_shape);
      }
      else{
        cout<<"unrealized: i>1"<<endl;
//        result_Info.second.clear();
//        result_Info.second.push_back(i);
//        result_Info.second.push_back(e2);
//
//        llvm::SmallVector<int64_t , 8> newShape;
//        Value reshapeOp = createReshapeOp(b,loc,insOp,newShape);
//        //    //reduce_sum [2,24]->[1,24]
//        //    ins_Info.second = result_Info.second;
//        //    insOp = newInsOp;
//        //    result_Info.second.erase(result_Info.second.begin());
//        //    result_Info.second.insert(result_Info.second.begin(),1,1);
//        //    newInsOp = createReduceSumOp(b,loc,insOp,ins_Info,result_Info.second);
//        //    //[1,24]->[1,2,3,4]
//        //    ins_Info.second = result_Info.second;
//        //    insOp = newInsOp;
//        //    newInsOp = createReshapeOp(b,loc,insOp,ins_Info,constraint_Info.second);
      }
    }
    else{
      int n = (ceil(i)*e2) - e1;
      //const
      llvm::SmallVector<int64_t , 8> newShape;
      newShape.push_back(n);
      Value constOp = createConstOpForMixConvert(b,loc,ins_type,newShape,func);
      //reshape
      //[1,3,9,2]->[54]
      newShape[0]=e2;
      Value reshapeOp1 = createReshapeOp(b,loc,conOp,newShape);  //flatten[1,3,9,2]->[54]
      //concat (54, 18(n) )->[72]
      Value concatOp = createConcatOp(b,loc,constOp,reshapeOp1);  //concat(constOp,flatten(insop))  concat(54,3)->57
      //reshape [72]->[3,24]
      newShape.clear();
      newShape.assign(con_shape.begin(),con_shape.end()) ;
      newShape.push_back(ceil(i));
      Value reshapeOp2 = createReshapeOp(b,loc,concatOp,newShape);
      //reduce_sum [3,24]->[1,24]
      Value reduceOp = createReduceOp(b,loc,reshapeOp2,newShape.size()-1);
      //reshape [1,24]->[1,2,3,4]
      newShape.pop_back();
      newInsOp = createReshapeOp(b,loc,reduceOp,newShape);
    }
  }

  return newInsOp;
}


Value MIXPass::Conversion(ImplicitLocOpBuilder b,Location loc,Value insOp,Value conOp,func::FuncOp f){
  Value SCOp;
  Value TCOp;
  if (insOp.getType().cast<ShapedType>().getShape()!=conOp.getType().cast<ShapedType>().getShape()){
    Value SCOp = shapeConversion(b,loc,insOp,conOp,f);
    insOp = SCOp;
  }

  if (insOp.getType().cast<ShapedType>().getElementType()!=conOp.getType().cast<ShapedType>().getElementType()){
    Value TCOp = createCastOp(b,loc,insOp,conOp);
    insOp = TCOp;
  }
  return insOp;
}


SmallVector<Value> MIXPass::insertTosaOp(ImplicitLocOpBuilder b,Location loc,Value conOp,func::FuncOp func, SmallVector<Value> valuePool){
//  llvm::ArrayRef<StringRef> opPool = { "concat","select","abs",   "bitwise_not", "ceil", "clz","exp",
//                                       "floor",   "log",  "logical_not", "reciprocal",
//                                       "rsqrt", "sigmoid",     "tanh", "argmax", "reduce_all", "reduce_any",  "reduce_max",
//                                       "reduce_min", "reduce_prod", "reduce_sum","reverse", "add",         "bitwise_and",
//                                       "bitwise_or",  "bitwise_xor",
//                                       "div",         "equal",
//                                       "greater",     "greater_equal",
//                                       "logical_and", "logical_left_shift",
//                                       "logical_or",  "logical_right_shift",
//                                       "logical_xor", "maximum",
//                                       "minimum",     "pow",
//                                       "sub","conv2d","conv3d","transpose_conv2d","depthwise_conv2d","avg_pool2d","max_pool2d"};


  llvm::ArrayRef<StringRef> opPool = {"concat","abs","bitwise_not", "ceil","clz","exp","floor","log","logical_not",
                                      "reciprocal","rsqrt","sigmoid","tanh","argmax","reduce_all","reduce_any","reduce_max","reduce_min","reduce_prod",
                                      "reduce_sum","reverse","add","bitwise_and","bitwise_or","bitwise_xor","div","greater_equal",
                                      "logical_and","logical_left_shift","logical_or","logical_right_shift","logical_xor","maximum","minimum","pow","sub","conv2d",
                                      "conv3d","depthwise_conv2d","avg_pool2d","max_pool2d","reshape","cast"};

  genUtils.analyseAllOp(opPool);

  SmallVector<Value> allInsOps = {};
  //插入op的条数
  int opNum = genUtils.genRandomN(3, 10);;
  Value newOp;
  for(int i=0 ; i < opNum ; i++) {
    string selectedOp = genUtils.getNextOp(conOp);
//    selectedOp = "concat";
    cout<<"selectedOp  "<<selectedOp<<endl;
    if(selectedOp==" ")
      return allInsOps;
//    selectedOp = "max_pool2d";
    infogen.initInfo(selectedOp);
    infogen.addInputType(b, conOp);
    //    info.inputs.push_back(conOp);
    Value v = genUtils.skipMatch(valuePool);
    if(v==nullptr){
      infogen.addInputs(b, loc, func, conOp);
    }else{
      info.inputs.push_back(conOp);
      info.inputs.push_back(v);
      info.inputType={};
      info.inputType.push_back(conOp.getType());
      info.inputType.push_back(v.getType());
    }

    infogen.addAttrs(b, loc);
    infogen.addResult(b);
    newOp =  create.createOp(b,loc);
    allInsOps.push_back(newOp);
    conOp=newOp;
  }

  return allInsOps;
}

Value MIXPass::createReshapeOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc, mlir::Value insOp,
                               llvm::SmallVector<int64_t , 8> newShape) {
  //填充info结构体
  string opname = "reshape";
  infogen.initInfo(opname);

  info.inputType.push_back(insOp.getType());
  info.inputs.push_back(insOp);

  //添加reshape的属性
  SmallVector<NamedAttribute> namedAttrs;
  namedAttrs.push_back(b.getNamedAttr("new_shape",b.getDenseI64ArrayAttr(newShape)));
  info.attrs = namedAttrs;
  infogen.addResult(b);

  //创建op
  Value reshapeOp = create.createOpWithOneAttr(b, loc);
  return reshapeOp;
}

Value MIXPass::createConstOp(ImplicitLocOpBuilder b, Location loc, string et, llvm::SmallVector<int64_t , 8> concatShape,func::FuncOp funcOp){
  Value constOp;
  if (genUtils.getElementNum(concatShape) > 10000){
    string opname = "log";
    infogen.initInfo(opname);
    llvm::SmallVector<Type , 8> args;
    args.push_back(genUtils.genTensorType(b,concatShape,et));
    create.insertFuncArg(b,funcOp,args); //插入index
    infogen.addInputs(b,loc,funcOp);
    constOp = info.inputs[0];
  }
  else{
    DenseIntOrFPElementsAttr inputAttr = genUtils.getDenseAttr(b,et,concatShape);
    constOp = b.create<tosa::ConstOp>(loc, inputAttr.getType(), inputAttr);
  }
  return constOp;
}

Value MIXPass::createConcatOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc, mlir::Value insOp,
                              Value constOp){
  //填充info结构体
  string opname = "concat";
  infogen.initInfo(opname);

  info.inputType.push_back(insOp.getType());
  info.inputType.push_back(constOp.getType());
  info.inputs.push_back(insOp);
  info.inputs.push_back(constOp);

  infogen.addAttrs(b, loc);
//  SmallVector<NamedAttribute> namedAttrs;
//  namedAttrs.push_back(b.getNamedAttr("axis", b.getI64IntegerAttr(0)));
//  info.attrs = namedAttrs;
  infogen.addResult(b);

  Value concatOp = create.createOpWithOneAttr(b, loc);
  return concatOp;
}

Value MIXPass::createConstOpForMixConvert(ImplicitLocOpBuilder b, Location loc,string et,  llvm::SmallVector<int64_t , 8> inputShape,func::FuncOp funcOp){

  Value constOp;
  if (genUtils.getElementNum(inputShape) > 10000){
    string opname = "log";
    infogen.initInfo(opname);
    llvm::SmallVector<Type , 8> args;
    args.push_back(genUtils.genTensorType(b,inputShape,et));
    create.insertFuncArg(b,funcOp,args); //插入index
    infogen.addInputs(b,loc,funcOp);
    constOp = info.inputs[0];
  }
  else {

    DenseIntOrFPElementsAttr inputAttr =
        genUtils.getDenseAttr(b, et, inputShape);
    constOp =
        b.create<tosa::ConstOp>(loc, inputAttr.getType(), inputAttr);
  }
  return constOp;
}

Value MIXPass::createCastOp(ImplicitLocOpBuilder b,Location loc,Value insOp,Value conOp){
  //填充info结构体
  string opname = "cast";
  infogen.initInfo(opname);

  info.inputType.push_back(insOp.getType());
  info.inputs.push_back(insOp);
  info.resultType = conOp.getType();
  //创建op
  Value castOp = create.createOpWithNoAttr(b, loc);
  return castOp;
}

Value MIXPass::createReduceOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc, mlir::Value insOp,
                              int axis){
  //填充info结构体
  string opname = "reduce_sum";
  infogen.initInfo(opname);

  info.inputType.push_back(insOp.getType());
  info.inputs.push_back(insOp);

  SmallVector<NamedAttribute> namedAttrs;
  namedAttrs.push_back(b.getNamedAttr("axis", b.getI64IntegerAttr(axis)));
  info.attrs = namedAttrs;
  infogen.addResult(b);

  Value concatOp = create.createOpWithOneAttr(b, loc);
  return concatOp;
}


Value MIXPass::createSliceOp(mlir::ImplicitLocOpBuilder b, mlir::Location loc, mlir::Value insOp,
                             llvm::SmallVector<int64_t , 8> con_shape){
  //填充info结构体
  string opname = "slice";
  infogen.initInfo(opname);

  info.inputType.push_back(insOp.getType());
  info.inputs.push_back(insOp);
  llvm::SmallVector<int64_t , 8> startVec;
  for(auto x:con_shape){
    startVec.push_back(0);
  }

  SmallVector<NamedAttribute> namedAttrs;
  namedAttrs.push_back(b.getNamedAttr("start",b.getDenseI64ArrayAttr(startVec)));
  namedAttrs.push_back(b.getNamedAttr("size",b.getDenseI64ArrayAttr(con_shape)));
  info.attrs = namedAttrs;

  infogen.addResult(b);

  //创建op
  Value sliceOp = create.createOpWithMulAttrs(b, loc);
  return sliceOp;
}

namespace mlir {
void registerMixMutatePass() { PassRegistration<MIXPass>(); }
} // namespace mlir
