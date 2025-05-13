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

GameObject* SceneUtils::createMeshGameObject(entt::registry& registry, Shader* shader, const SceneData& rootData, const std::string& filePath) {
    // Parse the mesh
    std::vector<SceneData> nodeData;
    std::vector<Mesh*> meshes;
    ResourceLoader::loadMeshVector(filePath, meshes, nodeData);
    if (meshes.empty()) {
        return nullptr;
    }

    // Create the root for the mesh
    entt::entity rootEntity = registry.create();
    GameObject* rootObject = SceneUtils::addGameObjectComponent(registry, rootEntity, rootData);
    if (!rootObject) {
        return nullptr;
    }

    // First pass: Create entities and assign models
    std::vector<entt::entity> entities(nodeData.size());
    for (size_t i = 0; i < nodeData.size(); ++i) {
        entities[i] = registry.create();
        GameObject* gameObj = SceneUtils::addGameObjectComponent(registry, entities[i], nodeData[i]);

        // If this node has a mesh, assign it
        if (i < meshes.size() && meshes[i] != nullptr) {
            Material* gltfMat = new Material(shader);
            gltfMat->albedoColor = glm::vec4(1.0f);
            gltfMat->isDeferred = true;
            meshes[i]->material = gltfMat;
            registry.emplace<Mesh*>(entities[i], meshes[i]);
        }

        // Initially, set all nodes as children of the root entity
        gameObj->setParent(rootEntity);
    }

    // Second pass: Set up the actual hierarchy as defined by the glTF file
    for (size_t i = 0; i < nodeData.size(); ++i) {
        for (size_t childIdx : nodeData[i].children) {
            if (childIdx >= entities.size())
                continue;

            GameObject* childObj = registry.try_get<GameObject>(entities[childIdx]);
            if (!childObj)
                continue;

            // Set the proper parent according to the glTF hierarchy
            childObj->setParent(entities[i]);
        }
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

void SceneUtils::createModel(entt::registry& registry, const SceneData& data, Mesh* mesh) {
    if (!mesh) {
        return;
    }

    entt::entity entity = registry.create();
    GameObject* gameObject = addGameObjectComponent(registry, entity, data);

    registry.emplace<Mesh*>(entity, mesh);
}
