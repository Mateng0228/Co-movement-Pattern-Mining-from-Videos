cmake_minimum_required(VERSION 3.16.3)
project(Project)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_EXE_LINKER_FLAGS -static)
#add_compile_options(-O3)

add_executable(Project main.cpp utils.h query.h algorithm/position.h algorithm/cluster.h algorithm/tree.h algorithm/mining_base.h algorithm/mining_sw.h algorithm/result.h)
