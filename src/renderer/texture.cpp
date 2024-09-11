#include "texture.h"
#include "../resources/resource_loader.h"

#include <iostream>

Texture::Texture(const std::string& filePath) : m_textureID(0), width(0), height(0), nrChannels(0) {
    // Use ResourceLoader to load image data
    unsigned char* data = ResourceLoader::loadImage(filePath, &width, &height, &nrChannels);

    if (data) {
        GLenum format;
        if (nrChannels == 1)
            // Handle in fragment shader
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else
            format = GL_RGB;

        // Generate an OpenGL texture
        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // Repeat wrapping mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);    // Repeat wrapping mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Linear filtering with mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear filtering

        // Upload the texture data to the GPU
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        // Unbind texture to avoid accidentally modifying it
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        std::cerr << "ERROR::TEXTURE::Failed to load texture: " << filePath << "\n";
    }

    // Free image data after uploading to GPU
    ResourceLoader::freeImage(data);
}

Texture::~Texture() {
    glDeleteTextures(1, &m_textureID);
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
