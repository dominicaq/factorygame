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

GameObject* SceneUtils::createMeshGameObject(
    entt::registry& registry,
    std::vector<EntityMeshDefinition>& meshEntityPairs,
    const std::string& filePath,
    const std::string& vertPath,
    const std::string& fragPath) {

    std::vector<SceneData> nodeData;
    std::vector<std::unique_ptr<RawMeshData>> meshes;
    std::vector<std::unique_ptr<MaterialDefinition>> materialDefs;

    // Load glTF with material definitions
    ResourceLoader::loadMeshVector(filePath, meshes, materialDefs, nodeData);

    if (meshes.empty()) {
        return nullptr;
    }

    std::vector<entt::entity> entities(nodeData.size());
    std::vector<GameObject*> gameObjects(nodeData.size());
    std::vector<bool> meshAssigned(meshes.size(), false);

    for (size_t i = 0; i < nodeData.size(); ++i) {
        entities[i] = registry.create();

        // Check if this node has a mesh
        int meshIndex = nodeData[i].meshIndex;
        if (meshIndex >= 0 && meshIndex < static_cast<int>(meshes.size()) && meshes[meshIndex] != nullptr) {
            // Create EntityMeshDefinition with both mesh and material data
            EntityMeshDefinition newPair{entities[i]};
            newPair.rawMeshData = std::move(meshes[meshIndex]);

            // Copy material definition if available FIRST
            if (meshIndex < static_cast<int>(materialDefs.size()) && materialDefs[meshIndex]) {
                *newPair.materialDef = *materialDefs[meshIndex];
            }

            // THEN set shader paths (after copying material data)
            newPair.materialDef->vertexShaderPath = vertPath;
            newPair.materialDef->fragmentShaderPath = fragPath;

            meshEntityPairs.emplace_back(std::move(newPair));
            meshAssigned[meshIndex] = true;
        }

        gameObjects[i] = SceneUtils::addGameObjectComponent(registry, entities[i], nodeData[i]);
    }

    // Set up parent-child relationships
    std::unordered_set<size_t> childrenSet;
    for (size_t i = 0; i < nodeData.size(); ++i) {
        for (size_t childIdx : nodeData[i].children) {
            if (childIdx >= entities.size() || !gameObjects[childIdx])
                continue;

            childrenSet.insert(childIdx);
            GameObject* childObj = gameObjects[childIdx];
            childObj->setParent(entities[i]);
        }
    }

    // Find the root node
    size_t rootIdx = 0;
    for (size_t i = 0; i < nodeData.size(); ++i) {
        if (childrenSet.find(i) == childrenSet.end()) {
            rootIdx = i;
            break;
        }
    }

    // Handle unassigned meshes
    for (size_t i = 0; i < meshes.size(); ++i) {
        if (meshAssigned[i] || meshes[i] == nullptr) {
            continue;
        }

        SceneData unassignedData;
        unassignedData.name = "UnassignedMesh(" + std::to_string(i) + ")";

        entt::entity newEntity = registry.create();
        GameObject* newGameObject = SceneUtils::addGameObjectComponent(registry, newEntity, unassignedData);

        // Create EntityMeshDefinition for unassigned mesh
        EntityMeshDefinition unassignedPair{newEntity};
        unassignedPair.rawMeshData = std::move(meshes[i]);

        // Copy material definition if available
        if (i < materialDefs.size() && materialDefs[i]) {
            *unassignedPair.materialDef = *materialDefs[i];
        }

        unassignedPair.materialDef->vertexShaderPath = vertPath;
        unassignedPair.materialDef->fragmentShaderPath = fragPath;

        meshEntityPairs.emplace_back(std::move(unassignedPair));
        newGameObject->setParent(entities[rootIdx]);
    }

    return gameObjects[rootIdx];
}

void SceneUtils::createEmptyGameObject(entt::registry& registry, const SceneData& data) {
    entt::entity entity = registry.create();
    addGameObjectComponent(registry, entity, data);
}

// entt::entity SceneUtils::createGizmo(entt::registry& registry, const SceneData& data, Material* mat, GizmoType type) {
    // entt::entity entity = registry.create();
    // GameObject* gameObject = addGameObjectComponent(registry, entity, data);

    // Mesh* gizmoMesh = nullptr;
    // switch (type) {
    //     case GizmoType::AXIS:
    //         gizmoMesh = Gizmos::createAxis();
    //         break;
    //     case GizmoType::CUBE:
    //         gizmoMesh = Gizmos::createCube();
    //         break;
    //     case GizmoType::PLANE:
    //         gizmoMesh = Gizmos::createPlane();
    //         break;
    // }

    // if (gizmoMesh != nullptr) {
    //     mat->isDeferred = false;
    //     gizmoMesh->material = mat;
    //     gizmoMesh->wireframe = true;
    //     registry.emplace<Mesh*>(entity, gizmoMesh);
    //     return entity;
    // }

    // std::cerr << "[Error] createGizmo() failed to create gizmo mesh!\n";
//     return entt::null;
// }
