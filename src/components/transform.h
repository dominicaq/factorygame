#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "ecs/entity.h"
#include "ecs/ecsworld.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include <vector>

// Forward declare non user functions
inline void updateModelMatrices(ECSWorld& world, const std::vector<Entity>& entities);
inline void markChildrenDirty(ECSWorld& world, const Entity& parent);

#pragma region Transform Components
struct ModelMatrix {
    // TODO: Potential performance gain in the future
    // make a new bitset component to hold state for things like this
    // will shrink struct from 68 bytes to 64
    glm::mat4 matrix = glm::mat4(1.0f);
    bool dirty = true;
};

// Define transformation components for world space
struct Position {
    glm::vec3 position = glm::vec3(0.0f);
};

struct Rotation {
    glm::quat quaternion = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
};

// Euler angles in degrees (pitch, yaw, roll)
struct EulerAngles {
    glm::vec3 euler = glm::vec3(0.0f);
};

struct Scale {
    glm::vec3 scale = glm::vec3(1.0f);
};

// Define parent-child relationship components
struct Parent {
    Entity parent;
};

struct Children {
    std::vector<Entity> children;
};
#pragma endregion

/*
* Transform compoents utility
*/
namespace Transform {
    inline bool hasTransformComponents(ECSWorld& world, Entity entity) {
        return world.hasComponent<ModelMatrix>(entity) &&
            world.hasComponent<Position>(entity) &&
            world.hasComponent<Rotation>(entity) &&
            world.hasComponent<Scale>(entity);
    }

    // Helper function to add transformation components to an entity
    inline void addTransformComponents(ECSWorld& world, Entity entity,
        const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& rotationEuler = glm::vec3(0.0f),
        const glm::vec3& scale = glm::vec3(1.0f))
    {
        world.addComponent(entity, Position{position});
        world.addComponent(entity, EulerAngles{rotationEuler});
        world.addComponent(entity, Rotation{glm::quat(glm::radians(rotationEuler))});
        world.addComponent(entity, Scale{scale});
        world.addComponent(entity, ModelMatrix{});  // Ensure ModelMatrix is added
    }

    // Function to set parent while preserving world transform
    inline void setParent(ECSWorld& world, Entity child, Entity newParent) {
        // Ensure both entities have transform components
        if (!hasTransformComponents(world, child) || !hasTransformComponents(world, newParent)) {
            return;
        }

        // Get child's current world transform components
        glm::vec3 childWorldPos = world.getComponent<Position>(child).position;
        glm::quat childWorldRot = world.getComponent<Rotation>(child).quaternion;
        glm::vec3 childWorldScale = world.getComponent<Scale>(child).scale;

        glm::vec3 parentWorldPos = glm::vec3(0.0f);
        glm::quat parentWorldRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 parentWorldScale = glm::vec3(1.0f);

        parentWorldPos = world.getComponent<Position>(newParent).position;
        parentWorldRot = world.getComponent<Rotation>(newParent).quaternion;
        parentWorldScale = world.getComponent<Scale>(newParent).scale;

        // Compute inverse of parent's rotation and scale
        glm::quat parentWorldRotInv = glm::inverse(parentWorldRot);
        glm::vec3 invParentScale = glm::vec3(1.0f / parentWorldScale.x,
                                            1.0f / parentWorldScale.y,
                                            1.0f / parentWorldScale.z);

        // Calculate local transform
        glm::vec3 localPos = parentWorldRotInv * ((childWorldPos - parentWorldPos) * invParentScale);
        glm::quat localRot = parentWorldRotInv * childWorldRot;
        glm::vec3 localScale = childWorldScale / parentWorldScale;

        // Update child's local transformation components
        world.getComponent<Position>(child).position = localPos;
        world.getComponent<Rotation>(child).quaternion = glm::normalize(localRot);
        world.getComponent<Scale>(child).scale = localScale;

        // Update EulerAngles based on the new local rotation
        glm::vec3 euler = glm::degrees(glm::eulerAngles(glm::normalize(localRot)));
        world.getComponent<EulerAngles>(child).euler = euler;

        // Set parent/child relations

        // Set the new parent
        world.addComponent(child, Parent{newParent});

        // Add child to parent's hildren component
        if (!world.hasComponent<Children>(newParent)) {
            world.addComponent(newParent, Children{});
        }
        world.getComponent<Children>(newParent).children.push_back(child);

        // Mark the child and its descendants as dirty
        markChildrenDirty(world, child);
    }

    // System to mark children as dirty and update model matrices
    inline void updateTransformsSystem(ECSWorld& world) {
        // Mark dirty if parent is dirty
        auto parentEntities = world.batchedQuery<Parent>();
        for (Entity entity : parentEntities) {
            const Entity& parent = world.getComponent<Parent>(entity).parent;
            const auto& parentModelMatrix = world.getComponent<ModelMatrix>(parent);
            if (parentModelMatrix.dirty) {
                auto& modelMatrix = world.getComponent<ModelMatrix>(entity);
                modelMatrix.dirty = true;
            }
        }

        // Update all dirty model matrices
        auto dirtyEntities = world.batchedQuery<ModelMatrix>();
        updateModelMatrices(world, dirtyEntities);
    }
}

