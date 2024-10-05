#ifndef ENGINE_H
#define ENGINE_H

// System
#include "system/window.h"
#include "system/input_manager.h"
#include "globals.h"

// ECS Components
#include "components/ecs/ecs.h"
#include "components/camera.h"
#include "components/mesh.h"
#include "components/transform.h"

#include "components/light.h"
#include "components/gameobject.h"
#include "components/gameobjectmanager.h"

#include "components/script.h"

// Resources
#include "resources/meshgen.h"
#include "resources/resource_loader.h"

// Renderer
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/renderer.h"

#endif // ENGINE_H
