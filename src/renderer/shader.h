#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    void use() const;
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;

private:
    unsigned int m_ID;

    unsigned int compileShader(unsigned int type, const char* source, int *status);
    void linkProgram(unsigned int vertexShader, unsigned int fragmentShader, int *status);
    int loadShaderSource(const std::string& path, std::string& sourceCode);
};

#endif
