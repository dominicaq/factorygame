#ifndef ECS_H
#define ECS_H

#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <cassert>
#include <iostream>
#include <type_traits>
#include "mesh.h"

// Entity class to uniquely identify entities
class Entity {
public:
    int id;

    explicit Entity(int id) : id(id) {}
};

// Interface for component arrays (type erasure)
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
};

// Type trait to determine if a type should be stored as a pointer
template<typename T>
struct ShouldStoreAsPointer : std::false_type {};

// Specialization for Mesh type to be stored as a pointer
template<>
struct ShouldStoreAsPointer<Mesh> : std::true_type {};

// SparseArray for value types
template<typename T, typename Enabled = void>
class SparseArray : public IComponentArray {
public:
    SparseArray() = default;

    bool hasComponent(size_t entityId) const {
        return entityId < components.size() && components[entityId].has_value();
    }

    void addComponent(size_t entityId, const T& component) {
        if (entityId >= components.size()) {
            components.resize(entityId + 1);
        }
        components[entityId] = component;
    }

    T& getComponent(size_t entityId) {
        assert(hasComponent(entityId) && "Component does not exist!");
        return components[entityId].value();
    }

private:
    std::vector<std::optional<T>> components;
};

// SparseArray specialization for pointer types
template<typename T>
class SparseArray<T, typename std::enable_if<std::is_pointer_v<T>>::type> : public IComponentArray {
public:
    SparseArray() = default;

    bool hasComponent(size_t entityId) const {
        return entityId < components.size() && components[entityId] != nullptr;
    }

    void addComponent(size_t entityId, T component) {
        if (entityId >= components.size()) {
            components.resize(entityId + 1, nullptr);
        }
        components[entityId] = component;
    }

    T getComponent(size_t entityId) {
        assert(hasComponent(entityId) && "Component does not exist!");
        return components[entityId];
    }

private:
    std::vector<T> components;
};

// EntityManager class to handle entity creation/destruction
class EntityManager {
public:
    Entity createEntity();
    size_t getNumEntities() const { return nextEntityId; }
    void destroyEntity(Entity entity);
    bool isActive(Entity entity) const;

private:
    int nextEntityId = 0;
    std::vector<bool> activeEntities;
};

// ECSWorld class to manage components and handle queries
class ECSWorld {
public:
    Entity createEntity();

    template<typename T>
    void addComponent(Entity entity, const T& component) {
        using ComponentType = std::conditional_t<ShouldStoreAsPointer<T>::value, T*, T>;
        if constexpr (ShouldStoreAsPointer<T>::value) {
            getComponentArray<ComponentType>()->addComponent(entity.id, &component);
        } else {
            getComponentArray<ComponentType>()->addComponent(entity.id, component);
        }
    }

    template<typename T>
    auto getComponent(Entity entity) -> std::conditional_t<ShouldStoreAsPointer<T>::value, T*, T&> {
        using ComponentType = std::conditional_t<ShouldStoreAsPointer<T>::value, T*, T>;
        return getComponentArray<ComponentType>()->getComponent(entity.id);
    }

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
    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> componentArrays;

    template<typename T>
    SparseArray<T>* getComponentArray() {
        std::type_index index = std::type_index(typeid(T));
        auto it = componentArrays.find(index);
        if (it == componentArrays.end()) {
            auto newArray = std::make_unique<SparseArray<T>>();
            SparseArray<T>* ptr = newArray.get();
            componentArrays[index] = std::move(newArray);
            return ptr;
        } else {
            return static_cast<SparseArray<T>*>(it->second.get());
        }
    }

    template<typename T>
    const SparseArray<T>* getComponentArray() const {
        std::type_index index = std::type_index(typeid(T));
        auto it = componentArrays.find(index);
        if (it != componentArrays.end()) {
            return static_cast<const SparseArray<T>*>(it->second.get());
        } else {
            return nullptr;
        }
    }

    template<typename T>
    bool hasComponent(size_t entityId) const {
        using ComponentType = std::conditional_t<ShouldStoreAsPointer<T>::value, T*, T>;
        const SparseArray<ComponentType>* array = getComponentArray<ComponentType>();
        if (array == nullptr) {
            return false;
        }
        return array->hasComponent(entityId);
    }
};

#endif // ECS_H
