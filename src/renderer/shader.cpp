#include "shader.h"
#include "../resources/resource_loader.h"

#include <iostream>
#include <sstream>

Shader::Shader() : m_ID(0) {}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
    load(vertexPath, fragmentPath);
}

Shader::~Shader() {
    if (glIsProgram(m_ID)) {
        glDeleteProgram(m_ID);
    }
}

/*
* Shader management
*/
void Shader::use() const {
    if (m_ID == 0) {
        std::cerr << "[Error] Shader::use: Shader program is not initialized\n";
        return;
    }
    glUseProgram(m_ID);
}

/*
* Uniform data
*/
GLint Shader::getUniformLocation(const std::string& name) const {
    // Check if location is already cached
    auto it = m_UniformLocationCache.find(name);
    if (it != m_UniformLocationCache.end()) {
        return it->second;
    }

    // If not in cache, query it and store for future use
    GLint location = glGetUniformLocation(m_ID, name.c_str());
    m_UniformLocationCache[name] = location;
    return location;
}

bool Shader::hasUniform(const std::string& name) const {
    GLint location = getUniformLocation(name);
    // If location is -1, the uniform doesn't exist or isn't active
    return location != -1;
}

void Shader::setBool(const std::string& name, bool value) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, (int)value);
    } else {
        std::cerr << "[Error] Shader::setBool: Uniform not found: " << name << "\n";
    }
}

void Shader::setInt(const std::string& name, int value) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value);
    } else {
        std::cerr << "[Error] Shader::setInt: Uniform not found: " << name << "\n";
    }
}

void Shader::setFloat(const std::string& name, float value) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1f(location, value);
    } else {
        std::cerr << "[Error] Shader::setFloat: Uniform not found: " << name << "\n";
    }
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    } else {
        std::cerr << "[Error] Shader::setMat4: Uniform not found: " << name << "\n";
    }
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform3fv(location, 1, glm::value_ptr(value));
    } else {
        std::cerr << "[Error] Shader::setVec3: Uniform not found: " << name << "\n";
    }
}

/*
* Shader creation
*/
bool Shader::load(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
    m_ID = 0;
    // Clear the uniform location cache when loading a new shader
    m_UniformLocationCache.clear();

    // Load shaders from files
    std::string vertexCode = ResourceLoader::readFile(vertexPath);
    if (vertexCode.empty()) {
        std::cerr << "[Error] Shader::load: Failed to read vertex shader file: " << vertexPath << "\n";
        return false;
    }

    std::string fragmentCode = ResourceLoader::readFile(fragmentPath);
    if (fragmentCode.empty()) {
        std::cerr << "[Error] Shader::load: Failed to read fragment shader file: " << fragmentPath << "\n";
        return false;
    }

    std::string geometryCode;
    if (!geometryPath.empty()) {
        geometryCode = ResourceLoader::readFile(geometryPath);
        if (geometryCode.empty()) {
            std::cerr << "[Error] Shader::load: Failed to read geometry shader file: " << geometryPath << "\n";
            return false;
        }
    }

    // Compile shaders
    int compileStatus = 0;
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode.c_str(), &compileStatus);
    if (compileStatus != 0) {
        std::cerr << "[Error] Shader::load: Vertex shader compilation failed\n";
        return false;
    }

    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str(), &compileStatus);
    if (compileStatus != 0) {
        std::cerr << "[Error] Shader::load: Fragment shader compilation failed\n";
        glDeleteShader(vertexShader);
        return false;
    }

    unsigned int geometryShader = 0;
    if (!geometryCode.empty()) {
        geometryShader = compileShader(GL_GEOMETRY_SHADER, geometryCode.c_str(), &compileStatus);
        if (compileStatus != 0) {
            std::cerr << "[Error] Shader::load: Geometry shader compilation failed\n";
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return false;
        }
    }

    // Link shaders into a program
    int linkStatus = 0;
    linkProgram(vertexShader, fragmentShader, geometryShader, &linkStatus);
    if (linkStatus != 0) {
        std::cerr << "[Error] Shader::load: Program linking failed\n";
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        if (geometryShader) {
            glDeleteShader(geometryShader);
        }
        return false;
    }

    // Clean up shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryShader) {
        glDeleteShader(geometryShader);
    }

    return true;
}

unsigned int Shader::compileShader(unsigned int type, const char* source, int* status) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int compileStatus;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "[Error] Shader::compileShader: Compilation failed\n" << infoLog << "\n";
        *status = -1;
    } else {
        *status = 0;
    }

    return shader;
}

void Shader::linkProgram(unsigned int vertexShader, unsigned int fragmentShader, unsigned int geometryShader, int* status) {
    m_ID = glCreateProgram();
    glAttachShader(m_ID, vertexShader);
    glAttachShader(m_ID, fragmentShader);
    if (geometryShader != 0) {
        glAttachShader(m_ID, geometryShader);
    }
    glLinkProgram(m_ID);

    int linkStatus;
    char infoLog[512];
    glGetProgramiv(m_ID, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus) {
        glGetProgramInfoLog(m_ID, 512, NULL, infoLog);
        std::cerr << "[Error] Shader::linkProgram: Linking failed\n" << infoLog << "\n";
        *status = -1;
        m_ID = 0;
    } else {
        *status = 0;
    }
}
