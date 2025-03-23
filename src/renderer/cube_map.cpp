#include "../engine.h"
#include "cube_map.h"

#include <glad/glad.h>
#include <iostream>

namespace CubeMap {
    unsigned int createFromImages(const std::vector<std::string>& faces) {
        int width, height, nrChannels;
        std::vector<unsigned char*> cubemapData = ResourceLoader::loadCubemapImages(faces, &width, &height, &nrChannels);
        if (cubemapData.empty()) {
            std::cerr << "[Error] Failed to load cubemap images\n";
            return 0;
        }

        unsigned int cubemapTexture;
        glGenTextures(1, &cubemapTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        for (unsigned int i = 0; i < cubemapData.size(); i++) {
            if (!cubemapData[i]) {
                std::cerr << "[Error] Failed to load texture for face: " << i << "\n";
                continue;
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, cubemapData[i]);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        // Free image data after uploading
        ResourceLoader::freeCubemapImages(cubemapData);

        return cubemapTexture;
    }
}
