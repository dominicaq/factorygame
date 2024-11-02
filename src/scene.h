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

// Meta data is the data saved to disk
struct MetaData {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 eulerAngles = glm::vec3(0.0f);
};

class Scene {
public:
    // Scene management
    void loadScene();
    Camera& getPrimaryCamera();
    void setPrimaryCamera(entt::entity cameraEntity);

    // Component Helpers
    GameObject* addGameObjectComponent(entt::registry& registry, entt::entity entity, const MetaData& data);

    entt::registry registry;
private:
    entt::entity m_primaryCameraEntity = entt::null;
};

#endif // SCENE_H
