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
void loadScene(Scene& scene, entt::registry& registry, LightSystem& lightSystem);
void setPrimaryCamera(Scene& scene, entt::entity cameraEntity);
Camera& getPrimaryCamera(const Scene& scene, entt::registry& registry);

// Component Helpers
bool hasTransformComponents(entt::registry& registry, entt::entity entity);
void addTransformComponents(entt::registry& registry, entt::entity entity,
    const glm::vec3& position = glm::vec3(0.0f),
    const glm::vec3& rotationEuler = glm::vec3(0.0f),
    const glm::vec3& scale = glm::vec3(1.0f));
GameObject* addGameObjectComponent(entt::registry& registry, entt::entity entity);

#endif // SCENE_H
