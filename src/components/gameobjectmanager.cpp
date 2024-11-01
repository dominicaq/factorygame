#include "gameobjectmanager.h"
#include <iostream>

GameObjectManager::GameObjectManager(entt::registry& registry) : m_registry(registry) {}

GameObjectManager::~GameObjectManager() {}

GameObject* GameObjectManager::createGameObject(entt::entity entity, bool runScripts) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Check if the GameObject already exists
    if (m_gameObjects.find(entity) != m_gameObjects.end()) {
        return m_gameObjects[entity].get();
    }

    // Create and store the GameObject
    m_gameObjects[entity] = std::make_unique<GameObject>(entity, m_registry);

    // Initialize the GameObject (call startScripts)
    if (runScripts && m_gameObjects[entity]->isActive) {
        m_gameObjects[entity]->startScripts();
    }

    // Return the GameObject so scripts can be added
    return m_gameObjects[entity].get();
}

GameObject* GameObjectManager::getGameObject(entt::entity entity) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_gameObjects.find(entity);
    if (it != m_gameObjects.end()) {
        return it->second.get();
    }

    return nullptr;  // GameObject does not exist
}

void GameObjectManager::removeGameObject(entt::entity entity) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_gameObjects.find(entity);
    if (it == m_gameObjects.end()) {
        std::cerr << "[Warning]: GameObjectManager::removeGameObject: Invalid entity.\n";
        return;
    }

    // Perform any necessary cleanup on the GameObject
    m_gameObjects.erase(it);        // Destroy the GameObject
    m_registry.destroy(entity);     // Remove from EnTT registry
}

void GameObjectManager::startAll() {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& [entity, gameObjectPtr] : m_gameObjects) {
        if (gameObjectPtr && gameObjectPtr->isActive) {
            gameObjectPtr->startScripts();
        }
    }
}

void GameObjectManager::updateAll(float deltaTime) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& [entity, gameObjectPtr] : m_gameObjects) {
        if (gameObjectPtr && gameObjectPtr->isActive) {
            gameObjectPtr->updateScripts(deltaTime);
        }
    }
}

std::vector<GameObject*> GameObjectManager::getActiveGameObjects() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<GameObject*> activeObjects;
    activeObjects.reserve(m_gameObjects.size());

    for (const auto& [entity, gameObjectPtr] : m_gameObjects) {
        if (gameObjectPtr && gameObjectPtr->isActive) {
            activeObjects.push_back(gameObjectPtr.get());
        }
    }

    return activeObjects;
}

size_t GameObjectManager::getTotalGameObjects() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_gameObjects.size();
}
