#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "transform_components.h"
#include <entt/entt.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Transform {

    glm::vec3& getPosition(entt::registry& registry, entt::entity entity);
    void setPosition(entt::registry& registry, entt::entity entity, const glm::vec3& pos);

    glm::vec3& getEuler(entt::registry& registry, entt::entity entity);
    void setEuler(entt::registry& registry, entt::entity entity, const glm::vec3& euler);

    glm::quat& getRotation(entt::registry& registry, entt::entity entity);
    void setRotation(entt::registry& registry, entt::entity entity, const glm::quat& rotation);

    glm::vec3& getScale(entt::registry& registry, entt::entity entity);
    void setScale(entt::registry& registry, entt::entity entity, const glm::vec3& scale);

    glm::vec3 getForward(entt::registry& registry, entt::entity entity);
    glm::vec3 getUp(entt::registry& registry, entt::entity entity);
    glm::vec3 getRight(entt::registry& registry, entt::entity entity);

    bool hasTransformComponents(entt::registry& registry, entt::entity entity);
    void addTransformComponents(entt::registry& registry, entt::entity entity,
        const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& rotationEuler = glm::vec3(0.0f),
        const glm::vec3& scale = glm::vec3(1.0f));

    void setParent(entt::registry& registry, entt::entity child, entt::entity newParent);

} // namespace Transform

void markChildrenDirty(entt::registry& registry, entt::entity parent);

#endif // TRANSFORM_H
