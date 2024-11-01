#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <entt/entt.hpp>
#include "engine.h"

// Paths and screen dimensions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
static const std::string SHADER_DIR = ASSET_DIR "shaders/";
static const std::string MODEL_DIR = ASSET_DIR "models/";
static const std::string TEXTURE_DIR = ASSET_DIR "textures/";

struct Scene {
    entt::entity primaryCameraEntity = entt::null;
    std::vector<entt::entity> cameraEntities;
    std::vector<entt::entity> lightEntities;
    std::vector<entt::entity> gameEntities;
};

// Functions for Scene Management
void loadScene(Scene& scene, entt::registry& registry, LightSystem& lightSystem, GameObjectManager& gameObjectManager);
void setPrimaryCamera(Scene& scene, entt::entity cameraEntity);
Camera& getPrimaryCamera(const Scene& scene, entt::registry& registry);

#endif // SCENE_H
