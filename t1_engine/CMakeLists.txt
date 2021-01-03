cmake_minimum_required(VERSION 3.15.0)

set(CMAKE_CXX_STANDARD 14)

set(ALL_INCS ${ALL_INCS} ${PROJECT_SOURCE_DIR}/)

add_library(t1_engine
  glad/glad.c
  resources/shader_loader.cpp
  shading/shader.cpp
  t1_engine.cpp
)

target_include_directories(t1_engine PUBLIC ${ALL_INCS})