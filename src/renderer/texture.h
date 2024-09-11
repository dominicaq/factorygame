#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <glad/glad.h>

class Texture {
public:
    Texture(const std::string& filePath);
    ~Texture();

    // Texture usage
    void bind(unsigned int slot = 0) const;
    void unbind() const;

    // Properties
    int width, height, nrChannels;

private:
    unsigned int m_textureID;
};

#endif
