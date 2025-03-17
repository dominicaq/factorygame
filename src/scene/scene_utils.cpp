#include "scene_utils.h"

void SceneUtils::addLightComponents(entt::registry& registry, entt::entity entity, Light lightData) {
    registry.emplace<Light>(entity, lightData);
    registry.emplace<LightSpaceMatrix>(entity);
}

void SceneUtils::addPointLightComponents(entt::registry& registry, entt::entity entity, Light lightData) {
    registry.emplace<Light>(entity, lightData);
    registry.emplace<LightSpaceMatrixCube>(entity);
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

void SceneUtils::createGizmo(entt::registry& registry, const SceneData& data, Material* mat, PrimitiveType type) {
    entt::entity entity = registry.create();
    GameObject* gameObject = addGameObjectComponent(registry, entity, data);

    Mesh* primitiveMesh = nullptr;

    switch (type) {
        case PrimitiveType::AXIS:
            primitiveMesh = Gizmos::createAxis();
            break;
        case PrimitiveType::CUBE:
            primitiveMesh = Gizmos::createCube();
            break;
        case PrimitiveType::PLANE:
            primitiveMesh = Gizmos::createPlane();
            break;
    }

    if (primitiveMesh != nullptr) {
        mat->isDeferred = false;
        primitiveMesh->material = mat;
        registry.emplace<Mesh*>(entity, primitiveMesh);
    }
}

void SceneUtils::createModel(entt::registry& registry, const SceneData& data, Mesh* mesh) {
    if (!mesh) return;

    entt::entity entity = registry.create();
    GameObject* gameObject = addGameObjectComponent(registry, entity, data);

    registry.emplace<Mesh*>(entity, mesh);
}
