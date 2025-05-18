#ifndef TRANSFORM_SYSTEM_H
#define TRANSFORM_SYSTEM_H

#include "../transform_components.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>

class TransformSystem {
public:
    TransformSystem(entt::registry& registry);
    ~TransformSystem() = default;

    void updateTransformComponents();

private:
    void updateTransformRecursive(entt::entity entity, const glm::mat4& parentMatrix);

    entt::registry& m_registry;
};

#endif // TRANSFORM_SYSTEM_H
