#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "ecs/entity.h"
#include "ecs/ecsworld.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include <vector>

struct ModelMatrix {
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

// Function to update the quaternion from Euler angles (whenever Euler angles are changed)
inline void updateQuaternionFromEuler(ECSWorld& world, Entity entity) {
    auto& eulerAngles = world.getComponent<EulerAngles>(entity).euler;
    auto& rotation = world.getComponent<Rotation>(entity).quaternion;

    // Update quaternion from Euler angles
    rotation = glm::quat(glm::radians(eulerAngles));
}

// Function to mark child entities as dirty if their parent is dirty
inline void markChildrenDirty(ECSWorld& world, const std::vector<Entity>& entities) {
    for (Entity entity : entities) {
        auto& parentComponent = world.getComponent<Parent>(entity);
        if (parentComponent.parent.isValid()) {
            const auto& parentModelMatrix = world.getComponent<ModelMatrix>(parentComponent.parent);

            if (parentModelMatrix.dirty) {
                auto& modelMatrix = world.getComponent<ModelMatrix>(entity);
                modelMatrix.dirty = true;
            }
        }
    }
}

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
        localMatrix = glm::translate(glm::mat4(1.0f), position);
        localMatrix *= glm::mat4_cast(rotation);
        localMatrix = glm::scale(localMatrix, scale);

        // If the entity has a parent, combine with the parent's model matrix
        if (world.hasComponent<Parent>(entity)) {
            Entity parent = world.getComponent<Parent>(entity).parent;
            if (parent.isValid()) {
                auto& parentModelMatrix = world.getComponent<ModelMatrix>(parent).matrix;
                localMatrix = parentModelMatrix * localMatrix;
            }
        }

        // Update the model matrix
        modelMatrix.matrix = localMatrix;
        modelMatrix.dirty = false;
    }
}

// Define the HierarchySystem for updating world space transformations
namespace Transform {
    // Helper function to add transformation components to an entity
    inline void addTransform(ECSWorld& world, Entity entity,
                             const glm::vec3& position = glm::vec3(0.0f),
                             const glm::vec3& rotationEuler = glm::vec3(0.0f),
                             const glm::vec3& scale = glm::vec3(1.0f)) {
        world.addComponent(entity, Position{position});
        world.addComponent(entity, EulerAngles{rotationEuler});
        world.addComponent(entity, Rotation{glm::quat(glm::radians(rotationEuler))});
        world.addComponent(entity, Scale{scale});
        world.addComponent(entity, ModelMatrix{});  // Ensure ModelMatrix is added
    }

    // Function to set parent while preserving world transform
    inline void setParent(ECSWorld& world, Entity child, Entity newParent) {
        // Ensure the child has necessary components
        if (!world.hasComponent<ModelMatrix>(child) ||
            !world.hasComponent<Position>(child) ||
            !world.hasComponent<Rotation>(child) ||
            !world.hasComponent<Scale>(child)) {
            // Handle error: child must have transformation components
            return;
        }

        // If setting a new parent, ensure it has necessary components
        if (newParent.isValid()) {
            if (!world.hasComponent<ModelMatrix>(newParent) ||
                !world.hasComponent<Position>(newParent) ||
                !world.hasComponent<Rotation>(newParent) ||
                !world.hasComponent<Scale>(newParent)) {
                // Handle error: new parent must have transformation components
                return;
            }
        }

        // Get child's current world transform components
        glm::vec3 childWorldPos = world.getComponent<Position>(child).position;
        glm::quat childWorldRot = world.getComponent<Rotation>(child).quaternion;
        glm::vec3 childWorldScale = world.getComponent<Scale>(child).scale;

        glm::vec3 parentWorldPos = glm::vec3(0.0f);
        glm::quat parentWorldRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 parentWorldScale = glm::vec3(1.0f);

        if (newParent.isValid()) {
            parentWorldPos = world.getComponent<Position>(newParent).position;
            parentWorldRot = world.getComponent<Rotation>(newParent).quaternion;
            parentWorldScale = world.getComponent<Scale>(newParent).scale;
        }

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

        // Set the new parent
        if (newParent.isValid()) {
            world.addComponent(child, Parent{newParent});
            // Optionally, add child to parent's Children component
            if (!world.hasComponent<Children>(newParent)) {
                world.addComponent(newParent, Children{});
            }
            world.getComponent<Children>(newParent).children.push_back(child);
        } else {
            // Remove Parent component if newParent is invalid (no parent)
            if (world.hasComponent<Parent>(child)) {
                world.removeComponent<Parent>(child);
            }
        }

        // Mark the child and its descendants as dirty
        auto entitiesToMark = std::vector<Entity>{child};
        markChildrenDirty(world, entitiesToMark);
    }

    // Function to mark children as dirty and update model matrices
    inline void updateTransforms(ECSWorld& world) {
        // First, mark children as dirty if their parent is dirty
        auto childEntities = world.batchedQuery<Parent, ModelMatrix>();
        markChildrenDirty(world, childEntities);

        // Then, update the model matrices
        auto entities = world.batchedQuery<ModelMatrix>();
        updateModelMatrices(world, entities);
    }

    /*
    * Setter and getter data
    */
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
        auto& rotationComponent = world->getComponent<Rotation>(entity);
        eulerComponent.euler = euler;

        // Convert Euler angles to quaternion
        rotationComponent.quaternion = glm::quat(glm::radians(euler));

        // Mark the ModelMatrix as dirty to ensure it gets updated
        world->getComponent<ModelMatrix>(entity).dirty = true;
    }

    inline glm::quat& getRotation(ECSWorld* world, Entity entity) {
        return world->getComponent<Rotation>(entity).quaternion;
    }

    inline void setRotation(ECSWorld* world, Entity entity, const glm::quat& rotation) {
        world->getComponent<Rotation>(entity).quaternion = rotation;

        // Convert the quaternion back to Euler angles for consistency
        world->getComponent<EulerAngles>(entity).euler = glm::degrees(glm::eulerAngles(rotation));

        // Mark the ModelMatrix as dirty to ensure it gets updated
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

#endif // TRANSFORM_H
