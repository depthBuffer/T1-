cmake_minimum_required(VERSION 3.15.0)

set(CMAKE_CXX_STANDARD 14)

set(ALL_INCS ${ALL_INCS} ${PROJECT_SOURCE_DIR}/)

add_library(t1_engine
  camera/camera_frustum.cpp
  camera/camera.cpp
  camera/fly_camera.cpp
  glad/glad.c
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui.cpp
  mesh/circle.cpp
  mesh/cube.cpp
  mesh/mesh.cpp
  mesh/plane.cpp
  mesh/quad.cpp
  mesh/sphere.cpp
  mesh/torus.cpp
  renderer/command_buffer.cpp
  renderer/gl_cache.cpp
  renderer/MaterialLibrary.cpp
  renderer/pbr_capture.cpp
  renderer/PostProcessor.cpp
  renderer/PBR.cpp
  renderer/render_target.cpp
  renderer/renderer.cpp
  resources/mesh_loader.cpp
  resources/resources.cpp
  resources/shader_loader.cpp
  resources/texture_loader.cpp
  stb/stb_image.cpp
  scene/background.cpp
  scene/scene_node.cpp
  scene/scene.cpp
  shading/material.cpp
  shading/shader.cpp
  shading/texture_cube.cpp
  shading/Texture.cpp
  stb/stb_image.cpp
  t1_engine.cpp
)

target_include_directories(t1_engine PUBLIC ${ALL_INCS})