/*
* Setter and getter data
*/
namespace Transform {
    inline glm::vec3& getPosition(ECSWorld* world, Entity entity) {
        return world->getComponent<Position>(entity).position;
    }

    inline void setPosition(ECSWorld* world, Entity entity, const glm::vec3& pos) {
        world->getComponent<Position>(entity).position = pos;
        world->getComponent<ModelMatrix>(entity).dirty = true;
    }

    inline glm::vec3& getEuler(ECSWorld* world, Entity entity) {
        return world->getComponent<EulerAngles>(entity).euler;
    }

    inline void setEuler(ECSWorld* world, Entity entity, const glm::vec3& euler) {
        auto& eulerComponent = world->getComponent<EulerAngles>(entity);
        eulerComponent.euler = euler;

        // When euler is set, rotation must also update
        // Convert Euler angles to quaternion
        auto& rotationComponent = world->getComponent<Rotation>(entity);
        rotationComponent.quaternion = glm::quat(glm::radians(euler));

        world->getComponent<ModelMatrix>(entity).dirty = true;
    }

    inline glm::quat& getRotation(ECSWorld* world, Entity entity) {
        return world->getComponent<Rotation>(entity).quaternion;
    }

    inline void setRotation(ECSWorld* world, Entity entity, const glm::quat& rotation) {
        world->getComponent<Rotation>(entity).quaternion = rotation;
        // When rotation is set, euler must also update
        world->getComponent<EulerAngles>(entity).euler = glm::degrees(glm::eulerAngles(rotation));
        world->getComponent<ModelMatrix>(entity).dirty = true;
    }

    inline glm::vec3& getScale(ECSWorld* world, Entity entity) {
        return world->getComponent<Scale>(entity).scale;
    }

    inline void setScale(ECSWorld* world, Entity entity, const glm::vec3& scale) {
        world->getComponent<Scale>(entity).scale = scale;
        world->getComponent<ModelMatrix>(entity).dirty = true;
    }

    /*
    * Directional Vectors
    */
    inline glm::vec3 getForward(ECSWorld* world, Entity entity) {
        auto& rotation = world->getComponent<Rotation>(entity).quaternion;
        return rotation * glm::vec3(0, 0, 1);
    }

    inline glm::vec3 getUp(ECSWorld* world, Entity entity) {
        auto& rotation = world->getComponent<Rotation>(entity).quaternion;
        return rotation * glm::vec3(0, 1, 0);
    }

    inline glm::vec3 getRight(ECSWorld* world, Entity entity) {
        auto& rotation = world->getComponent<Rotation>(entity).quaternion;
        return rotation * glm::vec3(1, 0, 0);
    }
}

#pragma region Misc Functions
// System to update the ModelMatrix component if marked as dirty
inline void updateModelMatrices(ECSWorld& world, const std::vector<Entity>& entities) {
    for (Entity entity : entities) {
        auto& modelMatrix = world.getComponent<ModelMatrix>(entity);

        // Only update if the model matrix is dirty
        if (!modelMatrix.dirty) {
            continue;
        }

        // Get local transformations
        auto& position = world.getComponent<Position>(entity).position;
        auto& rotation = world.getComponent<Rotation>(entity).quaternion;
        auto& scale = world.getComponent<Scale>(entity).scale;

        // Apply local transformations to matrix
        glm::mat4 localMatrix = glm::mat4(1.0f);
        localMatrix = glm::translate(localMatrix, position);
        localMatrix *= glm::mat4_cast(rotation);
        localMatrix = glm::scale(localMatrix, scale);

        // If the entity has a parent, combine with the parent's model matrix
        if (world.hasComponent<Parent>(entity)) {
            Entity parent = world.getComponent<Parent>(entity).parent;
            const auto& parentModelMatrix = world.getComponent<ModelMatrix>(parent).matrix;
            localMatrix = parentModelMatrix * localMatrix;
        }

        // Update the model matrix
        modelMatrix.matrix = localMatrix;
        modelMatrix.dirty = false;
    }
}

// Recursively mark all children as dirty if their parent is dirty
inline void markChildrenDirty(ECSWorld& world, const Entity& parent) {
    if (!world.hasComponent<Children>(parent)) {
        return;
    }

    for (Entity child : world.getComponent<Children>(parent).children) {
        auto& modelMatrix = world.getComponent<ModelMatrix>(child);
        modelMatrix.dirty = true;
        // Recursively mark all grand children
        markChildrenDirty(world, child);
    }
}
#pragma endregion

#endif // TRANSFORM_H
