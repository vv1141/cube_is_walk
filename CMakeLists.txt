cmake_minimum_required(VERSION 3.11)

project(ecs_demo)

SET(CMAKE_CXX_FLAGS "-O3 -Werror -std=c++0x -pthread")

# ---------------------------------------------------------------------------------------------------
# Set output directories
# ---------------------------------------------------------------------------------------------------

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---------------------------------------------------------------------------------------------------
# Define sources and executable
# ---------------------------------------------------------------------------------------------------

set(EXECUTABLE_NAME "ecs_demo")
add_executable(
  ${EXECUTABLE_NAME}
  cpp/main.cpp
  c/flecs.c
  cpp/Object.cpp
  cpp/Player.cpp
  cpp/Camera.cpp
  cpp/RenderObject.cpp
  cpp/Shader.cpp
  cpp/Model.cpp
  cpp/Texture.cpp
  cpp/Light.cpp
  cpp/World.cpp
  cpp/Polygon.cpp
  cpp/Utility.cpp
  cpp/Debug.cpp
  cpp/Input.cpp
  cpp/Keybind.cpp
)

# ---------------------------------------------------------------------------------------------------
# Find and add OpenGL and GLEW
# ---------------------------------------------------------------------------------------------------

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARY} ${GLEW_LIBRARIES})

# ---------------------------------------------------------------------------------------------------
# Find and add SFML
# ---------------------------------------------------------------------------------------------------

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
#Find any version 2.X of SFML
#See the FindSFML.cmake file for additional details and instructions
find_package(SFML 2 REQUIRED graphics window system)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
endif()

# ---------------------------------------------------------------------------------------------------
# Include additional libraries
# ---------------------------------------------------------------------------------------------------

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/include)
set_property(SOURCE flecs.c APPEND PROPERTY COMPILE_OPTIONS -std=gnu99)

# ---------------------------------------------------------------------------------------------------
# Copy assets
# ---------------------------------------------------------------------------------------------------

add_custom_target(
  copy-resources ALL
  COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res
  DEPENDS ${EXECUTABLE_NAME}
  COMMENT "copying assets" VERBATIM
)
