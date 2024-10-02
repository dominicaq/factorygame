#ifndef COMPONENT_TRAITS_H
#define COMPONENT_TRAITS_H

#include <type_traits>

// Alias for std::false_type (default: components are stored by value)
template<typename T>
using StoreAsPointer = std::false_type;

// Default: Components are stored by value
template<typename T>
struct ShouldStoreAsPointer : StoreAsPointer<T> {};

// Alias for conditional storage type (pointer or reference)
template<typename T>
using ConditionalStorage = std::conditional_t<ShouldStoreAsPointer<T>::value, T*, T&>;

// ComponentStorage determines the storage type based on ShouldStoreAsPointer
template<typename T>
struct ComponentStorage {
    using StorageType = ConditionalStorage<T>;
};

#endif // COMPONENT_TRAITS_H
