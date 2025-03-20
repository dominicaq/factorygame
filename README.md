# Factory Game

This project is an OpenGL deferred renderer with plans to become a game engine, currently in development.

<div style="text-align: center;">
    <img src="./resources/demo.gif" alt="Demo" style="display: block; margin: 0 auto; border: none;">
</div>


## Features
- **Deferred Rendering** – Uses G-buffer separation for Position, Normal, and Color channels.
- **ECS Architecture** – The rendering pipeline is built on an ECS system, ensuring efficient, on-demand updates only when necessary.
- **Cube Maps** – Utilized in `SkyboxPass` and `ShadowPass` (point lights). *Note: The engine doesn't support compute shaders (yet), so cube maps require 6 draw calls per scene.*
- **Shadow Atlas (WIP)** – Spotlights render to a shadow atlas, which can be resized as needed. *Default resolution: 8K.*
- **Simple Frame Graph** – Each render pass inherits from `RenderPass` for compilation, resource management, and execution.
- **Script Interface** – Allows each `GameObject` to run its own custom scripts. The `Script` class provides an interface for defining custom logic for each object, with lifecycle functions like `start()`, `update()`, and `onDestroy()` that can be implemented per object.
- **GameObject Parent-Child Hierarchy** – Supports a parent-child structure for `GameObject` transformations, where child objects inherit transformations from their parent objects.
- **Debug Gizmos** - Draw wireframe gizmos for visual debugging

## Installation
```sh
git clone git@github.com:dominicaq/factorygame.git
```
## Downloading Additional Assets (Optional)
If you wish to download the additional 3D model assets along with the project, run:
```sh
git submodule update --init --recursive
```
## Build Instructions
1. Linux:
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
- A PC with OpenGL support (minimum OpenGL version 4.3)
- CMake 3.10 or higher
- A C++ compiler supporting C++17 or higher
