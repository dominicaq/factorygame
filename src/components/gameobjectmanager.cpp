
#include "gameobjectmanager.h"
#include <iostream>

GameObjectManager::GameObjectManager(ECSWorld& world) : m_world(world) {
    m_gameObjects.reserve(100);
}

GameObjectManager::~GameObjectManager() {}

GameObject* GameObjectManager::createGameObject(const Entity& entity, bool runScripts) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Check if we need to resize the vector to accommodate the entity ID
    if (entity.id >= static_cast<int>(m_gameObjects.size())) {
        if (entity.id >= static_cast<int>(m_gameObjects.capacity())) {
            // Double the capacity when it's reached
            size_t newCapacity = std::max(m_gameObjects.capacity() * 2, size_t(1));
            m_gameObjects.reserve(newCapacity);
            std::cerr << "[Warning] GameObjectManager vector capacity doubled to " << newCapacity << "." << std::endl;
        }
        // Resize the vector to ensure entity.id is a valid index
        m_gameObjects.resize(entity.id + 1);
    }

    // Create and store the GameObject
    m_gameObjects[entity.id] = std::make_unique<GameObject>(entity, &m_world);

    // Initialize the GameObject (call startScripts)
    if (runScripts && m_gameObjects[entity.id]->isActive) {
        m_gameObjects[entity.id]->startScripts();
    }

    // Return the GameObject so scripts can be added
    return m_gameObjects[entity.id].get();
}

// Retrieve a GameObject by its entity ID (Entity used as index)
GameObject* GameObjectManager::getGameObject(const Entity& entity) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Ensure the entity ID is valid
    if (entity.id < static_cast<int>(m_gameObjects.size()) && m_gameObjects[entity.id]) {
        return m_gameObjects[entity.id].get();
    }

    return nullptr;  // Entity ID is invalid or GameObject does not exist
}

// Remove a GameObject by its entity ID
void GameObjectManager::removeGameObject(const Entity& entity) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (entity.id >= m_gameObjects.size() || !m_gameObjects[entity.id]) {
        std::cerr << "[Warning]: GameObjectManager::removeGameObject: Invalid ID: " << entity.id << ".\n";
        return;
    }

    // Perform any necessary cleanup on the GameObject
    m_gameObjects[entity.id].reset();  // Destroy the GameObject
    m_freeList.push(entity.id);        // Add the slot to the free list
    m_world.destroyEntity(entity);     // Remove from ECSWorld
}

// Start all active GameObjects' scripts
void GameObjectManager::startAll() {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& gameObjectPtr : m_gameObjects) {
        if (gameObjectPtr && gameObjectPtr->isActive) {
            gameObjectPtr->startScripts();
        }
    }
}

// Update all active GameObjects' scripts
void GameObjectManager::updateAll(float deltaTime) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& gameObjectPtr : m_gameObjects) {
        if (gameObjectPtr && gameObjectPtr->isActive) {
            gameObjectPtr->updateScripts(deltaTime);
        }
    }
}

// Get all active GameObjects (for rendering, etc.)
std::vector<GameObject*> GameObjectManager::getActiveGameObjects() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<GameObject*> activeObjects;
    activeObjects.reserve(m_gameObjects.size());

    for (const auto& gameObjectPtr : m_gameObjects) {
        if (gameObjectPtr && gameObjectPtr->isActive) {
            activeObjects.push_back(gameObjectPtr.get());
        }
    }

    return activeObjects;
}

// Get the total number of GameObjects (including inactive)
size_t GameObjectManager::getTotalGameObjects() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_gameObjects.size();
}
