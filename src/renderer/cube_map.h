#ifndef CUBE_MAP_H
#define CUBE_MAP_H

#include <string>
#include <vector>
#include <glad/glad.h>

class CubeMap {
public:
    CubeMap(const std::vector<std::string>& faces);
    ~CubeMap();

    // Bind and unbind cubemap texture
    void bind(unsigned int slot = 0) const;
    void unbind() const;

private:
    unsigned int m_textureID;
};

#endif // CUBE_MAP_H
