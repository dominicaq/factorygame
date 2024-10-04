#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "script.h"
#include "ecs/ecs.h"
#include "transform.h"

#include <vector>
#include <memory>
#include <iostream>

class GameObject {
public:
    bool isActive = true;

private:
    Entity m_entity;
    ECSWorld* m_world;
    std::vector<std::unique_ptr<Script>> m_scripts;

public:
    GameObject(Entity entity, ECSWorld* world)
        : m_entity(entity), m_world(world) {
        // Automatically add transform components if not already present
        if (!m_world->hasComponent<Position>(m_entity)) {
            m_world->addComponent<Position>(m_entity);
        }
        if (!m_world->hasComponent<EulerAngles>(m_entity)) {
            m_world->addComponent<EulerAngles>(m_entity);
        }
        if (!m_world->hasComponent<Rotation>(m_entity)) {
            m_world->addComponent<Rotation>(m_entity);
        }
        if (!m_world->hasComponent<Scale>(m_entity)) {
            m_world->addComponent<Scale>(m_entity);
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
    Entity getEntity() const { return m_entity; }

    glm::vec3& getPosition() {
        return Transform::getPosition(m_world, m_entity);
    }

    void setPosition(const glm::vec3& pos) {
        Transform::setPosition(m_world, m_entity, pos);
    }

    glm::vec3& getEuler() {
        return Transform::getEuler(m_world, m_entity);
    }

    void setEuler(const glm::vec3& euler) {
        Transform::setEuler(m_world, m_entity, euler);
    }

    glm::quat& getRotation() {
        return Transform::getRotation(m_world, m_entity);
    }

    void setRotation(const glm::quat& rotation) {
        Transform::setRotation(m_world, m_entity, rotation);
    }

    glm::vec3& getScale() {
        return Transform::getScale(m_world, m_entity);
    }

    void setScale(const glm::vec3& scale) {
        Transform::setScale(m_world, m_entity, scale);
    }

    glm::vec3 getForward() {
        return Transform::getForward(m_world, m_entity);
    }

    glm::vec3 getUp() {
        return Transform::getUp(m_world, m_entity);
    }

    glm::vec3 getRight() {
        return Transform::getRight(m_world, m_entity);
    }

    /*
    * ECS Accessors
    */
    // Get ECS component from this GameObject by type
    template<typename ComponentType>
    ComponentType* getComponent() {
        if constexpr (ShouldStoreAsPointer<ComponentType>::value) {
            return m_world->getComponent<ComponentType>(m_entity);
        } else {
            return &(m_world->getComponent<ComponentType>(m_entity));
        }
    }

    template<typename ResourceType>
    ResourceType& getResource() {
        return m_world->getResource<ResourceType>();
    }
};

#endif // GAMEOBJECT_H
