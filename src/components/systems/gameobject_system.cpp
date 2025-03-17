#include "gameobject_system.h"
#include "engine.h"
#include <iostream>

GameObjectSystem::GameObjectSystem(entt::registry& registry) : m_registry(registry) {}

GameObjectSystem::~GameObjectSystem() {
    for (const auto& [entity, gameObject] : m_registry.view<GameObject>().each()) {
        m_registry.destroy(entity);
    }
}

void GameObjectSystem::startAll() {
    for (const auto& [entity, gameObject] : m_registry.view<GameObject>().each()) {
        if (gameObject.isActive) {
            gameObject.startScripts();
        }
    }
}

void GameObjectSystem::updateAll(const float& currentTime, const float& deltaTime) {
    for (const auto& [entity, gameObject] : m_registry.view<GameObject>().each()) {
        if (!gameObject.isActive) {
            continue;
        }

        if (Mesh** meshPtr = m_registry.try_get<Mesh*>(entity)) {
            Mesh* mesh = *meshPtr;
            if (mesh && mesh->material) {
                mesh->material->time = currentTime;
            }
        }
        gameObject.updateScripts(deltaTime);
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