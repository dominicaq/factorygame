#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "script.h"
#include "ecs/ecs.h"

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
    // Constructor that takes the entity ID and ECSWorld pointer
    GameObject(Entity entity, ECSWorld* world)
        : m_entity(entity), m_world(world) {}

    // Method to add a script to the GameObject
    template<typename ScriptType>
    void addScript() {
        static_assert(std::is_base_of<Script, ScriptType>::value, "ScriptType must derive from Script");

        // Create a new script instance
        auto script = std::make_unique<ScriptType>();

        // Set the gameObject pointer in the script
        script->gameObject = this;

        // Add the script to the scripts vector
        m_scripts.push_back(std::move(script));
    }

    // Method to get a specific script
    template<typename ScriptType>
    ScriptType* getScript() {
        for (const auto& script : m_scripts) {
            if (ScriptType* s = dynamic_cast<ScriptType*>(script.get())) {
                return s;
            }
        }
        return nullptr;  // Return nullptr if the script isn't found
    }

    // Update all scripts attached to this GameObject
    void updateScripts(float deltaTime) {
        for (const auto& script : m_scripts) {
            if (script->isActive) {
                script->update(deltaTime);
            }
        }
    }

    // Call the start() method on all scripts
    void startScripts() {
        for (const auto& script : m_scripts) {
            if (script->isActive) {
                script->start();
            }
        }
    }

    /*
    * ECS Components
    */
    Entity getEntity() const { return m_entity; }

    // Get ECS component from this GameObject by type
    template<typename ComponentType>
    ComponentType* getComponent() {
        // Check if the component is a Mesh (which should return a pointer)
        if constexpr (std::is_same<ComponentType, Mesh>::value) {
            return m_world->getComponent<ComponentType>(m_entity);
        } else {
            // For other components, return a pointer to the reference
            return &(m_world->getComponent<ComponentType>(m_entity));
        }
    }
};

#endif // GAMEOBJECT_H