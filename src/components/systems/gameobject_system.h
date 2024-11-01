#ifndef GAMEOBJECTSYSTEM_H
#define GAMEOBJECTSYSTEM_H

#include "../gameobject.h"
#include <entt/entt.hpp>
#include <vector>

class GameObjectSystem {
public:
    GameObjectSystem(entt::registry& registry);
    ~GameObjectSystem();

    // GameObject management
    GameObject* getGameObject(entt::entity entity) const;
    void removeGameObject(entt::entity entity);

    // Start() and Update() calls
    void startAll();
    void updateAll(float deltaTime);

    // Access all gameobjects
    std::vector<GameObject*> getActiveGameObjects() const;

private:
    entt::registry& m_registry;
};

#endif // GameObjectSystem_H
