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
using MeshEntityPair = std::pair<std::unique_ptr<RawMeshData>, entt::entity>;

class Scene {
public:
    entt::registry registry;
    std::vector<MeshEntityPair> meshEntityPairs;
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
    // Mesh Instancing
    // =========================================================================
    /*
     * Retrieves the instance map that stores mesh instances and their transformation matrices.
     * @return A reference to the unordered map of instance data.
     */
    const std::unordered_map<size_t, std::vector<glm::mat4>>& getInstanceMap() const { return m_instanceMap; }

    /*
     * Retrieves the list of mesh instances in the scene.
     * @return A reference to the vector of mesh pointers.
     */
    const std::vector<Mesh*>& getMeshInstances() const { return m_meshInstances; }

    /*
    * Tell the scene to recount the number of active instances to keep instance map up to date
    */
    void flagDirtyInstanceCount() { m_instanceCountsDirty = true; };

    /*
     * Adds a mesh to be instanced and returns the associated MeshInstance component.
     * @param mesh - The mesh to be instanced.
     * @return The MeshInstance component created for the mesh.
     */
    MeshInstance addMeshInstance(Mesh* mesh);

    // =========================================================================
    // TODO: TEMP, PENDING REMOVAL
    // =========================================================================
    void createSpheres(int n, float fieldSize, float minHeight, float maxHeight, Shader* basicShader, bool litSpheres);
    void createSpotLights(int n, float circleRadius, float yPosition, Shader* basicShader);
    void createSuns(int n, float circleRadius, float yPosition, Shader* basicShader);

private:
    // Scene Instancing
    std::unordered_map<size_t, std::vector<glm::mat4>> m_instanceMap;
    std::vector<Mesh*> m_meshInstances;
    std::unordered_map<size_t, size_t> m_instanceCounts;
    std::unordered_map<size_t, size_t> m_currentIndices;
    unsigned int m_skyboxHandle;
    bool m_instanceCountsDirty = true;

    // Misc
    Material* m_wireframeMaterial;

    // Scene Management
    std::unique_ptr<Octree<entt::entity>> m_octree;
    entt::entity m_primaryCameraEntity = entt::null;
};

#endif // SCENE_H
