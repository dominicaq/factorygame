#pragma once

#include <string>
#include <unordered_map>
#include <set>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    // Constructor and destructor
    Shader();
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    // Program operations
    bool load(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;

    // Query functions
    bool hasUniform(const std::string& name) const;

    // Uniform setters
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    // Shader program ID
    unsigned int m_ID;

    // Cache for uniform locations
    mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;

    // Helper methods for compilation and linking
    unsigned int compileShader(unsigned int type, const char* source, int* status);
    void linkProgram(unsigned int vertexShader, unsigned int fragmentShader, int* status);

    // Process includes in shader source code
    std::string processIncludes(const std::string& source, const std::string& shaderDirectory);
    std::string processIncludes(const std::string& source, const std::string& shaderDirectory,
                               std::set<std::string>& includedFiles);

    // Uniform location getter
    GLint getUniformLocation(const std::string& name) const;
};
