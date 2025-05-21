#include "shader.h"
#include "../resources/resourceLoader.h"

#include <iostream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <set>

Shader::Shader() : m_ID(0) {}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    load(vertexPath, fragmentPath);
}

Shader::~Shader() {
    if (glIsProgram(m_ID)) {
        glDeleteProgram(m_ID);
    }
}

/*
* Process shader includes
*/
std::string Shader::processIncludes(const std::string& source, const std::string& shaderDirectory,
                                    std::set<std::string>& includedFiles) {
    std::istringstream stream(source);
    std::ostringstream processedSource;
    std::string line;
    std::regex includeRegex("#include\\s+\"([^\"]+)\"");

    while (std::getline(stream, line)) {
        std::smatch match;
        if (std::regex_search(line, match, includeRegex)) {
            // Found an include directive
            std::string includePath = match[1].str();

            // Construct the full path starting from shader directory
            std::filesystem::path fullPath = std::filesystem::path(shaderDirectory) / includePath;
            std::string canonicalPath = std::filesystem::absolute(fullPath).string();

            // Check if this file has already been included
            if (includedFiles.find(canonicalPath) != includedFiles.end()) {
                // File already included, add a comment instead
                continue;
            }

            // Mark this file as included
            includedFiles.insert(canonicalPath);

            // Read the included file
            std::string includedCode = ResourceLoader::readFile(canonicalPath);

            if (includedCode.empty()) {
                std::cerr << "[Error] Failed to include shader file: " << canonicalPath << "\n";
            } else {
                // The included file may also have includes, so process recursively
                // Use the directory of the included file as the base for any nested includes
                std::string includedFileDir = fullPath.parent_path().string();
                std::string processedInclude = processIncludes(includedCode, includedFileDir, includedFiles);

                // Add a comment to mark where the include happened and add the processed code
                processedSource << processedInclude << "\n";
            }
        } else {
            // Not an include line, just copy it
            processedSource << line << "\n";
        }
    }

    return processedSource.str();
}

// Public wrapper for processIncludes that initializes the tracking set
std::string Shader::processIncludes(const std::string& source, const std::string& shaderDirectory) {
    std::set<std::string> includedFiles;
    return processIncludes(source, shaderDirectory, includedFiles);
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

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform2fv(location, 1, glm::value_ptr(value));
    } else {
        std::cerr << "[Error] Shader::setVec2: Uniform not found: " << name << "\n";
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

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform4fv(location, 1, glm::value_ptr(value));
    } else {
        std::cerr << "[Error] Shader::setVec4: Uniform not found: " << name << "\n";
    }
}

/*
* Shader creation
*/
bool Shader::load(const std::string& vertexPath, const std::string& fragmentPath) {
    m_ID = 0;
    // Clear the uniform location cache when loading a new shader
    m_UniformLocationCache.clear();

    // Extract the shader directory paths
    std::filesystem::path vertexFsPath(vertexPath);
    std::filesystem::path fragmentFsPath(fragmentPath);
    std::string vertexDir = vertexFsPath.parent_path().string();
    std::string fragmentDir = fragmentFsPath.parent_path().string();

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

    // Process includes in both shaders (each with its own tracking set)
    vertexCode = processIncludes(vertexCode, vertexDir);
    fragmentCode = processIncludes(fragmentCode, fragmentDir);

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

    // Link shaders into a program
    int linkStatus = 0;
    linkProgram(vertexShader, fragmentShader, &linkStatus);
    if (linkStatus != 0) {
        std::cerr << "[Error] Shader::load: Program linking failed\n";
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    // Clean up shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
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

void Shader::linkProgram(unsigned int vertexShader, unsigned int fragmentShader, int* status) {
    m_ID = glCreateProgram();
    glAttachShader(m_ID, vertexShader);
    glAttachShader(m_ID, fragmentShader);
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
