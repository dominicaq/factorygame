#ifndef COMPONENT_ARRAY_H
#define COMPONENT_ARRAY_H

#include <vector>
#include <optional>
#include <iostream>
#include "component_traits.h"

// Type alias for common std::optional use
template<typename T>
using OptionalType = std::optional<T>;

// Interface for component arrays (type erasure)
class IComponentArray {
public:
    virtual ~IComponentArray() = default;

    // Virtual resize method to resize the array when requested by the EntityManager
    virtual void resize(size_t newSize) = 0;
};

// ComponentArray manages storage of components, either by value or by pointer
template<typename T>
class ComponentArray : public IComponentArray {
public:
    using StorageType = typename ComponentStorage<T>::StorageType;

    ComponentArray(size_t initialSize = 100) {
        m_components.resize(initialSize);
    }

    bool hasComponent(size_t entityId) const {
        return entityId < m_components.size() && m_components[entityId].has_value();
    }

    // Add component stored by value
    template<typename U = T>
    std::enable_if_t<!ShouldStoreAsPointer<U>::value, void>
    addComponent(size_t entityId, const U& component) {
        ensureCapacity(entityId);
        m_components[entityId] = component;
    }

    // Add component stored by pointer
    template<typename U = T>
    std::enable_if_t<ShouldStoreAsPointer<U>::value, void>
    addComponent(size_t entityId, U* component) {
        ensureCapacity(entityId);
        m_components[entityId] = component;
    }

    // Retrieve component
    StorageType getComponent(size_t entityId) {
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
    void removeComponent(size_t entityId) {
        if (entityId < m_components.size()) {
            m_components[entityId].reset();
        }
    }

    // Resize method called by the EntityManager
    void resize(size_t newSize) override {
        m_components.resize(newSize);
    }

private:
    using OptionalComponentType = std::conditional_t<ShouldStoreAsPointer<T>::value, T*, T>;
    using OptionalComponent = OptionalType<OptionalComponentType>;

    std::vector<OptionalComponent> m_components;  // Component storage

    // Ensure the component array is large enough to hold the entityId
    void ensureCapacity(size_t entityId) {
        if (entityId >= m_components.size()) {
            resizeComponentArray();
        }
    }

    // Resize component array by doubling its size
    void resizeComponentArray() {
        size_t newSize = std::max(m_components.size() * 2, size_t(1));
        m_components.resize(newSize);
    }
};

#endif // COMPONENT_ARRAY_H
