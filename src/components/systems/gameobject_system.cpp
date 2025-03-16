#include "gameobject_system.h"
#include "engine.h"
#include <iostream>

GameObjectSystem::GameObjectSystem(entt::registry& registry) : m_registry(registry) {}

GameObjectSystem::~GameObjectSystem() {}

GameObject* GameObjectSystem::getGameObject(entt::entity entity) const {
    if (m_registry.all_of<GameObject>(entity)) {
        return &m_registry.get<GameObject>(entity);
    }
    return nullptr;
}

void GameObjectSystem::removeGameObject(entt::entity entity) {
    if (!m_registry.valid(entity)) {
        std::cerr << "[Warning]: GameObjectSystem::removeGameObject: Invalid entity.\n";
        return;
    }

    // Call onDestroy for all scripts before removal
    auto* gameObject = getGameObject(entity);
    if (gameObject) {
        gameObject->destroyScripts();
    }

    m_registry.remove<GameObject>(entity);
    m_registry.destroy(entity);
}

void GameObjectSystem::startAll() {
    for (const auto& [entity, gameObject] : m_registry.view<GameObject>().each()) {
        if (gameObject.isActive) {
            gameObject.startScripts();
        }
    }
}

void GameObjectSystem::updateAll(float currentTime, float deltaTime) {
    for (const auto& [entity, gameObject] : m_registry.view<GameObject>().each()) {
        if (!gameObject.isActive) {
            return;
        }

        gameObject.updateScripts(deltaTime);
        if (Mesh** meshPtr = m_registry.try_get<Mesh*>(entity)) {
            Mesh* mesh = *meshPtr;
            if (mesh && mesh->material) {
                mesh->material->time = currentTime;
            }
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