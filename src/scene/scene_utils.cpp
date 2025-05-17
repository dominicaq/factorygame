#include "scene_utils.h"

void SceneUtils::addLightComponents(entt::registry& registry, entt::entity entity, Light lightData) {
    registry.emplace<Light>(entity, lightData);

    // Save memory footprint by not adding matrix component
    if (!lightData.castShadow) {
        return;
    }

    bool singleMatrix = lightData.type == LightType::Spot;
    if (singleMatrix) {
        registry.emplace<LightSpaceMatrix>(entity);
    } else {
        registry.emplace<LightSpaceMatrixArray>(entity);
    }
}

GameObject* SceneUtils::addGameObjectComponent(entt::registry& registry, entt::entity entity, const SceneData& data) {
    if (!registry.valid(entity)) {
        return nullptr;
    }

    // Meta data
    registry.emplace<MetaData>(entity, data.name);

    // Transform
    registry.emplace<Position>(entity, data.position);
    registry.emplace<EulerAngles>(entity, data.eulerAngles);
    registry.emplace<Rotation>(entity, glm::quat(glm::radians(data.eulerAngles)));
    registry.emplace<Scale>(entity, data.scale);
    registry.emplace<ModelMatrix>(entity);

    return &registry.emplace<GameObject>(entity, entity, registry);
}

GameObject* SceneUtils::createMeshGameObject(entt::registry& registry, Shader* shader, const std::string& filePath) {
    // Parse the mesh
    std::vector<SceneData> nodeData;
    std::vector<Mesh*> meshes;
    ResourceLoader::loadMeshVector(filePath, meshes, nodeData, shader);
    if (meshes.empty()) {
        return nullptr;
    }

    // Mesh bounding box
    glm::vec3 min, max;
    float maxSingleMeshDim;
    calculateMeshBounds(meshes, min, max, maxSingleMeshDim);

    // Calculate overall size (diagonal of the bounding box)
    float boundingBoxDiagonal = glm::length(max - min);

    // Constants for scaling decisions
    const float VERY_LARGE_THRESHOLD = 1000.0f;  // Models larger than this are "very large"
    const float LARGE_MODEL_ROOT_SCALE = 0.01f;  // Root scale to use for very large models
    const float SMALL_THRESHOLD = 0.01f;         // Models smaller than this are "small"
    const float TARGET_SIZE = 2.0f;              // Target size for small models

    float rootScale = 1.0f;

    // Decide on scaling approach based on model size
    if (boundingBoxDiagonal > VERY_LARGE_THRESHOLD) {
        rootScale = 1.0f / LARGE_MODEL_ROOT_SCALE;
    } else if (boundingBoxDiagonal < SMALL_THRESHOLD) {
        rootScale = TARGET_SIZE / boundingBoxDiagonal;
    }

    std::vector<entt::entity> entities(nodeData.size());
    std::vector<GameObject*> gameObjects(nodeData.size());
    std::vector<bool> meshAssigned(meshes.size(), false);

    // First pass: Create entities and assign meshes
    for (size_t i = 0; i < nodeData.size(); ++i) {
        entities[i] = registry.create();
        gameObjects[i] = SceneUtils::addGameObjectComponent(registry, entities[i], nodeData[i]);

        // If this node has a mesh reference, use the meshIndex to connect to the right mesh
        int meshIndex = nodeData[i].meshIndex;
        if (meshIndex >= 0 && meshIndex < meshes.size() && meshes[meshIndex] != nullptr) {
            registry.emplace<Mesh*>(entities[i], meshes[meshIndex]);

            // Mark this mesh as assigned
            meshAssigned[meshIndex] = true;
        }
    }

    // Second pass: Set up the hierarchy as defined by the file
    std::unordered_set<size_t> childrenSet;
    for (size_t i = 0; i < nodeData.size(); ++i) {
        for (size_t childIdx : nodeData[i].children) {
            childrenSet.insert(childIdx);
            if (childIdx >= entities.size())
                continue;

            GameObject* childObj = gameObjects[childIdx];
            if (!childObj)
                continue;

            // Set the proper parent according to the hierarchy
            childObj->setParent(entities[i]);
        }
    }

    // Find the root object - the node that doesn't have a parent in the hierarchy
    // The root is the first node that is not anyone's child
    size_t rootIdx = 0;
    for (size_t i = 0; i < nodeData.size(); ++i) {
        if (childrenSet.find(i) == childrenSet.end()) {
            rootIdx = i;
            break;
        }
    }

    GameObject* rootObject = gameObjects[rootIdx];
    rootObject->setScale(glm::vec3(rootScale));

    // Create GameObjects for any unassigned meshes and parent them to the root object
    for (size_t i = 0; i < meshes.size(); ++i) {
        if (meshAssigned[i] || meshes[i] == nullptr) {
            continue;
        }

        // Create a default SceneData for the unassigned mesh
        SceneData unassignedData;
        unassignedData.name = "UnassignedMesh(" + std::to_string(i) + ")";

        // Create a new entity and GameObject for this unassigned mesh
        entt::entity newEntity = registry.create();
        GameObject* newGameObject = SceneUtils::addGameObjectComponent(registry, newEntity, unassignedData);

        // Assign material and mesh
        registry.emplace<Mesh*>(newEntity, meshes[i]);

        // Parent to root object
        newGameObject->setParent(entities[rootIdx]);
    }

    return rootObject;
}

