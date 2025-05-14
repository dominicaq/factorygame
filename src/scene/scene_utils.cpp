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
    ResourceLoader::loadMeshVector(filePath, meshes, nodeData);
    if (meshes.empty()) {
        return nullptr;
    }

    // Create entities for each node
    std::vector<entt::entity> entities(nodeData.size());
    std::vector<GameObject*> gameObjects(nodeData.size());

    // First pass: Create entities and assign meshes/materials
    for (size_t i = 0; i < nodeData.size(); ++i) {
        entities[i] = registry.create();
        gameObjects[i] = SceneUtils::addGameObjectComponent(registry, entities[i], nodeData[i]);

        // If this node has a mesh, assign it
        if (i < meshes.size() && meshes[i] != nullptr) {
            Material* gltfMat = new Material(shader);
            meshes[i]->material = gltfMat;
            registry.emplace<Mesh*>(entities[i], meshes[i]);
        }
    }

    // Second pass: Set up the hierarchy as defined by the file
    for (size_t i = 0; i < nodeData.size(); ++i) {
        for (size_t childIdx : nodeData[i].children) {
            if (childIdx >= entities.size())
                continue;

            GameObject* childObj = gameObjects[childIdx];
            if (!childObj)
                continue;

            // Set the proper parent according to the hierarchy
            childObj->setParent(entities[i]);
        }
    }

    // Collect all children indices
    std::unordered_set<size_t> childrenSet;
    for (const auto& node : nodeData) {
        for (size_t child : node.children) {
            childrenSet.insert(child);
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

    return gameObjects[rootIdx];
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

void SceneUtils::createModel(entt::registry& registry, const SceneData& data, Mesh* mesh) {
    if (!mesh) {
        return;
    }

    entt::entity entity = registry.create();
    GameObject* gameObject = addGameObjectComponent(registry, entity, data);

    registry.emplace<Mesh*>(entity, mesh);
}
