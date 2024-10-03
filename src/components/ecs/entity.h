#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <iostream>
#include <functional>

// Entity class
class Entity {
public:
    int id;

    Entity() : id(-1) {}

    explicit Entity(int id) : id(id) {}

    bool isValid() const {
        return id >= 0;
    }
};

// EntityManager class
class EntityManager {
public:
    using ResizeCallback = std::function<void(size_t)>;

    EntityManager(size_t initialSize = 100) {
        m_activeEntities.resize(initialSize);
    }

    // Create a new entity
    Entity createEntity() {
        if (!m_freeIds.empty()) {
            int id = m_freeIds.back();
            m_freeIds.pop_back();
            m_activeEntities[id] = true;
            return Entity(id);
        } else {
            if (m_nextEntityId >= m_activeEntities.size()) {
                resizeEntities();
            }
            int id = m_nextEntityId++;
            m_activeEntities[id] = true;
            return Entity(id);
        }
    }

    // Destroy an entity
    void destroyEntity(Entity entity) {
        if (entity.isValid() && entity.id < m_activeEntities.size() && m_activeEntities[entity.id]) {
            m_activeEntities[entity.id] = false;
            m_freeIds.push_back(entity.id);  // Reuse entity ID later
        } else {
            std::cerr << "[Error] EntityManager::destroyEntity: Invalid entity ID " << entity.id << "!\n";
        }
    }

    // Check if an entity is active
    bool isActive(Entity entity) const {
        if (!entity.isValid() || entity.id >= m_activeEntities.size()) {
            std::cerr << "[Error] EntityManager::isActive: Invalid entity ID " << entity.id << "!\n";
            return false;
        }
        return m_activeEntities[entity.id];
    }

    size_t getNumEntities() const { return m_nextEntityId; }

    // Register callback to notify other systems when resizing entity storage
    void registerResizeCallback(ResizeCallback callback) {
        m_resizeCallback = std::move(callback);
    }

private:
    int m_nextEntityId = 0;
    std::vector<bool> m_activeEntities;
    std::vector<int> m_freeIds;
    ResizeCallback m_resizeCallback;

    // Resize entity storage and notify other systems
    void resizeEntities() {
        size_t newSize = std::max(m_activeEntities.size() * 2, size_t(1));
        std::cerr << "[Warning] Resizing entity manager from " << m_activeEntities.size() << " to " << newSize << ".\n";
        m_activeEntities.resize(newSize);

        // Notify other systems to resize their data structures
        if (m_resizeCallback) {
            m_resizeCallback(newSize);
        }
    }
};

#endif // ENTITY_H
