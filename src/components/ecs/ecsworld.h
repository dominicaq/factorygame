#ifndef ECSWORLD_H
#define ECSWORLD_H

#include "entity.h"
#include "component_array.h"
#include "resource.h"

#include <vector>
#include <typeindex>
#include <memory>
#include <bitset>
#include <cassert>

// Signature will represent the components an entity has
constexpr size_t MAX_COMPONENTS = 64;
using Signature = std::bitset<MAX_COMPONENTS>;

class ECSWorld {
public:
    ECSWorld() {
        // Initialize entity signatures
        m_entitySignatures.resize(100);

        // Register callback with EntityManager to resize component arrays
        m_entityManager.registerResizeCallback([this](size_t newSize) {
            resizeAllComponentArrays(newSize);
        });
    }

    ~ECSWorld() {
        // Clean up resources
        m_componentArrays.clear();
        m_resources.clear();
    }

    // Create a new entity
    Entity createEntity() {
        return m_entityManager.createEntity();
    }

    // Destroy an entity and remove its components
    void destroyEntity(Entity entity) {
        if (!validateEntity(entity)) return;

        m_entityManager.destroyEntity(entity);
        m_entitySignatures[entity.id].reset();

        // Remove all components associated with the entity
        for (auto& array : m_componentArrays) {
            if (array) {
                array->removeComponent(entity.id);
            }
        }
    }

    // Add component stored by value
    template<typename T>
    void addComponent(Entity entity, const T& component) {
        if (!validateEntity(entity)) return;

        getComponentArray<T>()->addComponent(entity.id, component);
        m_entitySignatures[entity.id].set(getComponentTypeId<T>());
    }

    // Add component stored by pointer
    template<typename T>
    void addComponent(Entity entity, T* component) {
        if (!validateEntity(entity)) return;

        getComponentArray<T>()->addComponent(entity.id, component);
        m_entitySignatures[entity.id].set(getComponentTypeId<T>());
    }

    // Add component by default constructor
    template<typename T>
    void addComponent(Entity entity) {
        if (!validateEntity(entity)) return;

        getComponentArray<T>()->addComponent(entity.id, T());
        m_entitySignatures[entity.id].set(getComponentTypeId<T>());
    }

    // Retrieve component
    template<typename T>
    auto getComponent(Entity entity) -> typename ComponentStorage<T>::StorageType {
        if (!validateEntity(entity)) {
            throw std::runtime_error("Invalid entity");
        }

        return getComponentArray<T>()->getComponent(entity.id);
    }

    // Remove component
    template<typename T>
    void removeComponent(Entity entity) {
        if (!validateEntity(entity)) return;

        getComponentArray<T>()->removeComponent(entity.id);
        m_entitySignatures[entity.id].reset(getComponentTypeId<T>());
    }

    // Check if entity has component
    template<typename T>
    bool hasComponent(Entity entity) const {
        return validateEntity(entity) && m_entitySignatures[entity.id].test(getComponentTypeId<T>());
    }

    // Add resource to the world
    template<typename T>
    void insertResource(T resource) {
        m_resources[std::type_index(typeid(T))] = std::make_unique<ResourceHolder<T>>(std::move(resource));
    }

    // Get resource from the world
    template<typename T>
    T& getResource() {
        auto it = m_resources.find(std::type_index(typeid(T)));
        assert(it != m_resources.end() && "Resource not found!");
        return static_cast<ResourceHolder<T>*>(it->second.get())->m_resource;
    }

    // Batched query for entities with specific components
    template<typename... Components>
    std::vector<Entity> batchedQuery() const {
        Signature querySignature;
        (querySignature.set(getComponentTypeId<Components>()), ...);

        std::vector<Entity> matchingEntities;
        for (size_t entityId = 0; entityId < m_entityManager.getNumEntities(); ++entityId) {
            if ((m_entitySignatures[entityId] & querySignature) == querySignature) {
                matchingEntities.emplace_back(entityId);
            }
        }

        return matchingEntities;
    }

private:
    EntityManager m_entityManager;
    std::vector<std::unique_ptr<IComponentArray>> m_componentArrays;
    std::unordered_map<std::type_index, std::unique_ptr<IResource>> m_resources;
    std::vector<Signature> m_entitySignatures;

    // Helper to validate entity before performing actions
    bool validateEntity(Entity entity) const {
        if (!entity.isValid()) {
            std::cerr << "[Error] ECSWorld: Invalid entity!\n";
            return false;
        }
        return true;
    }

    // Helper to get or create a component array for a type
    template<typename T>
    ComponentArray<T>* getComponentArray() {
        size_t index = getComponentTypeId<T>();
        if (index >= m_componentArrays.size()) {
            m_componentArrays.resize(index + 1);
        }

        if (!m_componentArrays[index]) {
            m_componentArrays[index] = std::make_unique<ComponentArray<T>>();
        }

        return static_cast<ComponentArray<T>*>(m_componentArrays[index].get());
    }

    // Helper to resize all component arrays when EntityManager resizes
    void resizeAllComponentArrays(size_t newSize) {
        for (auto& array : m_componentArrays) {
            if (array) {
                array->resize(newSize);
            }
        }
        m_entitySignatures.resize(newSize);
    }

    // Helper to get component type ID using static variable for uniqueness
    template<typename T>
    static size_t getComponentTypeId() {
        static size_t typeId = GetNextComponentTypeId();
        return typeId;
    }

    static size_t GetNextComponentTypeId() {
        static size_t nextId = 0;
        assert(nextId < MAX_COMPONENTS && "Exceeded maximum number of component types!");
        return nextId++;
    }
};

#endif // ECSWORLD_H
