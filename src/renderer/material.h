#ifndef MATERIAL_H
#define MATERIAL_H

#include "shader.h"
#include "texture.h"

#include <glm.hpp>
#include <memory>

struct Material {
    Shader* shader = nullptr;

    // Diffuse
    Texture* albedoTexture = nullptr;
    glm::vec3 albedoColor = glm::vec3(1.0f);

    // Specular
    float shininess = 32.0f;
    Texture* specularTexture = nullptr;
    glm::vec3 specularColor = glm::vec3(1.0f);

    Material(Shader* shader) : shader(shader) {}

    void bind() const {
        shader->use();  // Ensure the shader is active

        // Bind the albedo color uniform
        shader->setVec3("u_AlbedoColor", albedoColor);  // Ensure the uniform name matches

        // Bind the albedo texture if it exists
        if (albedoTexture) {
            shader->setInt("u_AlbedoTexture", 0);
            albedoTexture->bind(0);
        }
    }

};

#endif // MATERIAL_H
