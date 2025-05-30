#include "gameobjectSystem.h"
#include "engine.h"
#include <iostream>

GameObjectSystem::GameObjectSystem(entt::registry& registry) : m_registry(registry) {}

GameObjectSystem::~GameObjectSystem() {
    const auto& view = m_registry.view<GameObject>();
    for (const auto& entity : view) {
        m_registry.destroy(entity);
    }
}

void GameObjectSystem::startAll() {
    const auto& view = m_registry.view<GameObject>();
    for (const auto& entity : view) {
        GameObject& gameObject = view.get<GameObject>(entity);
        if (gameObject.isActive) {
            gameObject.startScripts();
        }
    }
}

void GameObjectSystem::updateAll(const float& currentTime, const float& deltaTime) {
    std::vector<entt::entity> destroyQueue;

    const auto& view = m_registry.view<GameObject>();
    for (const auto& entity : view) {
        GameObject& gameObject = view.get<GameObject>(entity);

        if (!gameObject.isActive || gameObject.getEntity() == entt::null) {
            continue;
        }

        // TODO: OLD
        // if (Mesh** meshPtr = m_registry.try_get<Mesh*>(entity)) {
        //     Mesh* mesh = *meshPtr;
        //     if (mesh && mesh->material) {
        //        mesh->material->time = currentTime;
        //     }
        // }

        // Tick this gameobject
        gameObject.updateScripts(deltaTime);

        // Check if this entity is set to be destroyed (children are already set from gameObject->destroy())
        if (m_registry.get<EntityStatus>(entity).status.test(EntityStatus::DESTROY_ENTITY)) {
            destroyQueue.push_back(entity);
        }
    }

    // Destroy all entites marked after tick is over
    for (const auto& entity : destroyQueue) {
        if (m_registry.valid(entity)) {
            m_registry.destroy(entity);
        }
    }
}
std::vector<GameObject*> GameObjectSystem::getActiveGameObjects() const {
    std::vector<GameObject*> activeObjects;
    for (const auto& [entity, gameObject] : m_registry.view<GameObject>().each()) {
        if (gameObject.isActive) {
            activeObjects.push_back(&gameObject);
        }
    }

    return activeObjects;
}
