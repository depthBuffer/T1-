cmake_minimum_required(VERSION 3.15.0)
project(T1)

set(CMAKE_CXX_STANDARD 14)

if(NOT (WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux"))
  message(FATAL_ERROR "T1 supports Windows and Linux only (requires an OpenGL 4.3 core context).")
endif()

if(MSVC)
  # glad.h includes <windows.h>; keep its min/max macros from breaking std::min/std::max.
  add_compile_definitions(NOMINMAX _USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS)
  add_compile_options(/MP)
endif()

find_package(OpenGL REQUIRED)

# ------------------------------------------------------------------------------------------------
# Third-party libraries: GLFW 3.2.1 and Assimp 3.3.1 are downloaded and built from source the first
# time CMake configures the project (into build/_deps/). Both are built as static libraries, so no
# DLLs or shared objects have to be shipped next to the demos.
# ------------------------------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.2.1
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG        v3.3.1
  GIT_SHALLOW    TRUE
)

# both projects predate CMake's support for setting their options with normal variables, so the
# options are set in the cache
set(BUILD_SHARED_LIBS          OFF CACHE BOOL "" FORCE)

set(GLFW_BUILD_EXAMPLES        OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS           OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS            OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL               OFF CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_TESTS         OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS  OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES       OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT           ON  CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw assimp)

# ------------------------------------------------------------------------------------------------
# T1
# ------------------------------------------------------------------------------------------------
add_subdirectory(t1_math)
add_subdirectory(t1_utility)
add_subdirectory(t1_engine)

# Assimp 3.3.1's target does not carry its include directory, so add it explicitly.
target_include_directories(t1_engine PUBLIC ${assimp_SOURCE_DIR}/include)
target_link_libraries(t1_engine PUBLIC t1_utility t1_math glfw assimp OpenGL::GL)

add_executable(demo
  demo/scenes/pbr_test.cpp
  demo/program.cpp
)

# second demo: the Foundry Yard environment
add_executable(foundry
  demo/foundry.cpp
)

set(T1_DEMOS demo foundry)

foreach(T1_DEMO ${T1_DEMOS})
  target_include_directories(${T1_DEMO} PUBLIC ${PROJECT_SOURCE_DIR}/)
  target_link_libraries(${T1_DEMO} t1_engine)

  # run from build/ (where shaders/, textures/ and meshes/ live) when debugging in Visual Studio
  set_property(TARGET ${T1_DEMO} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
endforeach()

set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT demo)
