#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <iostream>

// Entity class to uniquely identify entities
class Entity {
public:
    int id;

    explicit Entity(int id) : id(id) {}
};

// EntityManager class to handle entity creation/destruction
class EntityManager {
public:
    Entity createEntity();
    size_t getNumEntities() const { return m_nextEntityId; }
    void destroyEntity(Entity entity);
    bool isActive(Entity entity) const;

private:
    int m_nextEntityId = 0;
    std::vector<bool> m_activeEntities;
};

#endif // ENTITY_H
