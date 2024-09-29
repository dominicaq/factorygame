#ifndef ECS_H
#define ECS_H

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <cassert>
#include <iostream>

// Entity class to uniquely identify entities
class Entity {
public:
    int id;

    // Prevent setting of id = num in user code
    explicit Entity(int id) : id(id) {}
};

// Archetype class to group entities based on their component sets
class Archetype {
public:
    std::vector<int> entities;
    std::unordered_map<std::type_index, std::vector<void*>> components;

    void addEntity(int entity) {
        entities.push_back(entity);
    }

    template<typename T>
    void addComponent(int entity, T* component) {
        if (components.find(typeid(T)) == components.end()) {
            components[typeid(T)] = std::vector<void*>();
        }
        // Store component
        components[typeid(T)].push_back(static_cast<void*>(component));
    }

    template<typename T>
    T* getComponent(int entity) {
        if (components.find(typeid(T)) == components.end()) {
            std::cerr << "[Error] Archetype::getComponent: Component not found for entity " << entity << "!\n";
            return nullptr;
        }

        int index = 0;
        for (auto& ent : entities) {
            if (ent == entity) break;
            index++;
        }

        return static_cast<T*>(components[typeid(T)][index]);
    }
};

// EntityManager class to handle entity creation/destruction
class EntityManager {
public:
    Entity createEntity();
    void destroyEntity(Entity entity);
    bool isActive(Entity entity) const;

private:
    int nextEntityId = 0;
    std::vector<bool> m_active;
};

// ECSWorld class to manage archetypes and handle queries
class ECSWorld {
public:
    Entity createEntity();

    template<typename T>
    void addComponent(Entity entity, T component) {
        if (archetypes.find(entity.id) == archetypes.end()) {
            archetypes[entity.id] = Archetype();
        }

        archetypes[entity.id].addEntity(entity.id);
        T* componentPtr = new T(component);
        archetypes[entity.id].addComponent<T>(entity.id, componentPtr);
    }

    template<typename... Components>
    std::vector<Entity> queryEntities() {
        std::vector<Entity> result;
        for (auto& [entityId, archetype] : archetypes) {
            bool matches = (... && (archetype.components.find(typeid(Components)) != archetype.components.end()));
            if (matches) {
                result.push_back(Entity(entityId));
            }
        }
        return result;
    }

    template<typename T>
    T& getComponent(Entity entity) {
        T* component = archetypes[entity.id].getComponent<T>(entity.id);
        if (!component) {
            std::cerr << "[Error] ECSWorld::getComponent: Component not found for entity " << entity.id << "!\n";
            static T emptyComponent;
            return emptyComponent;
        }
        return *component;
    }

private:
    std::unordered_map<int, Archetype> archetypes;  // Store entities grouped by component sets
    EntityManager entityManager;
};

#endif // ECS_H
