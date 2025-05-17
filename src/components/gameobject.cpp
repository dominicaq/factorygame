#include "gameobject.h"
#include "mesh.h"

GameObject::GameObject(entt::entity entity, entt::registry& registry)
    : m_entity(entity), m_registry(registry) {
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

void GameObject::updateScripts(const float& deltaTime) {
    for (auto& script : m_scripts) {
        if (!script->isActive) {
            continue;
        }
        // Function pointer call
        script->updateFunc(script.get(), deltaTime);
    }
}

void GameObject::destroyScripts() {
    for (auto& script : m_scripts) {
        script->onDestroy();
    }
    m_scripts.clear();
}

void GameObject::destroy() {
    if (!m_registry.valid(m_entity)) {
        return; // Entity already destroyed
    }

    destroyScripts();

    // Get the list of all descendants before we start destroying entities
    std::vector<entt::entity> entitiesToDestroy;
    entitiesToDestroy.push_back(m_entity);

    // Remove from parent's children list
    if (auto* parent = m_registry.try_get<Parent>(m_entity)) {
        if (m_registry.valid(parent->parent) && m_registry.any_of<Children>(parent->parent)) {
            auto& siblings = m_registry.get<Children>(parent->parent).children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), m_entity), siblings.end());
        }
    }

    // Collect all descendants with breadth-first search
    size_t index = 0;
    while (index < entitiesToDestroy.size()) {
        entt::entity current = entitiesToDestroy[index++];

        if (m_registry.valid(current) && m_registry.any_of<Children>(current)) {
            auto& children = m_registry.get<Children>(current).children;
            for (auto child : children) {
                if (m_registry.valid(child)) {
                    entitiesToDestroy.push_back(child);
                }
            }
            // Clear the children list to prevent invalid references
            children.clear();
        }
    }

    // Now process in reverse order (children first, then parents)
    for (auto it = entitiesToDestroy.rbegin(); it != entitiesToDestroy.rend(); ++it) {
        entt::entity entity = *it;
        if (m_registry.valid(entity)) {
            // Queue these entities for destruction
            m_registry.emplace_or_replace<PendingDestroy>(entity);
        }
    }

    // Mark this gameobject as no longer usable
    m_entity = entt::null;
}

/*
* Meta data
*/
std::string GameObject::getName() {
    return m_registry.get<MetaData>(m_entity).name;
}

/*
* Transform
*/
void GameObject::setParent(entt::entity newParent) {
    // Get current world position and rotation
    glm::vec3 childWorldPos = m_registry.get<Position>(m_entity).position;
    glm::quat childWorldRot = m_registry.get<Rotation>(m_entity).quaternion;

    // Get new parent world transform
    glm::vec3 parentWorldPos = m_registry.get<Position>(newParent).position;
    glm::quat parentWorldRot = m_registry.get<Rotation>(newParent).quaternion;
    glm::vec3 parentWorldScale = calculateWorldScale(newParent);

    glm::quat parentWorldRotInv = glm::inverse(parentWorldRot);
    glm::vec3 invParentScale = glm::vec3(1.0f) / parentWorldScale;

    // Calculate local transforms
    glm::vec3 localPos = parentWorldRotInv * ((childWorldPos - parentWorldPos) * invParentScale);
    glm::quat localRot = parentWorldRotInv * childWorldRot;

    // For scale, we calculate a new local scale that will maintain the current world scale
    glm::vec3 childWorldScale = calculateWorldScale(m_entity);
    glm::vec3 localScale = childWorldScale * invParentScale;

    // Set components
    m_registry.get<Position>(m_entity).position = localPos;
    m_registry.get<Rotation>(m_entity).quaternion = glm::normalize(localRot);
    m_registry.get<Scale>(m_entity).scale = localScale;
    m_registry.get<EulerAngles>(m_entity).euler = glm::degrees(glm::eulerAngles(glm::normalize(localRot)));

    // Set up parent-child relationship
    m_registry.emplace<Parent>(m_entity, newParent);

    if (!m_registry.any_of<Children>(newParent)) {
        m_registry.emplace<Children>(newParent);
    }
    m_registry.get<Children>(newParent).children.push_back(m_entity);
}

glm::vec3& GameObject::getPosition() {
    return m_registry.get<Position>(m_entity).position;
}

void GameObject::setPosition(const glm::vec3& pos) {
    m_registry.get<Position>(m_entity).position = pos;
    markTransformsDirty(m_entity);
}

glm::vec3& GameObject::getEuler() {
    return m_registry.get<EulerAngles>(m_entity).euler;
}

void GameObject::setEuler(const glm::vec3& euler) {
    auto& eulerComponent = m_registry.get<EulerAngles>(m_entity);
    eulerComponent.euler = euler;

    auto& rotationComponent = m_registry.get<Rotation>(m_entity);
    rotationComponent.quaternion = glm::quat(glm::radians(euler));

    markTransformsDirty(m_entity);
}

glm::quat& GameObject::getRotation() {
    return m_registry.get<Rotation>(m_entity).quaternion;
}

void GameObject::setRotation(const glm::quat& rotation) {
    auto& rotationComponent = m_registry.get<Rotation>(m_entity);
    auto& eulerComponent = m_registry.get<EulerAngles>(m_entity);

    rotationComponent.quaternion = rotation;
    eulerComponent.euler = glm::degrees(glm::eulerAngles(rotation));

    markTransformsDirty(m_entity);
}

glm::vec3& GameObject::getScale() {
    return m_registry.get<Scale>(m_entity).scale;
}

void GameObject::setScale(const glm::vec3& newScale) {
    auto& scaleComponent = m_registry.get<Scale>(m_entity);
    scaleComponent.scale = newScale;
    markTransformsDirty(m_entity);
}

glm::vec3 GameObject::calculateWorldScale(entt::entity entity) {
    glm::vec3 localScale = m_registry.get<Scale>(entity).scale;

    // If we have a parent, multiply by parent's world scale
    if (m_registry.all_of<Parent>(entity)) {
        entt::entity parentEntity = m_registry.get<Parent>(entity).parent;
        if (m_registry.valid(parentEntity)) {
            return localScale * calculateWorldScale(parentEntity);
        }
    }

    // No parent or invalid parent, local scale is world scale
    return localScale;
}

glm::vec3 GameObject::getForward() {
    const auto& rotation = m_registry.get<Rotation>(m_entity).quaternion;
    return glm::normalize(rotation * glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 GameObject::getUp() {
    glm::vec3 front = getForward();
    glm::vec3 right = getRight();
    return glm::normalize(glm::cross(right, front));
}

glm::vec3 GameObject::getRight() {
    glm::vec3 front = getForward();
    return glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
}

void GameObject::markTransformsDirty(entt::entity parent) {
    m_registry.get<ModelMatrix>(parent).dirty = true;
    if (!m_registry.all_of<Children>(parent)) {
        return;
    }

    const auto& children = m_registry.get<Children>(parent).children;
    for (auto child : children) {
        if (m_registry.all_of<ModelMatrix>(child)) {
            auto& modelMatrix = m_registry.get<ModelMatrix>(child);
            modelMatrix.dirty = true;
        }

        markTransformsDirty(child);
    }
}
