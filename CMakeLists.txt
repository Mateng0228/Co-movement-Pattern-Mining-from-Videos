cmake_minimum_required(VERSION 3.23)
project(Project)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_EXE_LINKER_FLAGS -static)
#add_compile_options(-O3)

add_executable(Project main.cpp algorithm1/cmc_tree.h algorithm1/group.h utils.h algorithm1/cmc_mining.h algorithm1/cmc_query.h algorithm2/st_query.h algorithm2/position.h algorithm2/cluster.h algorithm2/tree.h algorithm2/st_mining1.h algorithm2/st_mining2.h result.h)
