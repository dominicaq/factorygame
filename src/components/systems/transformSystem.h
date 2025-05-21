#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "../transform.h"
#include "../metadata.h"

class TransformSystem {
public:
    TransformSystem(entt::registry& registry);
    ~TransformSystem() = default;

    void updateTransformComponents();

private:
    void updateTransformRecursive(entt::entity entity, const glm::mat4& parentMatrix);

    entt::registry& m_registry;
};
