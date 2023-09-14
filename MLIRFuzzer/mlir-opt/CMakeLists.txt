set(LLVM_OPTIONAL_SOURCES
  null.cpp
)

get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)
set(LLVM_LINK_COMPONENTS
  Core
  Support
  AsmParser
  )

if(MLIR_INCLUDE_TESTS)
  set(test_libs
    MLIRTestFuncToLLVM
    MLIRAffineTransformsTestPasses
    MLIRArithTestPasses
    MLIRBufferizationTestPasses
    MLIRControlFlowTestPasses
    MLIRDLTITestPasses
    MLIRFuncTestPasses
    MLIRGPUTestPasses
    MLIRLinalgTestPasses
    MLIRMathTestPasses
    MLIRMemRefTestPasses
    MLIRNVGPUTestPasses
    MLIRSCFTestPasses
    MLIRShapeTestPasses
    MLIRSPIRVTestPasses
    MLIRTensorTestPasses
    MLIRTestAnalysis
    MLIRTestDialect
    MLIRTestDynDialect
    MLIRTestIR
    MLIRTestPass
    MLIRTestPDLL
    MLIRTestReducer
    MLIRTestRewrite
    MLIRTestTransformDialect
    MLIRTestTransforms
    MLIRTilingInterfaceTestPasses
    MLIRVectorTestPasses
    MLIRLLVMTestPasses
    )
endif()

set(LIBS
  ${dialect_libs}
  ${conversion_libs}
  ${test_libs}
  MLIRAffineAnalysis
  MLIRAnalysis
  MLIRDialect
  MLIROptLib
  MLIRParser
  MLIRPass
  MLIRTransforms
  MLIRTransformUtils
  MLIRSupport
  MLIRIR
  MLIRFuzzer
  )

# Exclude from libMLIR.so because this has static options intended for
# opt-like tools only.
add_llvm_executable(mlirfuzzer-opt mlir-opt.cpp)

llvm_update_compile_flags(mlirfuzzer-opt)
target_link_libraries(mlirfuzzer-opt PRIVATE ${LIBS})

mlir_check_all_link_libraries(mlirfuzzer-opt)

