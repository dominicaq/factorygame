// Material.h
#pragma once
#include <glm/glm.hpp>
#include "shader.h"
#include "texture.h"

struct Material {
    std::shared_ptr<Shader> shader;
    glm::vec3 diffuseColor;
    glm::vec3 specularColor;
    float shininess;
    Texture* diffuseTexture;
    Texture* specularTexture;

    Material(std::shared_ptr<Shader> shader)
        : shader(shader), diffuseColor(1.0f), specularColor(1.0f), shininess(32.0f) {}

    void bind() const {
        shader->use();
        shader->setVec3("u_DiffuseColor", diffuseColor);
        shader->setVec3("u_SpecularColor", specularColor);
        shader->setFloat("u_Shininess", shininess);

        // Bind the diffuse texture if available
        if (diffuseTexture) {
            shader->setInt("u_DiffuseTexture", 0);  // Texture unit 0
            diffuseTexture->bind(0);
        }

        // Bind the specular texture if available
        if (specularTexture) {
            shader->setInt("u_SpecularTexture", 1);  // Texture unit 1
            specularTexture->bind(1);
        }
    }
};
