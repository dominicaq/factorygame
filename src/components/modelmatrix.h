#ifndef MODELMATRIX_H
#define MODELMATRIX_H

#include "ecs.h"
#include "transform.h"

#include "glm.hpp"

#include <vector>

// ModelMatrix component
struct ModelMatrix {
    glm::mat4 matrix = glm::mat4(1.0f);
};

inline void updateModelMatrices(ECSWorld& ecs, const std::vector<Entity>& modelMatrixQuery) {
    for (Entity entity : modelMatrixQuery) {
        Transform& transform = ecs.getComponent<Transform>(entity);
        ModelMatrix& modelMatrix = ecs.getComponent<ModelMatrix>(entity);

        // If the transform was updated
        if (transform.isDirty()) {
            // Recalculate the model matrix and update the ModelMatrix component
            modelMatrix.matrix = transform.calculateModelMatrix();

            // Clear the dirty flag after the model matrix is updated
            transform.clearDirtyFlag();
        }
    }
}

#endif
