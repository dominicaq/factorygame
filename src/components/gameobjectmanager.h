
#ifndef GAMEOBJECTMANAGER_H
#define GAMEOBJECTMANAGER_H

#include "gameobject.h"
#include "ecs/entity.h"
#include <vector>
#include <memory>
#include <stack>
#include <mutex>

class GameObjectManager {
public:
    // Constructor
    GameObjectManager(ECSWorld& world);

    // Destructor
    ~GameObjectManager();

    // Gameobject management
    GameObject* createGameObject(const Entity& entity, bool runScripts = false);
    GameObject* getGameObject(const Entity& entity) const;
    bool removeGameObject(const Entity& entity);

    // Start() and Update() calls
    void startAll();
    void updateAll(float deltaTime);

    // Access all gameobjects
    std::vector<GameObject*> getActiveGameObjects() const;
    size_t getTotalGameObjects() const;

private:
    ECSWorld& m_world;
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    std::stack<int> m_freeList;  // Stack to store free Entity IDs
    mutable std::mutex m_mutex;  // For thread-safety
};

#endif // GAMEOBJECTMANAGER_H
