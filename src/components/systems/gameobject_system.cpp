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

    // If any objects called to destroy themselves, we clean them up
    bool updateInstanceCounts = false;
    for (const auto& [entity] : m_registry.view<PendingDestroy>().each()) {
        if (m_registry.valid(entity)) {
            if (m_registry.any_of<MeshInstance>(entity)) {
                updateInstanceCounts = true;
            }

            m_registry.destroy(entity);
        }
    }

    if (updateInstanceCounts && m_onDirtyInstance) {
        m_onDirtyInstance();
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

void GameObjectSystem::setOnDirtyInstanceCallback(std::function<void()> callback) {
    m_onDirtyInstance = std::move(callback);
}
