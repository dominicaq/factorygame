#ifndef MATERIAL_H
#define MATERIAL_H

#include "shader.h"
#include "texture.h"

#include <glm/glm.hpp>
#include <memory>

struct Material {
    // Shader ptr
    Shader* shader = nullptr;

    // Texture ptr(s)
    Texture* albedoMap = nullptr;
    Texture* specularMap = nullptr;
    Texture* normalMap = nullptr;

    // RGB Material properties
    glm::vec3 albedoColor = glm::vec3(1.0f);
    glm::vec3 specularColor = glm::vec3(1.0f);

    float shininess = 32.0f;

    // Flag(s)
    bool isDeferred = false;

    // Constructor
    Material(Shader* shader)
        : shader(shader),
          albedoColor(glm::vec3(1.0f)),
          specularColor(glm::vec3(1.0f)),
          shininess(32.0f),
          isDeferred(false)
    {}

    // Function to bind the material data to the shader
    void bind(Shader* shaderOverride = nullptr) const {
        if (shaderOverride == nullptr) {
            // Use default shader if no shader is passed
            shaderOverride = shader;
        }

        // Bind albedo map if it exists, otherwise set default color
        if (albedoMap) {
            shaderOverride->setInt("u_AlbedoMap", 0);
            albedoMap->bind(0);
        }
        shaderOverride->setVec3("u_AlbedoColor", albedoColor);

        // Bind normal map if it exists, otherwise set flag for shader
        if (normalMap) {
            shaderOverride->setInt("u_NormalMap", 1);
            shaderOverride->setBool("u_HasNormalMap", true);
            normalMap->bind(1);
        } else {
            shaderOverride->setBool("u_HasNormalMap", false);
        }

        // Bind specular map if it exists
        // if (specularMap) {
        //     shaderOverride->setInt("u_SpecularMap", 1);
        //     specularMap->bind(1);
        // }

        // Set material properties like specular color and shininess
        // shaderOverride->setVec3("u_SpecularColor", specularColor);
        // shaderOverride->setFloat("u_Shininess", shininess);

        // TODO: Unbind texture(s) if they exist
    }


};

#endif
