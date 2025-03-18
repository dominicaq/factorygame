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
    glm::vec4 albedoColor = glm::vec4(1.0f);
    glm::vec3 specularColor = glm::vec3(1.0f);

    float shininess = 32.0f;
    float time;

    // Flag(s)
    bool isDeferred = false;

    // Constructor
    Material(Shader* shader)
        : shader(shader),
          albedoColor(glm::vec4(1.0f)),
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

        // Requirement: Every material shader needs a color
        shaderOverride->setVec4("u_AlbedoColor", albedoColor);

        if (shaderOverride->hasUniform("u_Time")) {
            shaderOverride->setFloat("u_Time", time);
        }

        // Bind albedo map if it exists, otherwise set default color
        if (albedoMap) {
            if (shaderOverride->hasUniform("u_AlbedoMap")) {
                shaderOverride->setInt("u_AlbedoMap", 0);
                albedoMap->bind(0);
            }
        }

        // Bind normal map if it exists, otherwise set flag for shader
        if (normalMap) {
            if (shaderOverride->hasUniform("u_NormalMap")) {
                shaderOverride->setInt("u_NormalMap", 1);
                normalMap->bind(1);
            }

            if (shaderOverride->hasUniform("u_HasNormalMap")) {
                shaderOverride->setBool("u_HasNormalMap", true);
            }
        } else {
            if (shaderOverride->hasUniform("u_HasNormalMap")) {
                shaderOverride->setBool("u_HasNormalMap", false);
            }
        }

        // Bind specular map if it exists
        if (specularMap) {
            if (shaderOverride->hasUniform("u_SpecularMap")) {
                shaderOverride->setInt("u_SpecularMap", 2);
                specularMap->bind(2);
            }
        }
    }
};

#endif
