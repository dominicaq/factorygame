cmake_minimum_required(VERSION 3.10)

project(FactoryGame)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(WIN32 AND MSVC)
    set(CMAKE_C_RUNTIME_LIBRARY "MultiThreaded")
    set(CMAKE_CXX_RUNTIME_LIBRARY "MultiThreaded")
endif()

# User defined script directory
include_directories(
    ${CMAKE_SOURCE_DIR}/assets/scripts
)

# Include src directory for engine
include_directories(
    ${CMAKE_SOURCE_DIR}/src
)

# Include external directories
include_directories(
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/implot
    ${CMAKE_SOURCE_DIR}/external/
)

# IMGUI and ImPlot source files
set(IMGUI_SRC
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/external/implot/implot.cpp
    ${CMAKE_SOURCE_DIR}/external/implot/implot_items.cpp
)

# Find OpenGL
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL found.")

# Set OpenGL preference for Apple
if(APPLE)
    set(OpenGL_GL_PREFERENCE LEGACY)
endif()

# Set up GLFW
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
file(GLOB_RECURSE SCRIPT_SOURCES "${CMAKE_SOURCE_DIR}/assets/scripts/*.cpp")

# Define asset directory as an absolute path
set(ASSET_DIR "${CMAKE_SOURCE_DIR}/assets/")
add_definitions(-DASSET_DIR="${ASSET_DIR}")

# Add GLAD source file
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${SCRIPT_SOURCES} ${IMGUI_SRC})

# Link against libraries
target_link_libraries(${PROJECT_NAME} glad OpenGL::GL)
if(WIN32 AND MSVC)
    target_link_libraries(${PROJECT_NAME} "${GLFW_LIB_DIR}/glfw3_mt.lib")
else()
    target_link_libraries(${PROJECT_NAME} "${GLFW_LIB_DIR}/libglfw.3.dylib")
endif()

if (APPLE)
    # Silence deprecated warnings on macOS
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-declarations)
elseif (WIN32)
    # Silence deprecated warnings on Windows (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4996)
endif()