void SceneUtils::createEmptyGameObject(entt::registry& registry, const SceneData& data) {
    entt::entity entity = registry.create();
    addGameObjectComponent(registry, entity, data);
}

entt::entity SceneUtils::createGizmo(entt::registry& registry, const SceneData& data, Material* mat, GizmoType type) {
    entt::entity entity = registry.create();
    GameObject* gameObject = addGameObjectComponent(registry, entity, data);

    Mesh* gizmoMesh = nullptr;
    switch (type) {
        case GizmoType::AXIS:
            gizmoMesh = Gizmos::createAxis();
            break;
        case GizmoType::CUBE:
            gizmoMesh = Gizmos::createCube();
            break;
        case GizmoType::PLANE:
            gizmoMesh = Gizmos::createPlane();
            break;
    }

    if (gizmoMesh != nullptr) {
        mat->isDeferred = false;
        gizmoMesh->material = mat;
        gizmoMesh->wireframe = true;
        registry.emplace<Mesh*>(entity, gizmoMesh);
        return entity;
    }

    std::cerr << "[Error] createGizmo() failed to create gizmo mesh!\n";
    return entt::null;
}

void SceneUtils::calculateMeshBounds(const std::vector<Mesh*>& meshes, glm::vec3& outMin, glm::vec3& outMax, float& outMaxSingleMeshDim) {
    outMin = glm::vec3(FLT_MAX);
    outMax = glm::vec3(-FLT_MAX);
    outMaxSingleMeshDim = -FLT_MAX;

    for (const auto& mesh : meshes) {
        if (!mesh) continue;

        // Calculate bounding box for this mesh
        glm::vec3 meshMin(FLT_MAX);
        glm::vec3 meshMax(-FLT_MAX);

        for (const auto& vertex : mesh->vertices) {
            meshMin.x = std::min(meshMin.x, vertex.x);
            meshMin.y = std::min(meshMin.y, vertex.y);
            meshMin.z = std::min(meshMin.z, vertex.z);

            meshMax.x = std::max(meshMax.x, vertex.x);
            meshMax.y = std::max(meshMax.y, vertex.y);
            meshMax.z = std::max(meshMax.z, vertex.z);

            // Update global bounds
            outMin.x = std::min(outMin.x, vertex.x);
            outMin.y = std::min(outMin.y, vertex.y);
            outMin.z = std::min(outMin.z, vertex.z);

            outMax.x = std::max(outMax.x, vertex.x);
            outMax.y = std::max(outMax.y, vertex.y);
            outMax.z = std::max(outMax.z, vertex.z);
        }

        // Calculate largest dimension of this mesh
        float sizeX = meshMax.x - meshMin.x;
        float sizeY = meshMax.y - meshMin.y;
        float sizeZ = meshMax.z - meshMin.z;
        float largestDim = std::max(std::max(sizeX, sizeY), sizeZ);

        outMaxSingleMeshDim = std::max(outMaxSingleMeshDim, largestDim);
    }
}
