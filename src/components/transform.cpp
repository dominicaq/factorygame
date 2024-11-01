#include "transform.h"

namespace Transform {
    bool hasTransformComponents(entt::registry& registry, entt::entity entity) {
        return registry.all_of<ModelMatrix, Position, Rotation, Scale>(entity);
    }

    void addTransformComponents(entt::registry& registry, entt::entity entity,
        const glm::vec3& position,
        const glm::vec3& rotationEuler,
        const glm::vec3& scale)
    {
        registry.emplace<Position>(entity, position);
        registry.emplace<EulerAngles>(entity, rotationEuler);
        registry.emplace<Rotation>(entity, glm::quat(glm::radians(rotationEuler)));
        registry.emplace<Scale>(entity, scale);
        registry.emplace<ModelMatrix>(entity);
    }
} // namespace Transform

