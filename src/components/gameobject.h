#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "script.h"
#include "transform_components.h"
#include "transform.h"

#include <vector>
#include <memory>
#include <iostream>
#include <entt/entt.hpp>

class GameObject {
public:
    bool isActive = true;

private:
    entt::entity m_entity;
    entt::registry& m_registry;
    std::vector<std::unique_ptr<Script>> m_scripts;

public:
    GameObject(entt::entity entity, entt::registry& registry)
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

    ~GameObject() {}

    /*
    * Script management
    */
    template<typename ScriptType>
    void addScript() {
        static_assert(std::is_base_of<Script, ScriptType>::value, "ScriptType must derive from Script");

        auto script = std::make_unique<ScriptType>();
        script->gameObject = this;
        m_scripts.push_back(std::move(script));
    }

    template<typename ScriptType>
    ScriptType* getScript() {
        for (const auto& script : m_scripts) {
            if (ScriptType* s = dynamic_cast<ScriptType*>(script.get())) {
                return s;
            }
        }
        return nullptr;
    }

    void updateScripts(float deltaTime) {
        for (const auto& script : m_scripts) {
            if (script->isActive) {
                script->update(deltaTime);
            }
        }
    }

    void startScripts() {
        for (const auto& script : m_scripts) {
            if (script->isActive) {
                script->start();
            }
        }
    }

    /*
    * Transform
    */
    entt::entity getEntity() const { return m_entity; }

    glm::vec3& getPosition() {
        return Transform::getPosition(m_registry, m_entity);
    }

    void setPosition(const glm::vec3& pos) {
        Transform::setPosition(m_registry, m_entity, pos);
    }

    glm::vec3& getEuler() {
        return Transform::getEuler(m_registry, m_entity);
    }

    void setEuler(const glm::vec3& euler) {
        Transform::setEuler(m_registry, m_entity, euler);
    }

    glm::quat& getRotation() {
        return Transform::getRotation(m_registry, m_entity);
    }

    void setRotation(const glm::quat& rotation) {
        Transform::setRotation(m_registry, m_entity, rotation);
    }

    glm::vec3& getScale() {
        return Transform::getScale(m_registry, m_entity);
    }

    void setScale(const glm::vec3& scale) {
        Transform::setScale(m_registry, m_entity, scale);
    }

    glm::vec3 getForward() {
        return Transform::getForward(m_registry, m_entity);
    }

    glm::vec3 getUp() {
        return Transform::getUp(m_registry, m_entity);
    }

    glm::vec3 getRight() {
        return Transform::getRight(m_registry, m_entity);
    }

    /*
    * ECS Accessors
    */
    // Get ECS component from this GameObject by type
    template<typename ComponentType>
    ComponentType& getComponent() {
        return m_registry.get<ComponentType>(m_entity);
    }

    template<typename ComponentType>
    bool hasComponent() const {
        return m_registry.all_of<ComponentType>(m_entity);
    }

    template<typename ComponentType, typename... Args>
    ComponentType& addComponent(Args&&... args) {
        return m_registry.emplace<ComponentType>(m_entity, std::forward<Args>(args)...);
    }

    template<typename ResourceType>
    ResourceType& getResource() {
        // Access the context and retrieve the resource
        auto* resourcePtr = m_registry.ctx().find<ResourceType>();
        if (!resourcePtr) {
            throw std::runtime_error("Resource not found in registry context.");
        }
        return *resourcePtr;
    }
};

#endif // GAMEOBJECT_H
