# Exclude tests from libMLIR.so
add_mlir_library(MLIRTestPass
  TestDynamicPipeline.cpp
  TestPassManager.cpp
        AttrGenPass.cpp
        TrueGuard.cpp
        Substitution.cpp
        BogusCF.cpp
        NodeRemoval.cpp
        NodeCopy.cpp
        GetDialectName.cpp
        MIX.cpp
        NodeSwith.cpp
        tosaGen.cpp
        tosaGenC.cpp
        tosaGenU.cpp


  EXCLUDE_FROM_LIBMLIR

  ADDITIONAL_HEADER_DIRS
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/Pass

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRPass
  MLIRTestDialect
  )

target_include_directories(MLIRTestPass
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../Dialect/Test
  ${CMAKE_CURRENT_BINARY_DIR}/../Dialect/Test
  )

#target_include_directories(MLIRTestPass PUBLIC /usr/local/include/json)
target_link_libraries(MLIRTestPass PUBLIC /usr/local/lib/libjsoncpp.a TosaGenUtils)

#target_include_directories(MLIRTestPass PUBLIC /home/tongweiyuan/mlir/json)
# target_link_libraries(MLIRTestPass PUBLIC /home/tongweiyuan/mlir/libjsoncpp.a TosaGenUtils)


#target_link_libraries(MLIRTestPass PUBLIC TosaGenUtils)