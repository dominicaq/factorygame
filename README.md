# OpenGL Deferred Renderer (WIP)

This project is an OpenGL deferred renderer with plans to become a game engine, currently in development. It features a simple entity-component system (ECS) that is also a work in progress.

<div style="text-align: center;">
    <img src="./resources/demo.gif" alt="Demo" style="display: block; margin: 0 auto; border: none;">
</div>


## Features

- **Deferred Rendering**: 
    - G-buffers: Position, Normal, and Color.
- **Skybox**: 
    - Rendering of environment skybox.
- **Entity-Component System (WIP)**:
    - Custom ECS for managing scene entities.

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
