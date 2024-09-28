#ifndef ECS_H
#define ECS_H

#include <vector>

#define NO_COMPONENT -1

// EntityManager class to manage entity lifecycle
class EntityManager {
public:
    int createEntity();
    void destroyEntity(int entity);
    bool isActive(int entity) const;

    int size() const { return static_cast<int>(m_active.size()); }
private:
    std::vector<bool> m_active;
};

// ComponentArray class to handle dense storage of components
template <typename T>
class ComponentArray {
public:
    void addComponent(int entity, const T& component);
    bool hasComponent(int entity) const;
    T& getComponent(int entity);
    const T& getComponent(int entity) const;

    int size() const { return static_cast<int>(m_components.size()); }

private:
    std::vector<T> m_components;
    // Maps entity ID to component index
    std::vector<int> m_entityToComponent;
};

#include "ecs.cpp"

#endif // ECS_H
