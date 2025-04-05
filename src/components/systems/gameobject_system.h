#ifndef GAMEOBJECTSYSTEM_H
#define GAMEOBJECTSYSTEM_H

#include "../gameobject.h"
#include <entt/entt.hpp>
#include <vector>

class GameObjectSystem {
public:
    GameObjectSystem(entt::registry& registry);
    ~GameObjectSystem();

    void setOnDirtyInstanceCallback(std::function<void()> callback);

    // Start() and Update() calls
    void startAll();
    void updateAll(const float& currentTime, const float& deltaTime);

    // Access all gameobjects
    std::vector<GameObject*> getActiveGameObjects() const;

private:
    entt::registry& m_registry;
    std::function<void()> m_onDirtyInstance;
};

#endif // GameObjectSystem_H
