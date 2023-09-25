set(PRO_NAME "simple_server")

cmake_minimum_required(VERSION 3.0.0)

project(${PRO_NAME} VERSION 0.1.0 LANGUAGES C CXX)

file(GLOB SRC_ALL "src/*.cpp" "src/*.c")
file(GLOB LIB_ALL "lib/*.so")

include_directories(include)

add_executable(${PRO_NAME} ${SRC_ALL}
        include/inter_server.h
        util/Tvlog.h
        src/server_task.cpp
        include/server_task.h
        ../simple_chat/web_protocol.h
)

target_link_libraries(${PRO_NAME} PRIVATE ${LIB_ALL})




