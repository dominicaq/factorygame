#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <glad/glad.h> // OpenGL headers

class Texture {
public:
    Texture(const std::string& filePath);
    ~Texture();

    // Binds the texture for use in shaders
    void bind(unsigned int slot = 0) const;

    // Unbind the texture
    void unbind() const;

    // Returns the texture ID
    unsigned int getID() const;

private:
    unsigned int m_textureID;  // OpenGL texture ID
    int m_width, m_height, m_nrChannels;  // Texture properties
    std::string m_filePath;  // Path to the image file
};

#endif
