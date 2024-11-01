#ifndef TRANSFORM_SYSTEM_H
#define TRANSFORM_SYSTEM_H

#include "../transform.h"
#include "../transform_components.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>

class TransformSystem {
public:
    TransformSystem(entt::registry& registry);
    ~TransformSystem() = default;

    void forceCacheUpdate();
    void updateTransformComponents();

private:
    void updateDirtyMatrices();
    void checkDirtyParents();

    entt::registry& m_registry;
};

#endif // TRANSFORM_SYSTEM_H
