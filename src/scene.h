#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <entt/entt.hpp>
#include "engine.h"

// Screen dimensions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// File paths (assets dir is parent)
static const std::string SHADER_DIR = ASSET_DIR "shaders/";
static const std::string MODEL_DIR = ASSET_DIR "models/";
static const std::string TEXTURE_DIR = ASSET_DIR "textures/";

struct Scene {
    entt::entity primaryCameraEntity = entt::null;
    std::vector<entt::entity> cameraEntities;
    std::vector<entt::entity> lightEntities;
    std::vector<entt::entity> gameEntities;
};

// Meta data is the data saved to disk
struct MetaData {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 eulerAngles = glm::vec3(0.0f);
};

// Functions for Scene Management
void loadScene(Scene& scene, entt::registry& registry);
void setPrimaryCamera(Scene& scene, entt::entity cameraEntity);
Camera& getPrimaryCamera(const Scene& scene, entt::registry& registry);

// Component Helpers
GameObject* addGameObjectComponent(entt::registry& registry, entt::entity entity, const MetaData& data);

#endif // SCENE_H
