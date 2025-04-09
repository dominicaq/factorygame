# Factory Game

This project is an OpenGL deferred renderer with plans to become a game engine, currently in development.

<div style="text-align: center;">
    <img src="./resources/demo.gif" alt="Demo" style="display: block; margin: 0 auto; border: none;">
</div>

## ✨ Features

- **Deferred Rendering**
  Uses G-buffer separation for Position, Normal, and Color channels.

- **ECS Architecture**
  The rendering pipeline is built on an ECS system, ensuring efficient, on-demand updates only when necessary.

- **Cube Maps**
  Utilized in `SkyboxPass` and `ShadowPass` (point lights).

- **Simple Frame Graph**
  Each render pass inherits from `RenderPass` for compilation, resource management, and execution.

- **Script Interface**
  Allows each `GameObject` to run its own custom scripts. The `Script` class provides an interface for defining custom logic per object, with lifecycle functions like `start()`, `update()`, and `onDestroy()`.

- **GameObject Parent-Child Hierarchy**
  Supports parent-child structures for `GameObject` transformations, where children inherit transformations from their parents.

- **Debug Gizmos**
  Draw wireframe gizmos for visual debugging.

### 🎥 Light Rendering Demonstration

<p align="center">
  <strong>Demonstration of 1,000 and 10,000 non-shadow casting light sources</strong><br><br>
  <video src="https://github.com/user-attachments/assets/d83abfd2-544a-47c7-b1c7-16d012311dcd" controls style="max-width: 100%; height: auto;"></video>
</p>

> If the video doesn't load, [click here to view it directly](https://github.com/user-attachments/assets/d83abfd2-544a-47c7-b1c7-16d012311dcd).

---

# Engine Specifics

This section outlines key implementation details and limitations of the engine.

## 🧩 Components

The engine uses an **Entity-Component-System (ECS)** architecture powered by the [EnTT](https://github.com/skypjack/entt) library. The following core components are currently supported:

### Core Components:

- **GameObject**
  A thin abstraction layer over the ECS system, simplifying script development. Users can interact with "objects" similar to Unity, abstracting away direct entity management.

- **Position**
  Stores the 3D world position of an entity (`glm::vec3`).

- **Rotation**
  Stores the 3D rotation of an entity using a quaternion (`glm::quat`). Modifying this component directly updates the `EulerAngles` component and vice versa.

- **Scale**
  Stores the 3D scale of an entity (`glm::vec3`).

- **EulerAngles**
  Stores the 3D rotation of an entity using Euler angles (in degrees) (`glm::vec3`).
  *Note: While the engine primarily uses quaternions for internal calculations, Euler angles are maintained for potential use cases and editor integration.*

- **ModelMatrix**
  Flags the entity's model matrix as dirty, triggering recalculation.

- **MetaData**
  Stores basic information about the entity, such as its name (`std::string`).

- **Parent**
  Indicates the parent entity in a hierarchical structure.

- **Children**
  Stores a list of child entities.

- **Light**
  Stores metadata of a light, which the engine processes into Shader Storage Buffer Objects (SSBOs) for rendering.

- **Mesh**
  Stores all required data for a single mesh, including vertices, indices, normals, tangents, and bitangents.

---

## 🎮 Rendering

The engine supports a variety of rendering features:

- **Physically Based Rendering (PBR) Lighting Model**
  Implements a PBR lighting model for realistic material rendering.

- **Point and Spot Lights**
  Supports point, spot, and directional lights.

- **Mesh Instancing**
  Optionally assign a `MeshInstance` component to a `GameObject`, which simply stores an ID.
  The renderer will group and render all instances with the same ID in a single draw call, applying their respective transformation matrices.
  To manage this efficiently, use the helper function `addMeshInstance(Mesh* mesh)` in `scene.cpp`.

- **Optional Shadow Casting**
  Point, spot, and directional lights can be configured to cast shadows.

- **Optional Parallax Occlusion Mapping**
  If a `Mesh` `Material` has a `heightMap` texture, the engine will apply this feature.

---

## ⚠️ Limitations

- **Maximum Lights:**
  The engine currently supports up to **1000 lights** to maintain performance. This limit can be adjusted in the codebase if needed.

- **Maximum Shadow Casters:**
  The number of simultaneous shadow casters is limited to **20**, primarily due to hardware constraints.

---

### Miscellaneous

- **Forward Direction**
  The local forward direction of entities is defined as the **negative Z-axis (-Z)**.

---

## Installation

To clone the repository, run the following command:

```sh
git clone git@github.com:dominicaq/factorygame.git
```

---

## Downloading Additional Assets (Optional)

If you wish to download the additional 3D model assets along with the project, run:

```sh
git submodule update --init --recursive
```

---

## Build Instructions

### 1. Linux:
```sh
mkdir build
cd build
cmake ..
make
```

### 2. Windows:
```sh
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The compiled executable will be located in the `build/Release/FactoryGame.exe` directory on Windows.

---

## Dependencies

- A PC with OpenGL support (minimum OpenGL version 4.3)
- CMake 3.10 or higher
- A C++ compiler supporting C++17 or higher
