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
    Texture* normalMap = nullptr;

    // PBR Textures
    Texture* roughnessMap = nullptr;
    Texture* metallicMap = nullptr;
    Texture* aoMap = nullptr;
    Texture* heightMap = nullptr;
    float heightScale = 1.0f;

    // RGB Material properties
    glm::vec4 albedoColor = glm::vec4(1.0f);

    // Scale for texture tiling
    glm::vec2 uvScale = glm::vec2(1.0f, 1.0f);

    float shininess = 32.0f;
    float time;

    // Flag(s)
    bool isDeferred = false;

    // Constructor
    Material(Shader* shader)
        : shader(shader),
          albedoColor(glm::vec4(1.0f)),
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

        // Set tile amount
        if (shaderOverride->hasUniform("u_uvScale")) {
            shaderOverride->setVec2("u_uvScale", uvScale);
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

        // Bind metallic map if it exists
        if (metallicMap) {
            if (shaderOverride->hasUniform("u_MetallicMap")) {
                shaderOverride->setInt("u_MetallicMap", 2);
                metallicMap->bind(2);
            }

            if (shaderOverride->hasUniform("u_HasMetallicMap")) {
                shaderOverride->setBool("u_HasMetallicMap", true);
            }
        } else {
            if (shaderOverride->hasUniform("u_HasMetallicMap")) {
                shaderOverride->setBool("u_HasMetallicMap", false);
            }
        }

        // Bind roughness map if it exists
        if (roughnessMap) {
            if (shaderOverride->hasUniform("u_RoughnessMap")) {
                shaderOverride->setInt("u_RoughnessMap", 3);
                roughnessMap->bind(3);
            }

            if (shaderOverride->hasUniform("u_HasRoughnessMap")) {
                shaderOverride->setBool("u_HasRoughnessMap", true);
            }
        } else {
            if (shaderOverride->hasUniform("u_HasRoughnessMap")) {
                shaderOverride->setBool("u_HasRoughnessMap", false);
            }
        }

        // Bind AO map if it exists
        if (aoMap) {
            if (shaderOverride->hasUniform("u_AOMap")) {
                shaderOverride->setInt("u_AOMap", 4);
                aoMap->bind(4);
            }

            if (shaderOverride->hasUniform("u_HasAOMap")) {
                shaderOverride->setBool("u_HasAOMap", true);
            }
        } else {
            if (shaderOverride->hasUniform("u_HasAOMap")) {
                shaderOverride->setBool("u_HasAOMap", false);
            }
        }
    }
};

#endif
