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

// Helper struct to create a non-deduced context
template <typename T>
struct non_deduced {};

// Helper to assign unique IDs to component types
inline size_t GetUniqueComponentTypeId() {
    static size_t id = 0;
    return id++;
}

template <typename T>
size_t GetComponentTypeId() {
    static size_t id = GetUniqueComponentTypeId();
    return id;
}

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

    Entity createEntity() {
        return m_entityManager.createEntity();
    }

    void destroyEntity(Entity entity) {
        m_entityManager.destroyEntity(entity);
        // Clear the entity's signature
        m_entitySignatures[entity.id].reset();

        // Remove all components associated with the entity
        for (auto& array : m_componentArrays) {
            if (array) {
                array->removeComponent(entity.id);
            }
        }
    }

    // **Explicitly add component stored by value**
    template<typename T>
    std::enable_if_t<!ShouldStoreAsPointer<T>::value, void>
    addComponent(Entity entity, const T& component, non_deduced<T> = {}) {
        getComponentArray<T>()->addComponent(entity.id, component);
        m_entitySignatures[entity.id].set(GetComponentTypeId<T>());
    }

    // **Explicitly add component stored by pointer**
    template<typename T>
    std::enable_if_t<ShouldStoreAsPointer<T>::value, void>
    addComponent(Entity entity, T* component, non_deduced<T> = {}) {
        getComponentArray<T>()->addComponent(entity.id, component);
        m_entitySignatures[entity.id].set(GetComponentTypeId<T>());
    }

    // **Add component by default constructor (for value types)**
    template<typename T>
    std::enable_if_t<!ShouldStoreAsPointer<T>::value, void>
    addComponent(Entity entity, non_deduced<T> = {}) {
        getComponentArray<T>()->addComponent(entity.id, T());
        m_entitySignatures[entity.id].set(GetComponentTypeId<T>());
    }

    // **Add component by default constructor (for pointer types)**
    template<typename T>
    std::enable_if_t<ShouldStoreAsPointer<T>::value, void>
    addComponent(Entity entity, non_deduced<T> = {}) {
        getComponentArray<T>()->addComponent(entity.id, new T());
        m_entitySignatures[entity.id].set(GetComponentTypeId<T>());
    }

    // Retrieve component
    template<typename T>
    auto getComponent(Entity entity) -> typename ComponentStorage<T>::StorageType {
        return getComponentArray<T>()->getComponent(entity.id);
    }

    // Remove component
    template<typename T>
    void removeComponent(Entity entity) {
        getComponentArray<T>()->removeComponent(entity.id);
        m_entitySignatures[entity.id].reset(GetComponentTypeId<T>());
    }

    // Check if entity ID has component
    template<typename T>
    bool hasComponent(Entity entity) const {
        return m_entitySignatures[entity.id].test(GetComponentTypeId<T>());
    }

    // Add a resource to the world
    template<typename T>
    void insertResource(T resource) {
        std::type_index index = std::type_index(typeid(T));
        m_resources[index] = std::make_unique<ResourceHolder<T>>(std::move(resource));
    }

    // Get a resource from the world
    template<typename T>
    T& getResource() {
        std::type_index index = std::type_index(typeid(T));
        auto it = m_resources.find(index);
        assert(it != m_resources.end() && "Resource not found!");
        return static_cast<ResourceHolder<T>*>(it->second.get())->m_resource;
    }

    template<typename... Components>
    std::vector<Entity> batchedQuery() const {
        // Set the bits corresponding to each requested component
        Signature querySignature;
        (querySignature.set(GetComponentTypeId<Components>()) , ...);

        // Iterate through all entities, check their signatures
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

    // Component arrays, indexed by component type ID
    std::vector<std::unique_ptr<IComponentArray>> m_componentArrays;
    std::unordered_map<std::type_index, std::unique_ptr<IResource>> m_resources;

    // Track signatures of entities, which components they have
    std::vector<Signature> m_entitySignatures;

    // Get or create component array for a type
    template<typename T>
    ComponentArray<T>* getComponentArray() {
        size_t index = GetComponentTypeId<T>();
        if (index >= m_componentArrays.size()) {
            m_componentArrays.resize(index + 1);
        }

        if (!m_componentArrays[index]) {
            m_componentArrays[index] = std::make_unique<ComponentArray<T>>();
        }

        return static_cast<ComponentArray<T>*>(m_componentArrays[index].get());
    }

    // Const version of getComponentArray
    template<typename T>
    const ComponentArray<T>* getComponentArray() const {
        size_t index = GetComponentTypeId<T>();
        if (index < m_componentArrays.size() && m_componentArrays[index]) {
            return static_cast<const ComponentArray<T>*>(m_componentArrays[index].get());
        } else {
            return nullptr;
        }
    }

    // Resize all component arrays when EntityManager resizes
    void resizeAllComponentArrays(size_t newSize) {
        for (auto& array : m_componentArrays) {
            if (array) {
                array->resize(newSize);
            }
        }
        m_entitySignatures.resize(newSize);
    }
};

#endif // ECSWORLD_H
