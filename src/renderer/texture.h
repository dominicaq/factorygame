#pragma once

#include <glad/glad.h>
#include <string>

class Texture {
public:
    Texture(const std::string& filePath);
    ~Texture();

    // Bindless texture methods
    GLuint64 getHandle() const { return m_handle; }
    GLuint getTextureID() const { return m_textureID; }
    bool isValid() const { return m_handle != 0; }
    const std::string& getFilePath() const { return m_filePath; }

    void makeResident();
    void makeNonResident();

private:
    GLuint m_textureID;
    GLuint64 m_handle;
    bool m_isResident;
    std::string m_filePath;

    void createTexture(const std::string& filePath);
};
