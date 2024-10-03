#ifndef COMPONENT_ARRAY_H
#define COMPONENT_ARRAY_H

#include "component_traits.h"

#include <vector>
#include <optional>
#include <iostream>
#include <mutex>

template<typename T>
using OptionalType = std::optional<T>;

// Interface for component arrays
class IComponentArray {
public:
    virtual ~IComponentArray() = default;

    virtual void removeComponent(size_t entityId) = 0;
    virtual void resize(size_t newSize) = 0;
};

// ComponentArray manages storage of components, either by value or by pointer
template<typename T>
class ComponentArray : public IComponentArray {
public:
    using StorageType = typename ComponentStorage<T>::StorageType;

    ComponentArray(size_t initialSize = 100) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_components.resize(initialSize);
    }

    bool hasComponent(size_t entityId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return entityId < m_components.size() && m_components[entityId].has_value();
    }

    // Add component stored by value
    template<typename U = T>
    std::enable_if_t<!ShouldStoreAsPointer<U>::value, void>
    addComponent(size_t entityId, const U& component) {
        std::lock_guard<std::mutex> lock(m_mutex);
        ensureCapacity(entityId);
        m_components[entityId] = component;
    }

    // Add component stored by pointer
    template<typename U = T>
    std::enable_if_t<ShouldStoreAsPointer<U>::value, void>
    addComponent(size_t entityId, U* component) {
        std::lock_guard<std::mutex> lock(m_mutex);
        ensureCapacity(entityId);
        m_components[entityId] = component;
    }

    // Retrieve component
    StorageType getComponent(size_t entityId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (entityId >= m_components.size() || !m_components[entityId].has_value()) {
            if constexpr (ShouldStoreAsPointer<T>::value) {
                return nullptr;
            } else {
                throw std::runtime_error("Component not found!");
            }
        }
        return m_components[entityId].value();
    }

    // Remove component
    void removeComponent(size_t entityId) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (entityId < m_components.size()) {
            m_components[entityId].reset();
        }
    }

    // Resize method called by the EntityManager
    void resize(size_t newSize) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_components.resize(newSize);
    }

private:
    using OptionalComponentType = std::conditional_t<ShouldStoreAsPointer<T>::value, T*, T>;
    using OptionalComponent = OptionalType<OptionalComponentType>;

    std::vector<OptionalComponent> m_components;
    mutable std::mutex m_mutex;

    // Ensure the component array is large enough to hold the entityId
    void ensureCapacity(size_t entityId) {
        if (entityId >= m_components.size()) {
            size_t newSize = std::max(m_components.size() * 2, size_t(1));
            m_components.resize(newSize);
        }
    }
};

#endif // COMPONENT_ARRAY_H
