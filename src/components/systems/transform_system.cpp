#include "transform_system.h"

TransformSystem::TransformSystem(entt::registry& registry)
    : m_registry(registry) {}

void TransformSystem::forceCacheUpdate() {
    // m_dirtyModelCache = true;
    // m_dirtyParentCache = true;
}

void TransformSystem::updateTransformComponents() {
    checkDirtyParents();
    updateDirtyMatrices();
}

void TransformSystem::updateDirtyMatrices() {
    auto modelEntityCache = m_registry.view<ModelMatrix>();
    modelEntityCache.each([&](auto entity, ModelMatrix& modelMatrix) {
        if (!modelMatrix.dirty) {
            return;
        }

        auto& position = m_registry.get<Position>(entity).position;
        auto& rotation = m_registry.get<Rotation>(entity).quaternion;
        auto& scale = m_registry.get<Scale>(entity).scale;

        glm::mat4 localMatrix = glm::mat4(1.0f);
        localMatrix = glm::translate(localMatrix, position);
        localMatrix = localMatrix * glm::mat4_cast(rotation);
        localMatrix = glm::scale(localMatrix, scale);

        if (m_registry.any_of<Parent>(entity)) {
            entt::entity parent = m_registry.get<Parent>(entity).parent;
            const auto& parentModelMatrix = m_registry.get<ModelMatrix>(parent).matrix;
            localMatrix = parentModelMatrix * localMatrix;
        }

        modelMatrix.matrix = localMatrix;
        modelMatrix.dirty = false;
    });
}

void TransformSystem::checkDirtyParents() {
    auto parentEntityCache = m_registry.view<Parent>();

    parentEntityCache.each([&](auto entity, Parent& parent) {
        const auto& parentModelMatrix = m_registry.get<ModelMatrix>(parent.parent);

        if (parentModelMatrix.dirty) {
            auto& modelMatrix = m_registry.get<ModelMatrix>(entity);
            modelMatrix.dirty = true;
        }
    });
}
