#include "transform.h"

namespace Transform {

    glm::vec3& getPosition(entt::registry& registry, entt::entity entity) {
        return registry.get<Position>(entity).position;
    }

    void setPosition(entt::registry& registry, entt::entity entity, const glm::vec3& pos) {
        registry.get<Position>(entity).position = pos;
        registry.get<ModelMatrix>(entity).dirty = true;
    }

    glm::vec3& getEuler(entt::registry& registry, entt::entity entity) {
        return registry.get<EulerAngles>(entity).euler;
    }

    void setEuler(entt::registry& registry, entt::entity entity, const glm::vec3& euler) {
        auto& eulerComponent = registry.get<EulerAngles>(entity);
        eulerComponent.euler = euler;

        auto& rotationComponent = registry.get<Rotation>(entity);
        rotationComponent.quaternion = glm::quat(glm::radians(euler));

        registry.get<ModelMatrix>(entity).dirty = true;
    }

    glm::quat& getRotation(entt::registry& registry, entt::entity entity) {
        return registry.get<Rotation>(entity).quaternion;
    }

    void setRotation(entt::registry& registry, entt::entity entity, const glm::quat& rotation) {
        registry.get<Rotation>(entity).quaternion = rotation;
        registry.get<EulerAngles>(entity).euler = glm::degrees(glm::eulerAngles(rotation));
        registry.get<ModelMatrix>(entity).dirty = true;
    }

    glm::vec3& getScale(entt::registry& registry, entt::entity entity) {
        return registry.get<Scale>(entity).scale;
    }

    void setScale(entt::registry& registry, entt::entity entity, const glm::vec3& scale) {
        registry.get<Scale>(entity).scale = scale;
        registry.get<ModelMatrix>(entity).dirty = true;
    }

    glm::vec3 getForward(entt::registry& registry, entt::entity entity) {
        auto& rotation = registry.get<Rotation>(entity).quaternion;
        return rotation * glm::vec3(0, 0, 1);
    }

    glm::vec3 getUp(entt::registry& registry, entt::entity entity) {
        auto& rotation = registry.get<Rotation>(entity).quaternion;
        return rotation * glm::vec3(0, 1, 0);
    }

    glm::vec3 getRight(entt::registry& registry, entt::entity entity) {
        auto& rotation = registry.get<Rotation>(entity).quaternion;
        return rotation * glm::vec3(1, 0, 0);
    }

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

    void setParent(entt::registry& registry, entt::entity child, entt::entity newParent) {
        if (!hasTransformComponents(registry, child) || !hasTransformComponents(registry, newParent)) {
            return;
        }

        glm::vec3 childWorldPos = registry.get<Position>(child).position;
        glm::quat childWorldRot = registry.get<Rotation>(child).quaternion;
        glm::vec3 childWorldScale = registry.get<Scale>(child).scale;

        glm::vec3 parentWorldPos = registry.get<Position>(newParent).position;
        glm::quat parentWorldRot = registry.get<Rotation>(newParent).quaternion;
        glm::vec3 parentWorldScale = registry.get<Scale>(newParent).scale;

        glm::quat parentWorldRotInv = glm::inverse(parentWorldRot);
        glm::vec3 invParentScale = glm::vec3(1.0f) / parentWorldScale;

        glm::vec3 localPos = parentWorldRotInv * ((childWorldPos - parentWorldPos) * invParentScale);
        glm::quat localRot = parentWorldRotInv * childWorldRot;
        glm::vec3 localScale = childWorldScale / parentWorldScale;

        registry.get<Position>(child).position = localPos;
        registry.get<Rotation>(child).quaternion = glm::normalize(localRot);
        registry.get<Scale>(child).scale = localScale;
        registry.get<EulerAngles>(child).euler = glm::degrees(glm::eulerAngles(glm::normalize(localRot)));

        registry.emplace<Parent>(child, newParent);

        if (!registry.any_of<Children>(newParent)) {
            registry.emplace<Children>(newParent);
        }
        registry.get<Children>(newParent).children.push_back(child);

        markChildrenDirty(registry, child);
    }

} // namespace Transform

void markChildrenDirty(entt::registry& registry, entt::entity parent) {
    if (!registry.any_of<Children>(parent)) {
        return;
    }

    for (auto child : registry.get<Children>(parent).children) {
        auto& modelMatrix = registry.get<ModelMatrix>(child);
        modelMatrix.dirty = true;
        markChildrenDirty(registry, child);
    }
}
