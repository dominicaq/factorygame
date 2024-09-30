#ifndef COMPONENT_TRAITS_H
#define COMPONENT_TRAITS_H

#include <type_traits>

// Default: Components are stored by value
template<typename T>
struct ShouldStoreAsPointer : std::false_type {};

// ComponentStorage determines the storage type based on ShouldStoreAsPointer
template<typename T>
struct ComponentStorage {
    using StorageType = std::conditional_t<ShouldStoreAsPointer<T>::value, T*, T&>;
};

#endif // COMPONENT_TRAITS_H
