add_mlir_library(TosaGenUtils
        create.cpp
        opinfo.cpp
        utils.cpp
        transfer.cpp
)

#target_include_directories(TosaGenUtils PUBLIC /usr/local/include/json)
#target_link_libraries(TosaGenUtils PUBLIC /usr/local/lib/libjsoncpp.a)

#target_include_directories(TosaGenUtils PUBLIC /home/tongweiyuan/mlir/json)
#target_link_libraries(TosaGenUtils PUBLIC /home/tongweiyuan/mlir/libjsoncpp.a)