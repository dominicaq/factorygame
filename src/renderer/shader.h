#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>

class Shader {
public:
    Shader();
    Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");
    ~Shader();

    bool load(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");
    void use() const;

    /*
    * Set uniform data
    */
    bool hasUniform(const std::string& name) const;
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;

private:
    unsigned int m_ID;
    mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;

    unsigned int compileShader(unsigned int type, const char* source, int* status);
    void linkProgram(unsigned int vertexShader, unsigned int fragmentShader, unsigned int geometryShader, int* status);
    GLint getUniformLocation(const std::string& name) const;
};

#endif
