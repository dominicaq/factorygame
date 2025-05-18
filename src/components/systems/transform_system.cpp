#ifndef __APPLE__
    #define GLM_FORCE_INLINE
    #define GLM_FORCE_SSE2
#endif

#include "transform_system.h"
#include <unordered_map>
#include <vector>
#include <algorithm>

TransformSystem::TransformSystem(entt::registry& registry)
    : m_registry(registry) {}

void TransformSystem::updateTransformComponents() {
    // Start from root entities (those without a Parent)
    auto view = m_registry.view<ModelMatrix>(entt::exclude<Parent>);
    for (auto entity : view) {
        updateTransformRecursive(entity, glm::mat4(1.0f));
    }
}

void TransformSystem::updateTransformRecursive(entt::entity entity, const glm::mat4& parentMatrix) {
    auto& modelMatrix = m_registry.get<ModelMatrix>(entity);

    if (!modelMatrix.dirty) {
        return;
    }

    const auto& position = m_registry.get<Position>(entity).position;
    const auto& rotation = m_registry.get<Rotation>(entity).quaternion;
    const auto& scale = m_registry.get<Scale>(entity).scale;

    glm::mat4 localMatrix = glm::translate(glm::mat4(1.0f), position);
    localMatrix *= glm::mat4_cast(rotation);
    localMatrix = glm::scale(localMatrix, scale);

    modelMatrix.matrix = parentMatrix * localMatrix;
    modelMatrix.dirty = false;

    if (m_registry.all_of<Children>(entity)) {
        const auto& children = m_registry.get<Children>(entity).children;
        for (auto child : children) {
            updateTransformRecursive(child, modelMatrix.matrix);
        }
    }
}
