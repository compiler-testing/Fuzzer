# Exclude tests from libMLIR.so
add_mlir_library(MLIRFuzzer
    GetDialectName.cpp
    MIX.cpp
    tosaGen.cpp
    tosaGenC.cpp
    tosaGenU.cpp

  EXCLUDE_FROM_LIBMLIR

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/TosaGen
  ${PROJECT_SOURCE_DIR}/include/json

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRPass
  MLIRTestDialect
  )

target_include_directories(MLIRFuzzer
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../Dialect/Test
  ${CMAKE_CURRENT_BINARY_DIR}/../Dialect/Test
  )

# target_link_libraries(MLIRFuzzer PUBLIC /usr/local/lib/libjsoncpp.a TosaGenUtils)
target_link_libraries(MLIRFuzzer PUBLIC /usr/local/lib/libjsoncpp.a FuzzerUtils)

