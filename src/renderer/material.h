#ifndef MATERIAL_H
#define MATERIAL_H

#include "shader.h"
#include "texture.h"

#include <glm.hpp>
#include <memory>

struct Material {
    Shader* shader = nullptr;

    // Textures
    Texture* albedoMap = nullptr;
    Texture* specularMap = nullptr;
    Texture* normalMap = nullptr;

    // Colors
    glm::vec3 albedoColor = glm::vec3(1.0f);
    glm::vec3 specularColor = glm::vec3(1.0f);

    // Scalars
    float shininess = 32.0f;

    Material(Shader* shader) : shader(shader) {}

    void bind() const {
        shader->use();  // Ensure the shader is active

        // Bind the albedo color uniform
        shader->setVec3("u_AlbedoColor", albedoColor);

        // Add material maps for sampling if they exist
        if (albedoMap) {
            shader->setInt("u_AlbedoMap", 0);
            albedoMap->bind(0);
        }

        if (specularMap) {
            shader->setInt("u_SpecularMap", 1);
            specularMap->bind(1);
        }

        if (normalMap) {
            shader->setInt("u_NormalMap", 2);
            normalMap->bind(2);
        }
    }
};

#endif // MATERIAL_H
