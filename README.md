# Factory Game

This project is an OpenGL deferred renderer with plans to become a game engine, currently in development.

<div style="text-align: center;">
    <img src="./resources/demo.gif" alt="Demo" style="display: block; margin: 0 auto; border: none;">
</div>


## Features
- **Cube Maps**
    - Used for SkyboxPass and ShadowPass (point lights)
    - **Note**: Not using compute shaders, cube maps are x6 draw calls for the scene
- **Deferred Rendering**:
    - G-buffers: Position, Normal, and Color.
- **Shadow Atlas (WIP)**:
    - Spot lights will draw to a shadow atlas for sampling within the light pass. Shadow atlas can be any size.
    - **Note** By default, shadow atlas is 8k resolution
- **Render Graph (WIP)**
    - Every render pass inherits RenderPass for compilation, resource gathering, and running.

## Installation
```sh
git clone git@github.com:dominicaq/factorygame.git
```

## Build Instructions

1. MacOS/Linux:
    ```sh
    mkdir build
    cd build
    cmake ..
    make
    ```
2. Windows:
    ```sh
    mkdir build
    cd build
    cmake ..
    cmake --build . --config Release
    ```
The compiled executable will be located in the `build/Release/FactoryGame.exe` directory on Windows.

## Dependencies

- A PC with OpenGL support (minimum OpenGL version 3.3)
- CMake 3.10 or higher
- A C++ compiler supporting C++17 or higher
