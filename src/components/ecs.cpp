#include "ecs.h"

/*
* EntityManager implementation
*/

inline int EntityManager::createEntity() {
    int id = static_cast<int>(m_active.size());
    m_active.push_back(true);
    return id;
}

inline void EntityManager::destroyEntity(int entity) {
    if (entity < m_active.size() && m_active[entity]) {
        m_active[entity] = false;
    }
}

inline bool EntityManager::isActive(int entity) const {
    return entity < m_active.size() && m_active[entity];
}

/*
* ComponentArray implementation
*/

template <typename T>
inline void ComponentArray<T>::addComponent(int entity, const T& component) {
    // Resize mappings if necessary
    if (entity >= m_entityToComponent.size()) {
        m_entityToComponent.resize(entity + 1, NO_COMPONENT);
    }

    // Add component to dense array
    m_entityToComponent[entity] = static_cast<int>(m_components.size());
    m_components.push_back(component);
}

template <typename T>
inline bool ComponentArray<T>::hasComponent(int entity) const {
    return entity < m_entityToComponent.size() && m_entityToComponent[entity] != NO_COMPONENT;
}

template <typename T>
inline T& ComponentArray<T>::getComponent(int entity) {
    return m_components[m_entityToComponent[entity]];
}

template <typename T>
inline const T& ComponentArray<T>::getComponent(int entity) const {
    return m_components[m_entityToComponent[entity]];
}
