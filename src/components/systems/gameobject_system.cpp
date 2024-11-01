#include "gameobject_system.h"
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
    auto view = m_registry.view<GameObject>();
    for (auto entity : view) {
        auto& gameObject = view.get<GameObject>(entity);
        if (gameObject.isActive) {
            gameObject.startScripts();
        }
    }
}

void GameObjectSystem::updateAll(float deltaTime) {
    auto view = m_registry.view<GameObject>();
    for (auto entity : view) {
        auto& gameObject = view.get<GameObject>(entity);
        if (gameObject.isActive) {
            gameObject.updateScripts(deltaTime);
        }
    }
}

std::vector<GameObject*> GameObjectSystem::getActiveGameObjects() const {
    std::vector<GameObject*> activeObjects;
    auto view = m_registry.view<GameObject>();
    for (auto entity : view) {
        auto& gameObject = view.get<GameObject>(entity);
        if (gameObject.isActive) {
            activeObjects.push_back(&gameObject);
        }
    }
    return activeObjects;
}
