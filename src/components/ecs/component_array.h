// ComponentArray.h
#ifndef COMPONENT_ARRAY_H
#define COMPONENT_ARRAY_H

#include <vector>
#include <optional>
#include <cassert>
#include "component_traits.h"

// Interface for component arrays (type erasure)
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
};

// SparseArray manages storage of components, either by value or by pointer
template<typename T>
class SparseArray : public IComponentArray {
public:
    using StorageType = typename ComponentStorage<T>::StorageType;

    SparseArray() = default;

    bool hasComponent(size_t entityId) const {
        return entityId < m_components.size() && m_components[entityId].has_value();
    }

    // Add component stored by value
    template<typename U = T>
    std::enable_if_t<!ShouldStoreAsPointer<U>::value, void>
    addComponent(size_t entityId, const U& component) {
        if (entityId >= m_components.size()) {
            m_components.resize(entityId + 1);
        }
        m_components[entityId] = component;
    }

    // Add component stored by pointer
    template<typename U = T>
    std::enable_if_t<ShouldStoreAsPointer<U>::value, void>
    addComponent(size_t entityId, U* component) {
        if (entityId >= m_components.size()) {
            m_components.resize(entityId + 1);
        }
        m_components[entityId] = component;
    }

    // Retrieve component
    StorageType getComponent(size_t entityId) {
        assert(hasComponent(entityId) && "Component does not exist!");
        return m_components[entityId].value();
    }

private:
    using OptionalType = std::conditional_t<ShouldStoreAsPointer<T>::value, T*, T>;
    std::vector<std::optional<OptionalType>> m_components;
};

#endif // COMPONENT_ARRAY_H
