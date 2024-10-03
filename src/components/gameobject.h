#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "script.h"
#include "ecs/ecs.h"
#include "modelmatrix.h"

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
        if (!m_world->hasComponent<PositionComponent>(m_entity)) {
            m_world->addComponent<PositionComponent>(m_entity);
        }
        if (!m_world->hasComponent<RotationComponent>(m_entity)) {
            m_world->addComponent<RotationComponent>(m_entity);
        }
        if (!m_world->hasComponent<ScaleComponent>(m_entity)) {
            m_world->addComponent<ScaleComponent>(m_entity);
        }
        if (!m_world->hasComponent<ModelMatrix>(m_entity)) {
            m_world->addComponent<ModelMatrix>(m_entity);
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
    * ECS Components Accessors
    */
    Entity getEntity() const { return m_entity; }

    // Setters and getters for Position, Rotation, and Scale
    glm::vec3 getPosition() {
        return m_world->getComponent<PositionComponent>(m_entity).position;
    }

    void setPosition(const glm::vec3& pos) {
        m_world->getComponent<PositionComponent>(m_entity).position = pos;
        m_world->getComponent<ModelMatrix>(m_entity).dirty = true;
    }

    glm::vec3 getRotation() {
        return m_world->getComponent<RotationComponent>(m_entity).eulerAngles;
    }

    void setRotation(const glm::vec3& rot) {
        m_world->getComponent<RotationComponent>(m_entity).eulerAngles = rot;
        m_world->getComponent<ModelMatrix>(m_entity).dirty = true;
    }

    glm::vec3 getScale() {
        return m_world->getComponent<ScaleComponent>(m_entity).scale;
    }

    void setScale(const glm::vec3& scl) {
        m_world->getComponent<ScaleComponent>(m_entity).scale = scl;
        m_world->getComponent<ModelMatrix>(m_entity).dirty = true;
    }

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
