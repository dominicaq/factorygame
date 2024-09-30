#ifndef RESOURCE_H
#define RESOURCE_H

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <cassert>

// Resource manager interface
class IResource {
public:
    virtual ~IResource() = default;
};

// Resource holder class
template<typename T>
class ResourceHolder : public IResource {
public:
    explicit ResourceHolder(T resource) : m_resource(std::move(resource)) {}
    T m_resource;
};

#endif // RESOURCE_H
