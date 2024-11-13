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
#include "components/metadata.h"
#include "components/script.h"

#include "components/systems/gameobject_system.h"
#include "components/systems/transform_system.h"
#include "components/systems/light_system.h"

#include "components/resources.h"

// Resources
#include "resources/meshgen.h"
#include "resources/resource_loader.h"

// Renderer
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/renderer.h"

#endif // ENGINE_H
