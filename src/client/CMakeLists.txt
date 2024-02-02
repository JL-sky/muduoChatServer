#SRC_LIST变量定义了包含在该目录中的所有源文件
aux_source_directory(. SRC_LIST)

#使用该目录中的所有源文件生成可执行文件
add_executable(ChatClient ${SRC_LIST})
#将可执行文件与需要的库文件链接进行链接
target_link_libraries(ChatClient pthread)