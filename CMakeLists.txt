cmake_minimum_required(VERSION 3.10)

project(FactoryGame)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(WIN32 AND MSVC)
    set(CMAKE_C_RUNTIME_LIBRARY "MultiThreaded")
    set(CMAKE_CXX_RUNTIME_LIBRARY "MultiThreaded")
endif()

# Include external directories
include_directories(${CMAKE_SOURCE_DIR}/external/glad/include)
include_directories(${CMAKE_SOURCE_DIR}/external/)

# Include src directory and assets/scripts directory recursively
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/assets/scripts)

# Find required packages
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL found.")

# Set OpenGL preference for Apple
if(APPLE)
    set(OpenGL_GL_PREFERENCE LEGACY)
endif()

# Set up GLFW and its libraries
set(GLFW_ROOT "${CMAKE_SOURCE_DIR}/external/glfw-3.4")
include_directories("${GLFW_ROOT}/include")

if(WIN32)
    set(GLFW_LIB_DIR "${GLFW_ROOT}/lib-vc2022")
elseif(APPLE)
    set(GLFW_LIB_DIR "${GLFW_ROOT}/lib-arm64")
endif()
link_directories("${GLFW_LIB_DIR}")

# Find all .cpp files in src/ and its subdirectories
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Recursively find all header and script files in assets/scripts
file(GLOB_RECURSE SCRIPT_HEADERS "${CMAKE_SOURCE_DIR}/assets/scripts/*.h")
file(GLOB_RECURSE SCRIPT_SOURCES "${CMAKE_SOURCE_DIR}/assets/scripts/*.cpp")

# Add the scripts to the executable sources
set(SOURCES ${SOURCES} ${SCRIPT_SOURCES})

# Define asset directory as an absolute path
set(ASSET_DIR "${CMAKE_SOURCE_DIR}/assets/")
add_definitions(-DASSET_DIR="${ASSET_DIR}")

# Add GLAD source file
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link against glad, glfw3 (for macOS), and OpenGL
target_link_libraries(${PROJECT_NAME} glad OpenGL::GL)
if(WIN32 AND MSVC)
    target_link_libraries(${PROJECT_NAME} "${GLFW_LIB_DIR}/glfw3_mt.lib")
else()
    target_link_libraries(${PROJECT_NAME} "${GLFW_LIB_DIR}/libglfw.3.dylib")
endif()

# Silence deprecated warnings from entt
target_compile_options(FactoryGame PRIVATE -Wno-deprecated-declarations)

# Define ASSET_DIR macro globally for source files
target_compile_definitions(${PROJECT_NAME} PRIVATE ASSET_DIR="${ASSET_DIR}")