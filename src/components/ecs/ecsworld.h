#ifndef ECSWORLD_H
#define ECSWORLD_H

#include "entity.h"
#include "component_array.h"
#include "resource.h"

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>

class ECSWorld {
public:
    ECSWorld() = default;

    ~ECSWorld() {
        // Clean up resources
        m_componentArrays.clear();
        m_resources.clear();
    }

    Entity createEntity();

    // Add component stored by value
    template<typename T>
    std::enable_if_t<!ShouldStoreAsPointer<T>::value, void>
    addComponent(Entity entity, const T& component) {
        getComponentArray<T>()->addComponent(entity.id, component);
    }

    // Add component stored by pointer
    template<typename T>
    std::enable_if_t<ShouldStoreAsPointer<T>::value, void>
    addComponent(Entity entity, T* component) {
        getComponentArray<T>()->addComponent(entity.id, component);
    }

    // Retrieve component
    template<typename T>
    auto getComponent(Entity entity) -> typename ComponentStorage<T>::StorageType {
        return getComponentArray<T>()->getComponent(entity.id);
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

    // Component queries
    template<typename... Components>
    std::vector<Entity> batchedQuery() const {
        std::vector<Entity> matchingEntities;

        // Iterate through entities and check if all requested components exist
        for (size_t entityId = 0; entityId < m_entityManager.getNumEntities(); ++entityId) {
            if ((hasComponent<Components>(entityId) && ...)) {
                matchingEntities.emplace_back(entityId);
            }
        }

        return matchingEntities;
    }

private:
    EntityManager m_entityManager;

    // Component arrays, indexed by typeid
    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> m_componentArrays;

    // Resource storage
    std::unordered_map<std::type_index, std::unique_ptr<IResource>> m_resources;

    // Get or create component array for a type
    template<typename T>
    SparseArray<T>* getComponentArray() {
        std::type_index index = std::type_index(typeid(T));
        auto it = m_componentArrays.find(index);
        if (it == m_componentArrays.end()) {
            auto newArray = std::make_unique<SparseArray<T>>();
            SparseArray<T>* ptr = newArray.get();
            m_componentArrays[index] = std::move(newArray);
            return ptr;
        } else {
            return static_cast<SparseArray<T>*>(it->second.get());
        }
    }

    // Const version of getComponentArray
    template<typename T>
    const SparseArray<T>* getComponentArray() const {
        std::type_index index = std::type_index(typeid(T));
        auto it = m_componentArrays.find(index);
        if (it != m_componentArrays.end()) {
            return static_cast<const SparseArray<T>*>(it->second.get());
        } else {
            return nullptr;
        }
    }

    // Check if an entity has a specific component
    template<typename T>
    bool hasComponent(size_t entityId) const {
        const SparseArray<T>* array = getComponentArray<T>();
        if (array == nullptr) {
            return false;
        }
        return array->hasComponent(entityId);
    }
};

#endif // ECSWORLD_H
