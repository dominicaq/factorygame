#include "ecs.h"
#include <iostream>

/*
 * EntityManager implementation
 */

Entity EntityManager::createEntity() {
    int id = nextEntityId++;
    m_active.push_back(true);
    return Entity(id);
}

void EntityManager::destroyEntity(Entity entity) {
    if (entity.id < m_active.size()) {
        m_active[entity.id] = false;
    } else {
        std::cerr << "[Error] EntityManager::destroyEntity: Invalid entity ID " << entity.id << "!\n";
    }
}

bool EntityManager::isActive(Entity entity) const {
    if (entity.id >= m_active.size()) {
        std::cerr << "[Error] EntityManager::isActive: Invalid entity ID " << entity.id << "!\n";
        return false;
    }
    return m_active[entity.id];
}

/*
 * ECSWorld implementation (non-template functions)
 */

Entity ECSWorld::createEntity() {
    return entityManager.createEntity();
}
