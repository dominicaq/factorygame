#include "gameobject.h"

GameObject::GameObject(entt::entity entity, entt::registry& registry)
    : m_entity(entity), m_registry(registry) {
    // Automatically add transform components if not already present
    if (!m_registry.all_of<Position>(m_entity)) {
        m_registry.emplace<Position>(m_entity);
    }
    if (!m_registry.all_of<EulerAngles>(m_entity)) {
        m_registry.emplace<EulerAngles>(m_entity);
    }
    if (!m_registry.all_of<Rotation>(m_entity)) {
        m_registry.emplace<Rotation>(m_entity);
    }
    if (!m_registry.all_of<Scale>(m_entity)) {
        m_registry.emplace<Scale>(m_entity);
    }
}

GameObject::~GameObject() {
    m_scripts.clear();
    m_registry.destroy(m_entity);
}

/*
* Script Management
*/
void GameObject::startScripts() {
    for (const auto& script : m_scripts) {
        if (script->isActive) {
            script->start();
        }
    }
}

void GameObject::updateScripts(float deltaTime) {
    for (const auto& script : m_scripts) {
        if (script->isActive) {
            script->update(deltaTime);
        }
    }
}

/*
* Transform Management
*/
void GameObject::setParent(entt::entity newParent) {
    glm::vec3 childWorldPos = m_registry.get<Position>(m_entity).position;
    glm::quat childWorldRot = m_registry.get<Rotation>(m_entity).quaternion;
    glm::vec3 childWorldScale = m_registry.get<Scale>(m_entity).scale;

    glm::vec3 parentWorldPos = m_registry.get<Position>(newParent).position;
    glm::quat parentWorldRot = m_registry.get<Rotation>(newParent).quaternion;
    glm::vec3 parentWorldScale = m_registry.get<Scale>(newParent).scale;

    glm::quat parentWorldRotInv = glm::inverse(parentWorldRot);
    glm::vec3 invParentScale = glm::vec3(1.0f) / parentWorldScale;

    glm::vec3 localPos = parentWorldRotInv * ((childWorldPos - parentWorldPos) * invParentScale);
    glm::quat localRot = parentWorldRotInv * childWorldRot;
    glm::vec3 localScale = childWorldScale / parentWorldScale;

    m_registry.get<Position>(m_entity).position = localPos;
    m_registry.get<Rotation>(m_entity).quaternion = glm::normalize(localRot);
    m_registry.get<Scale>(m_entity).scale = localScale;
    m_registry.get<EulerAngles>(m_entity).euler = glm::degrees(glm::eulerAngles(glm::normalize(localRot)));

    m_registry.emplace<Parent>(m_entity, newParent);

    if (!m_registry.any_of<Children>(newParent)) {
        m_registry.emplace<Children>(newParent);
    }
    m_registry.get<Children>(newParent).children.push_back(m_entity);

    markChildrenDirty(m_entity);
}

glm::vec3& GameObject::getPosition() {
    return m_registry.get<Position>(m_entity).position;
}

void GameObject::setPosition(const glm::vec3& pos) {
    m_registry.get<Position>(m_entity).position = pos;
    m_registry.get<ModelMatrix>(m_entity).dirty = true;
}

glm::vec3& GameObject::getEuler() {
    return m_registry.get<EulerAngles>(m_entity).euler;
}

void GameObject::setEuler(const glm::vec3& euler) {
    auto& eulerComponent = m_registry.get<EulerAngles>(m_entity);
    eulerComponent.euler = euler;

    auto& rotationComponent = m_registry.get<Rotation>(m_entity);
    rotationComponent.quaternion = glm::quat(glm::radians(euler));

    m_registry.get<ModelMatrix>(m_entity).dirty = true;
}

glm::quat& GameObject::getRotation() {
    return m_registry.get<Rotation>(m_entity).quaternion;
}

void GameObject::setRotation(const glm::quat& rotation) {
    m_registry.get<Rotation>(m_entity).quaternion = rotation;
    m_registry.get<EulerAngles>(m_entity).euler = glm::degrees(glm::eulerAngles(rotation));
    m_registry.get<ModelMatrix>(m_entity).dirty = true;
}

glm::vec3& GameObject::getScale() {
    return m_registry.get<Scale>(m_entity).scale;
}

void GameObject::setScale(const glm::vec3& scale) {
    m_registry.get<Scale>(m_entity).scale = scale;
    m_registry.get<ModelMatrix>(m_entity).dirty = true;
}

glm::vec3 GameObject::getForward() {
    auto& rotation = m_registry.get<Rotation>(m_entity).quaternion;
    return rotation * glm::vec3(0, 0, 1);
}

glm::vec3 GameObject::getUp() {
    auto& rotation = m_registry.get<Rotation>(m_entity).quaternion;
    return rotation * glm::vec3(0, 1, 0);
}

glm::vec3 GameObject::getRight() {
    auto& rotation = m_registry.get<Rotation>(m_entity).quaternion;
    return rotation * glm::vec3(0, 0, 1);
}

void GameObject::markChildrenDirty(entt::entity parent) {
    if (!m_registry.any_of<Children>(parent)) {
        return;
    }

    for (auto child : m_registry.get<Children>(parent).children) {
        auto& modelMatrix = m_registry.get<ModelMatrix>(child);
        modelMatrix.dirty = true;
        markChildrenDirty(child);
    }
}
