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
