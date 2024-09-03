cmake_minimum_required(VERSION 3.10)

project(FactoryGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories for GLAD and GLFW
include_directories(${CMAKE_SOURCE_DIR}/external/glad/include)
include_directories(${CMAKE_SOURCE_DIR}/external/glfw-3.4/include)

find_package(OpenGL REQUIRED)

# Platform-specific settings
if (APPLE)
    set(OpenGL_GL_PREFERENCE LEGACY)
endif()

# Add your source files
add_executable(${PROJECT_NAME} src/main.cpp)

# Add GLAD source file
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c)

# Specify GLFW library path
set(GLFW_LIB_PATH ${CMAKE_SOURCE_DIR}/external/glfw-3.4/bin)
set(GLFW_LIB_NAME glfw.3)

# Add GLFW as a dynamic library
find_library(GLFW_LIBRARY NAMES ${GLFW_LIB_NAME} PATHS ${GLFW_LIB_PATH})

# Link GLAD, GLFW, and OpenGL
target_link_libraries(${PROJECT_NAME} glad ${GLFW_LIBRARY} OpenGL::GL ${EXTRA_LIBS})
