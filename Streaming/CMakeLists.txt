cmake_minimum_required(VERSION 3.22)
project(Experiment)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} main.cpp)
target_sources(${PROJECT_NAME} PRIVATE structs.h input_handler.h baseline.h baseline_ext.h miner.h result.h)

target_sources(${PROJECT_NAME} PRIVATE Offline/query.h)
target_sources(${PROJECT_NAME} PRIVATE Offline/cluster.h)
target_sources(${PROJECT_NAME} PRIVATE Offline/mining_sw.h)
target_sources(${PROJECT_NAME} PRIVATE Offline/position.h)
target_sources(${PROJECT_NAME} PRIVATE Offline/result.h)
target_sources(${PROJECT_NAME} PRIVATE Offline/tree.h)



