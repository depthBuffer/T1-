cmake_minimum_required(VERSION 3.15.0)
project(T1)

set(CMAKE_CXX_STANDARD 14)

set(DEMO_INCS ${DEMO_INCS} ${PROJECT_SOURCE_DIR}/)
set(DEMO_INCS ${DEMO_INCS} ${PROJECT_SOURCE_DIR}/libs/includes/)

set(T1_lib ${T1_lib} ${PROJECT_SOURCE_DIR}/libs/)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(t1_math)
add_subdirectory(t1_utility)
add_subdirectory(t1_engine)

link_directories(t1_math)
link_directories(t1_utility)
link_directories(t1_engine)

add_executable(demo
  demo/scenes/pbr_test.cpp
  demo/program.cpp
)

target_include_directories(demo PUBLIC ${DEMO_INCS})
target_link_libraries(demo t1_math t1_utility t1_engine ${T1_lib}libglfw.so.3.2 ${T1_lib}libassimp.so.3.3.1)