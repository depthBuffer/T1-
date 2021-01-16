cmake_minimum_required(VERSION 3.15.0)

set(CMAKE_CXX_STANDARD 14)

set(ALL_INCS ${ALL_INCS} ${PROJECT_SOURCE_DIR}/)

add_library(t1_engine
  camera/camera.cpp
  camera/fly_camera.cpp
  glad/glad.c
  mesh/circle.cpp
  mesh/cube.cpp
  mesh/mesh.cpp
  mesh/plane.cpp
  mesh/quad.cpp
  mesh/sphere.cpp
  mesh/torus.cpp
  resources/shader_loader.cpp
  resources/texture_loader.cpp
  stb/stb_image.cpp
  shading/material.cpp
  shading/shader.cpp
  shading/texture_cube.cpp
  shading/Texture.cpp
  stb/stb_image.cpp
  t1_engine.cpp
)

target_include_directories(t1_engine PUBLIC ${ALL_INCS})