#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class ComputeShader {
public:
    ComputeShader();
    ComputeShader(const std::string& computePath);
    ~ComputeShader();

    bool load(const std::string& computePath);
    void use() const;

    /*
    * Set uniform data
    */
    bool hasUniform(const std::string& name) const;
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;

    void dispatchCompute(unsigned int x, unsigned int y, unsigned int z) const;

private:
    unsigned int m_ID;
    mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;

    unsigned int compileShader(unsigned int type, const char* source, int* status);
    void linkProgram(unsigned int computeShader, int* status);
    GLint getUniformLocation(const std::string& name) const;
};

#endif
