cmake_minimum_required(VERSION 3.10)

project(FactoryGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include external directories
include_directories(${CMAKE_SOURCE_DIR}/external/glad/include)
include_directories(${CMAKE_SOURCE_DIR}/external/glfw-3.4/include)
include_directories(${CMAKE_SOURCE_DIR}/external/glm)
include_directories(${CMAKE_SOURCE_DIR}/external/stb)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Platform-specific settings
if (APPLE)
    set(OpenGL_GL_PREFERENCE LEGACY)
endif()

# Find all .cpp files in src/ and its subdirectories
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Define asset directory as an absolute path
set(ASSET_DIR "${CMAKE_SOURCE_DIR}/assets/")
add_definitions(-DASSET_DIR="${ASSET_DIR}")

# Add GLAD source file
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c)

# Find and link GLFW library
set(GLFW_LIB_PATH ${CMAKE_SOURCE_DIR}/external/glfw-3.4/bin)
set(GLFW_LIB_NAME glfw.3)
find_library(GLFW_LIBRARY NAMES ${GLFW_LIB_NAME} PATHS ${GLFW_LIB_PATH})
if (NOT GLFW_LIBRARY)
    message(FATAL_ERROR "Failed to find GLFW library.")
endif()

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link GLAD, GLFW, and OpenGL
target_link_libraries(${PROJECT_NAME} glad ${GLFW_LIBRARY} OpenGL::GL)
