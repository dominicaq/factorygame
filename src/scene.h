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
struct SceneData {
    // Meta data
    std::string name;

    // Transform
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 eulerAngles = glm::vec3(0.0f);
};

class Scene {
public:
    entt::registry registry;
    std::vector<Mesh*> instanceMeshes;

    // TODO: TEMP
    void createAsteroids(int n, float circleRadius, float yPosition, Shader* basicShader);
    void createLights(int n, float circleRadius, float yPosition, Shader* basicShader);
    // END OF TEMP

    // Scene management
    void loadScene();
    Camera& getPrimaryCamera();
    void setPrimaryCamera(entt::entity cameraEntity);
private:
    // Component Helpers
    GameObject* addGameObjectComponent(entt::registry& registry, entt::entity entity, const SceneData& data);
    void addLightComponents(entt::registry& registry, entt::entity entity, Light lightData);
    void addPointLightComponents(entt::registry& registry, entt::entity entity, Light lightData);

    entt::entity m_primaryCameraEntity = entt::null;
};

#endif // SCENE_H
