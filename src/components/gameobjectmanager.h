#ifndef GAMEOBJECTMANAGER_H
#define GAMEOBJECTMANAGER_H

#include "gameobject.h"
#include <entt/entt.hpp>
#include <unordered_map>
#include <memory>
#include <mutex>

class GameObjectManager {
public:
    // Constructor
    GameObjectManager(entt::registry& registry);

    // Destructor
    ~GameObjectManager();

    // GameObject management
    GameObject* createGameObject(entt::entity entity, bool runScripts = false);
    GameObject* getGameObject(entt::entity entity) const;
    void removeGameObject(entt::entity entity);

    // Start() and Update() calls
    void startAll();
    void updateAll(float deltaTime);

    // Access all gameobjects
    std::vector<GameObject*> getActiveGameObjects() const;
    size_t getTotalGameObjects() const;

private:
    entt::registry& m_registry;
    std::unordered_map<entt::entity, std::unique_ptr<GameObject>> m_gameObjects;
    mutable std::mutex m_mutex;
};

#endif // GAMEOBJECTMANAGER_H
