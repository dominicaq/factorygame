#ifndef ECSWORLD_H
#define ECSWORLD_H

#include "entity.h"
#include "component_array.h"
#include "resource.h"

#include <vector>
#include <typeindex>
#include <memory>
#include <cassert>

// Resource map
#include <unordered_map>

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
        // Remove all components associated with the entity
        for (auto& array : m_componentArrays) {
            if (array) {
                array->removeComponent(entity.id);
            }
        }
    }

    // Delete the function that allows implicit type deduction for value and universal references
    template<typename T>
    void addComponent(Entity entity, T&& component) = delete;

    // **Explicitly add component stored by value**
    template<typename T>
    std::enable_if_t<!ShouldStoreAsPointer<T>::value, void>
    addComponent(Entity entity, const T& component, non_deduced<T> = {}) {
        getComponentArray<T>()->addComponent(entity.id, component);
    }

    // **Explicitly add component stored by pointer**
    template<typename T>
    std::enable_if_t<ShouldStoreAsPointer<T>::value, void>
    addComponent(Entity entity, T* component, non_deduced<T> = {}) {
        getComponentArray<T>()->addComponent(entity.id, component);
    }

    // **Add component by default constructor (for value types)**
    template<typename T>
    std::enable_if_t<!ShouldStoreAsPointer<T>::value, void>
    addComponent(Entity entity, non_deduced<T> = {}) {
        getComponentArray<T>()->addComponent(entity.id, T());
    }

    // **Add component by default constructor (for pointer types)**
    template<typename T>
    std::enable_if_t<ShouldStoreAsPointer<T>::value, void>
    addComponent(Entity entity, non_deduced<T> = {}) {
        getComponentArray<T>()->addComponent(entity.id, new T());
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

    // **Batched Query: Retrieves entities that have all of the specified components**
    template<typename... Components>
    std::vector<Entity> batchedQuery() const {
        std::vector<Entity> matchingEntities;

        // Iterate through all entities
        for (size_t entityId = 0; entityId < m_entityManager.getNumEntities(); ++entityId) {
            if ((hasComponent<Components>(entityId) && ...)) {
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

    // Check if an entity has a specific component
    template<typename T>
    bool hasComponent(size_t entityId) const {
        const ComponentArray<T>* array = getComponentArray<T>();
        if (array == nullptr) {
            return false;
        }
        return array->hasComponent(entityId);
    }

    // Resize all component arrays when EntityManager resizes
    void resizeAllComponentArrays(size_t newSize) {
        for (auto& array : m_componentArrays) {
            if (array) {
                array->resize(newSize);
            }
        }
    }
};

#endif // ECSWORLD_H
