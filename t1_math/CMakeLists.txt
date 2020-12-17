cmake_minimum_required(VERSION 3.15.0)

set(CMAKE_CXX_STANDARD 14)

add_library(t1_math geometry/plane.cpp 
                    trigonometry/polar.cpp 
                    trigonometry/spherical.cpp)
                    
add_executable(t1_math_test program.cpp)