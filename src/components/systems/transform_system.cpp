#ifndef __APPLE__
    #define GLM_FORCE_INLINE
    #define GLM_FORCE_SSE2
#endif

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

void TransformSystem::checkDirtyParents() {
    // Check every entity that has a parent
    for (const auto& [entity, parent, modelMatrix] : m_registry.view<Parent, ModelMatrix>().each()) {
        const auto& parentModelMatrix = m_registry.get<ModelMatrix>(parent.parent);

        if (parentModelMatrix.dirty) {
            modelMatrix.dirty = true;
        }
    }
}

void TransformSystem::updateDirtyMatrices() {
    for (const auto& [entity, modelMatrix] : m_registry.view<ModelMatrix>().each()) {
        if (!modelMatrix.dirty) {
            continue;
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
    }
}
