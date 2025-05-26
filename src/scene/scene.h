#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <entt/entt.hpp>

// Scene includes
#include "scene_utils.h"
#include "octree.h"

#include "engine.h"

// Screen dimensions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// File paths (assets dir is parent)
static const std::string SHADER_DIR = ASSET_DIR "shaders/";
static const std::string MODEL_DIR = ASSET_DIR "models/";
static const std::string TEXTURE_DIR = ASSET_DIR "textures/";

class Scene {
public:
    entt::registry registry;
    std::vector<EntityMeshDefinition> meshEntityPairs;
    std::vector<InstancedMeshGroup> instancedMeshGroups;

    // =========================================================================
    // Scene Management
    // =========================================================================
    /*
     * Loads the scene, initializing all necessary components and entities.
     */
    void loadScene();

    /*
     * Retrieves the primary camera associated with the scene.
     * @return A reference to the primary Camera.
     */
    Camera& getPrimaryCamera();

    /*
     * Sets the primary camera for the scene.
     * @param cameraEntity - The entity representing the primary camera to set.
     */
    void setPrimaryCamera(entt::entity cameraEntity);

    // =========================================================================
    // SKybox Management
    // =========================================================================
    unsigned int getSkyBox() const { return m_skyboxHandle; }

    void loadSkyBox(const std::vector<std::string>& skyboxFilePaths);

    // =========================================================================
    // TODO: TEMP, PENDING REMOVAL
    // =========================================================================
    void createSpheres(int n, float fieldSize, float minHeight, float maxHeight, std::string vertexPath, std::string fragPath, bool litSpheres);
    void createSpotLights(int n, float circleRadius, float yPosition, std::string vertexPath, std::string fragPath);
    void createSuns(int n, float circleRadius, float yPosition, std::string vertexPath, std::string fragPath);

private:
    unsigned int m_skyboxHandle;

    // Scene Management
    std::unique_ptr<Octree<entt::entity>> m_octree;
    entt::entity m_primaryCameraEntity = entt::null;
};

#endif // SCENE_H
