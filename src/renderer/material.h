#ifndef MATERIAL_H
#define MATERIAL_H

#include "shader.h"
#include "texture.h"

#include <glm/glm.hpp>

struct Material {
    Shader* shader = nullptr;

    Texture* albedoMap = nullptr;
    Texture* normalMap = nullptr;

    // Combined metallic-roughness map (e.g. RGB channels: R=AO, G=Roughness, B=Metallic)
    Texture* metallicRoughnessMap = nullptr;
    float occlusionStrength = 1.0f;

    Texture* emissiveMap = nullptr;
    glm::vec3 emissiveColor = glm::vec3(1.0f);

    Texture* heightMap = nullptr;
    float heightScale = 1.0f;

    glm::vec4 albedoColor = glm::vec4(1.0f);

    glm::vec2 uvScale = glm::vec2(1.0f, 1.0f);

    float shininess = 32.0f;
    float time;

    bool isDeferred = true;

    Material(Shader* shader)
        : shader(shader),
          albedoColor(glm::vec4(1.0f)),
          shininess(32.0f),
          isDeferred(true) {}

    void bind(Shader* shaderOverride = nullptr) const {
        Shader* shaderToUse = shaderOverride ? shaderOverride : shader;

        struct TextureBinding {
            const char* uniformName;
            const char* hasUniformName;
            Texture* texture;
            int textureUnit;
            float* strength;
        };

        TextureBinding bindings[] = {
            {"u_AlbedoMap", nullptr, albedoMap, 0, nullptr},
            {"u_NormalMap", "u_HasNormalMap", normalMap, 1, nullptr},
            {"u_MetallicRoughnessMap", "u_HasMetallicRoughnessMap", metallicRoughnessMap, 2, nullptr},
            {"u_HeightMap", "u_HasHeightMap", heightMap, 3, nullptr},
            {"u_EmissiveMap", "u_HasEmissiveMap", emissiveMap, 4, nullptr}
        };

        for (const auto& binding : bindings) {
            if (binding.texture) {
                if (shaderToUse->hasUniform(binding.uniformName)) {
                    shaderToUse->setInt(binding.uniformName, binding.textureUnit);
                    binding.texture->bind(binding.textureUnit);
                }
                if (binding.hasUniformName && shaderToUse->hasUniform(binding.hasUniformName)) {
                    shaderToUse->setBool(binding.hasUniformName, true);
                }
            } else {
                if (binding.hasUniformName && shaderToUse->hasUniform(binding.hasUniformName)) {
                    shaderToUse->setBool(binding.hasUniformName, false);
                }
            }
        }

        shaderToUse->setVec4("u_AlbedoColor", albedoColor);

        if (shaderToUse->hasUniform("u_Time")) {
            shaderToUse->setFloat("u_Time", time);
        }
        if (shaderToUse->hasUniform("u_HeightScale")) {
            shaderToUse->setFloat("u_HeightScale", heightScale);
        }
        if (shaderToUse->hasUniform("u_uvScale")) {
            shaderToUse->setVec2("u_uvScale", uvScale);
        }
        if (shaderToUse->hasUniform("u_EmissiveColor")) {
            shaderToUse->setVec3("u_EmissiveColor", emissiveColor);
        }
    }
};

#endif
