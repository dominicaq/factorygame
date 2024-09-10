#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "texture.h"
#include <iostream>

Texture::Texture(const std::string& filePath) : m_filePath(filePath), m_textureID(0), m_width(0), m_height(0), m_nrChannels(0) {
    // Load image data
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(m_filePath.c_str(), &m_width, &m_height, &m_nrChannels, 0);

    if (data) {
        GLenum format;
        if (m_nrChannels == 1)
            // TODO: Handle grayscale properly
            format = GL_RED;
        else if (m_nrChannels == 3)
            format = GL_RGB;
        else if (m_nrChannels == 4)
            format = GL_RGBA;
        else
            format = GL_RGB; // Fallback in case of unusual channels

        // Generate an OpenGL texture
        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // Repeat wrapping mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);    // Repeat wrapping mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Linear filtering with mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear filtering

        // Upload the texture data to the GPU
        glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);

        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        // Unbind texture to avoid accidentally modifying it
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        std::cerr << "Failed to load texture: " << m_filePath << std::endl;
    }

    // Free image data after uploading to GPU
    stbi_image_free(data);
}

Texture::~Texture() {
    glDeleteTextures(1, &m_textureID);
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot); // Activate texture unit slot
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
}

unsigned int Texture::getID() const {
    return m_textureID;
}
