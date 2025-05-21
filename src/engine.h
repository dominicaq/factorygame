#pragma once

// System resources (input, window, etc)
#include "system/window.h"
#include "system/inputManager.h"
#include "globals.h"

// Engine Components
#include "entt/entt.hpp"
#include "components/camera.h"
#include "components/mesh.h"
#include "components/light.h"
#include "components/gameobject.h"
#include "components/metadata.h"
#include "components/script.h"

// Engine Systems
#include "components/systems/gameobjectSystem.h"
#include "components/systems/transformSystem.h"
#include "components/systems/lightSystem.h"

// Resource serializers / creators
#include "resources/meshgen.h"
#include "resources/resourceLoader.h"

// Renderer
#include "renderer/shader.h"
#include "renderer/computeShader.h"
#include "renderer/texture.h"
#include "renderer/renderer.h"

// Runtime debugging
#include "debugging/gizmos.h"
