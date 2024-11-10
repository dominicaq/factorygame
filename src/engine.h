#ifndef ENGINE_H
#define ENGINE_H

// System
#include "system/window.h"
#include "system/input_manager.h"
#include "globals.h"

// ECS Components
#include "entt/entt.hpp"
#include "components/camera.h"
#include "components/mesh.h"
#include "components/light.h"
#include "components/gameobject.h"

#include "components/script.h"

#include "components/systems/gameobject_system.h"
#include "components/systems/transform_system.h"

// Resources
#include "resources/meshgen.h"
#include "resources/resource_loader.h"

// Renderer
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/renderer.h"

#include "scene.h"

#endif // ENGINE_H
