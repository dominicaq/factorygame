#include "texture.h"
#include "../resources/resourceLoader.h"
#include <iostream>

Texture::Texture(const std::string& filePath)
    : m_textureID(0), m_handle(0), m_isResident(false), m_filePath(filePath) {
    createTexture(filePath);
}

Texture::~Texture() {
    if (m_isResident && m_handle != 0) {
        glMakeTextureHandleNonResidentARB(m_handle);
    }
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }
}

void Texture::createTexture(const std::string& filePath) {
    int width = 0, height = 0, nrChannels = 0;
    // Use ResourceLoader to load image data
    unsigned char* data = ResourceLoader::loadImage(filePath, &width, &height, &nrChannels);

    if (data) {
        // Determine the correct format based on the number of channels
        GLenum format;
        GLenum internalFormat;
        bool isGrayscale = false;

        if (nrChannels == 1) {
            format = GL_RED;
            internalFormat = GL_R8;
            isGrayscale = true;
        } else if (nrChannels == 3) {
            format = GL_RGB;
            internalFormat = GL_RGB8;
        } else if (nrChannels == 4) {
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
        } else {
            std::cerr << "[Error] Texture::createTexture: Unsupported number of channels: " << nrChannels << "\n";
            format = GL_RGB;
            internalFormat = GL_RGB8;
        }

        // Generate an OpenGL texture
        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        // Set texture parameters (repeat wrapping)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload the texture data to the GPU
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        // If the texture is grayscale, set texture swizzle parameters to replicate the red channel to RGB
        if (isGrayscale) {
            GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }

        // Generate mipmaps for the texture
        glGenerateMipmap(GL_TEXTURE_2D);

        // Apply anisotropic filtering if available
        if (glTexParameterf) { // Check if function is loaded
            GLfloat maxAniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
            if (maxAniso > 0.0f) {
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
            }
        }

        // Unbind the texture
        glBindTexture(GL_TEXTURE_2D, 0);

        // Create bindless handle
        if (glGetTextureHandleARB) { // Check if bindless texture functions are loaded
            m_handle = glGetTextureHandleARB(m_textureID);
            if (m_handle == 0) {
                std::cerr << "[Error] Texture::createTexture: Failed to create bindless handle for: " << filePath << "\n";
            }
        } else {
            std::cerr << "[Warning] Texture::createTexture: Bindless textures not supported\n";
        }

        // Error checking
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "[Error] Texture::createTexture: OpenGL error during texture creation: " << error << "\n";
        }
    } else {
        std::cerr << "[Error] Texture::createTexture: Failed to load texture: " << filePath << "\n";
    }

    // Free image data after uploading to the GPU
    ResourceLoader::freeImage(data);
}

void Texture::makeResident() {
    if (m_handle != 0 && !m_isResident && glMakeTextureHandleResidentARB) {
        glMakeTextureHandleResidentARB(m_handle);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "[Error] Texture::makeResident: OpenGL error: " << error << std::endl;
            m_isResident = false;
        } else {
            m_isResident = true;
        }
    }
}

void Texture::makeNonResident() {
    if (m_handle != 0 && m_isResident && glMakeTextureHandleNonResidentARB) {
        glMakeTextureHandleNonResidentARB(m_handle);
        m_isResident = false;

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "[Error] Texture::makeNonResident: OpenGL error: " << error << "\n";
        }
    }
}
