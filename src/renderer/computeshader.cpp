#include "computeShader.h"
#include "../resources/resourceLoader.h"

#include <iostream>
#include <sstream>

ComputeShader::ComputeShader() : m_ID(0) {}

ComputeShader::ComputeShader(const std::string& computePath) {
    load(computePath);
}

ComputeShader::~ComputeShader() {
    if (glIsProgram(m_ID)) {
        glDeleteProgram(m_ID);
    }
}

/*
* Shader management
*/
void ComputeShader::use() const {
    if (m_ID == 0) {
        std::cerr << "[Error] ComputeShader::use: Shader program is not initialized\n";
        return;
    }
    glUseProgram(m_ID);
}

/*
* Uniform data
*/
GLint ComputeShader::getUniformLocation(const std::string& name) const {
    auto it = m_UniformLocationCache.find(name);
    if (it != m_UniformLocationCache.end()) {
        return it->second;
    }

    GLint location = glGetUniformLocation(m_ID, name.c_str());
    m_UniformLocationCache[name] = location;
    return location;
}

bool ComputeShader::hasUniform(const std::string& name) const {
    GLint location = getUniformLocation(name);
    return location != -1;
}

void ComputeShader::setBool(const std::string& name, bool value) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, (int)value);
    } else {
        std::cerr << "[Error] ComputeShader::setBool: Uniform not found: " << name << "\n";
    }
}

void ComputeShader::setInt(const std::string& name, int value) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value);
    } else {
        std::cerr << "[Error] ComputeShader::setInt: Uniform not found: " << name << "\n";
    }
}

void ComputeShader::setFloat(const std::string& name, float value) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1f(location, value);
    } else {
        std::cerr << "[Error] ComputeShader::setFloat: Uniform not found: " << name << "\n";
    }
}

void ComputeShader::setVec2(const std::string& name, const glm::vec2& value) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform2fv(location, 1, glm::value_ptr(value));
    } else {
        std::cerr << "[Error] ComputeShader::setVec2: Uniform not found: " << name << "\n";
    }
}

void ComputeShader::setVec3(const std::string& name, const glm::vec3& value) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform3fv(location, 1, glm::value_ptr(value));
    } else {
        std::cerr << "[Error] ComputeShader::setVec3: Uniform not found: " << name << "\n";
    }
}

void ComputeShader::setVec4(const std::string& name, const glm::vec4& value) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform4fv(location, 1, glm::value_ptr(value));
    } else {
        std::cerr << "[Error] ComputeShader::setVec4: Uniform not found: " << name << "\n";
    }
}

/*
* Shader creation
*/
bool ComputeShader::load(const std::string& computePath) {
    m_ID = 0;
    m_UniformLocationCache.clear();

    // Load shader from file
    std::string computeCode = ResourceLoader::readFile(computePath);
    if (computeCode.empty()) {
        std::cerr << "[Error] ComputeShader::load: Failed to read compute shader file: " << computePath << "\n";
        return false;
    }

    // Compile shader
    int compileStatus = 0;
    unsigned int computeShader = compileShader(GL_COMPUTE_SHADER, computeCode.c_str(), &compileStatus);
    if (compileStatus != 0) {
        std::cerr << "[Error] ComputeShader::load: Compute shader compilation failed\n";
        return false;
    }

    // Link program
    int linkStatus = 0;
    linkProgram(computeShader, &linkStatus);
    if (linkStatus != 0) {
        std::cerr << "[Error] ComputeShader::load: Program linking failed\n";
        glDeleteShader(computeShader);
        return false;
    }

    glDeleteShader(computeShader);
    return true;
}

unsigned int ComputeShader::compileShader(unsigned int type, const char* source, int* status) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int compileStatus;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "[Error] ComputeShader::compileShader: Compilation failed\n" << infoLog << "\n";
        *status = -1;
    } else {
        *status = 0;
    }

    return shader;
}

void ComputeShader::linkProgram(unsigned int computeShader, int* status) {
    m_ID = glCreateProgram();
    glAttachShader(m_ID, computeShader);
    glLinkProgram(m_ID);

    int linkStatus;
    char infoLog[512];
    glGetProgramiv(m_ID, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus) {
        glGetProgramInfoLog(m_ID, 512, NULL, infoLog);
        std::cerr << "[Error] ComputeShader::linkProgram: Linking failed\n" << infoLog << "\n";
        *status = -1;
        m_ID = 0;
    } else {
        *status = 0;
    }
}

void ComputeShader::dispatchCompute(unsigned int x, unsigned int y, unsigned int z) const {
    if (m_ID == 0) {
        std::cerr << "[Error] ComputeShader::dispatchCompute: Shader program is not initialized\n";
        return;
    }
    glDispatchCompute(x, y, z);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // Ensure all reads/writes are completed
}
