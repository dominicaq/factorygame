#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "script.h"
#include "transform_components.h"
#include "metadata.h"

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
    std::vector<std::shared_ptr<Script>> m_scripts;

    void markChildrenDirty(entt::entity parent);

public:
    GameObject(entt::entity entity, entt::registry& registry);
    ~GameObject() = default;

    /*
    * Script management
    */
    template<typename ScriptType>
    void addScript() {
        static_assert(std::is_base_of<Script, ScriptType>::value, "ScriptType must derive from Script");

        auto script = std::make_shared<ScriptType>();
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

    void startScripts();
    void updateScripts(const float& deltaTime);
    void destroyScripts();

    /*
    * Meta data
    */
    std::string getName();

    /*
    * Transform
    */
    entt::entity getEntity() const { return m_entity; }

    void setParent(entt::entity newParent);

    glm::vec3& getPosition();
    void setPosition(const glm::vec3& pos);

    glm::vec3& getEuler();
    void setEuler(const glm::vec3& euler);

    glm::quat& getRotation();
    void setRotation(const glm::quat& rotation);

    glm::vec3& getScale();
    void setScale(const glm::vec3& scale);

    glm::vec3 getForward();
    glm::vec3 getUp();
    glm::vec3 getRight();

    /*
    * ECS Accessors
    */
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
