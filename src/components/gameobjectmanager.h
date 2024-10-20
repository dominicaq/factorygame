
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
    void removeGameObject(const Entity& entity);

    // Start() and Update() calls
    void startAll();
    void updateAll(float deltaTime);

    // Access all gameobjects
    std::vector<GameObject*> getActiveGameObjects() const;
    size_t getTotalGameObjects() const;

private:
    ECSWorld& m_world;
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    mutable std::mutex m_mutex;
};

#endif // GAMEOBJECTMANAGER_H
