#include "ecs.h"
#include <iostream>

/*
 * EntityManager implementation
 */

Entity EntityManager::createEntity() {
    int id = m_nextEntityId++;
    m_activeEntities.push_back(true);
    return Entity(id);
}

void EntityManager::destroyEntity(Entity entity) {
    if (entity.id < m_activeEntities.size()) {
        m_activeEntities[entity.id] = false;
    } else {
        std::cerr << "[Error] EntityManager::destroyEntity: Invalid entity ID " << entity.id << "!\n";
    }
}

bool EntityManager::isActive(Entity entity) const {
    if (entity.id >= m_activeEntities.size()) {
        std::cerr << "[Error] EntityManager::isActive: Invalid entity ID " << entity.id << "!\n";
        return false;
    }
    return m_activeEntities[entity.id];
}

/*
 * ECSWorld implementation
 */

Entity ECSWorld::createEntity() {
    return m_entityManager.createEntity();
}