cmake_minimum_required(VERSION 3.15.0)

set(CMAKE_CXX_STANDARD 14)

add_library(t1_utility logging/log.cpp random/random.cpp timing/time.cpp ${PROJECT_SOURCE_DIR}/t1_math/common.h)

include_directories(${PROJECT_SOURCE_DIR})