#pragma once

#include <string>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>  // Add this include

#ifdef _WIN32
    #include <windows.h>
    #include <intrin.h>
#elif __linux__
    #include <fstream>
#elif __APPLE__
    #include <sys/sysctl.h>
#endif

// Function to get CPU name
inline std::string getCPUName() {
    std::string cpuName = "Unknown CPU";

    #ifdef _WIN32
        int cpuInfo[4] = {0};
        char name[49] = {0};

        __cpuid(cpuInfo, 0x80000002);
        memcpy(name, cpuInfo, sizeof(cpuInfo));

        __cpuid(cpuInfo, 0x80000003);
        memcpy(name + 16, cpuInfo, sizeof(cpuInfo));

        __cpuid(cpuInfo, 0x80000004);
        memcpy(name + 32, cpuInfo, sizeof(cpuInfo));

        cpuName = std::string(name);
    #elif __linux__
        std::ifstream cpuFile("/proc/cpuinfo");
        std::string line;
        while (std::getline(cpuFile, line)) {
            if (line.find("model name") != std::string::npos) {
                cpuName = line.substr(line.find(":") + 2);
                break;
            }
        }
    #elif __APPLE__
        char name[256];
        size_t size = sizeof(name);
        sysctlbyname("machdep.cpu.brand_string", &name, &size, NULL, 0);
        cpuName = std::string(name);
    #endif

    return cpuName;
}

// Function to get GPU name
inline std::string getGPUName() {
    const GLubyte* renderer = glGetString(GL_RENDERER);

    if (renderer) {
        return std::string(reinterpret_cast<const char*>(renderer));
    }

    return "Unknown GPU";
}

// Function to print all relevant machine and GLFW information
void printPCInfo() {
    std::cout << "===================================\n";
    std::cout << "          HARDWARE INFO            \n";
    std::cout << "===================================\n";

    // CPU Info
    std::cout << "CPU: " << getCPUName() << "\n";

    // GPU Info
    std::cout << "GPU: " << getGPUName() << "\n";

    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "\nOpenGL Version: " << version << "\n";

    const GLubyte* vendor = glGetString(GL_VENDOR);
    std::cout << "OpenGL Vendor: " << vendor << "\n";

    const GLubyte* rendererStr = glGetString(GL_RENDERER);
    std::cout << "OpenGL Renderer: " << rendererStr << "\n";

    // Check hardware bindless texture support
    GLint numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    bool hasBindless = false;
    for (GLint i = 0; i < numExtensions; ++i) {
        const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, i);
        if (ext && strcmp(ext, "GL_ARB_bindless_texture") == 0) {
            hasBindless = true;
            break;
        }
    }
    std::cout << "GL_ARB_bindless_texture (Hardware): " << (hasBindless ? "Supported" : "NOT Supported") << "\n";

    // OpenGL Info
    GLint maxTextureSamplers = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureSamplers);
    std::cout << "\nMax Texture Samplers Supported: " << maxTextureSamplers << "\n";

    GLint maxVertexAttributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttributes);
    std::cout << "Max Vertex Attributes Supported: " << maxVertexAttributes << "\n";

    GLint maxCombinedTextureImageUnits = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedTextureImageUnits);
    std::cout << "Max Combined Texture Units: " << maxCombinedTextureImageUnits << "\n";

    GLint maxFragmentUniforms = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragmentUniforms);
    std::cout << "Max Fragment Shader Uniforms: " << maxFragmentUniforms << "\n";

    GLint maxVertexUniforms = 0;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUniforms);
    std::cout << "Max Vertex Shader Uniforms: " << maxVertexUniforms << "\n";

    std::cout << "\n===================================\n";
    std::cout << "             GLAD INFO             \n";
    std::cout << "===================================\n";

    // GLAD version support
    std::cout << "GLAD OpenGL 4.5 Support: " << (GLAD_GL_VERSION_4_5 ? "YES" : "NO") << "\n";
    std::cout << "GLAD OpenGL 4.6 Support: " << (GLAD_GL_VERSION_4_6 ? "YES" : "NO") << "\n";

    // Check if GLAD loaded bindless texture extension
    #ifdef GLAD_GL_ARB_bindless_texture
        std::cout << "GLAD_GL_ARB_bindless_texture: " << (GLAD_GL_ARB_bindless_texture ? "Loaded" : "Not Loaded") << "\n";
    #else
        std::cout << "GLAD_GL_ARB_bindless_texture: NOT DEFINED (extension not included in GLAD)\n";
    #endif

    // Check if GLAD loaded the function pointers
    std::cout << "\nGLAD Function Pointers:\n";
    std::cout << "  glGetTextureHandleARB: " << (glGetTextureHandleARB ? "Loaded" : "NULL") << "\n";
    std::cout << "  glMakeTextureHandleResidentARB: " << (glMakeTextureHandleResidentARB ? "Loaded" : "NULL") << "\n";
    std::cout << "  glMakeTextureHandleNonResidentARB: " << (glMakeTextureHandleNonResidentARB ? "Loaded" : "NULL") << "\n";

    // Test bindless texture creation if functions are available
    if (glGetTextureHandleARB && hasBindless) {
        std::cout << "\nTesting Bindless Texture Creation:\n";

        // Create a simple test texture
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Create a 1x1 white texture for testing
        unsigned char whitePixel[4] = {255, 255, 255, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Try to get bindless handle
        GLuint64 handle = glGetTextureHandleARB(textureID);

        if (handle != 0) {
            std::cout << "Successfully created bindless handle: 0x" << std::hex << handle << std::dec << "\n";

            // Try to make it resident
            glMakeTextureHandleResidentARB(handle);
            GLenum error = glGetError();

            if (error == GL_NO_ERROR) {
                std::cout << "Successfully made texture resident\n";
                std::cout << "Bindless textures are WORKING!\n";
                glMakeTextureHandleNonResidentARB(handle);
            } else {
                std::cout << "Error making texture resident: " << error << "\n";
            }
        } else {
            std::cout << "Failed to create bindless handle\n";
        }

        glDeleteTextures(1, &textureID);
    } else if (!hasBindless) {
        std::cout << "\n  Hardware does not support bindless textures\n";
    }

    std::cout << "\n===================================\n";
    std::cout << "             GLFW INFO            \n";
    std::cout << "===================================\n";

    // GLFW version info
    const char* glfwVersion = glfwGetVersionString();
    std::cout << "\nGLFW Version: " << glfwVersion << "\n";

    // GLFW version (Major.Minor.Revision)
    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    std::cout << "GLFW Version (Major.Minor.Revision): " << major << "." << minor << "." << rev << "\n";

    // GLFW monitor info
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    std::cout << "\nNumber of connected monitors: " << monitorCount << "\n";
    for (int i = 0; i < monitorCount; ++i) {
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        std::cout << "Monitor " << i + 1 << ": "
                  << mode->width << "x" << mode->height
                  << " @ " << mode->refreshRate << "Hz\n";
    }

    // OpenGL context version info
    GLint contextMajor, contextMinor;
    glGetIntegerv(GL_MAJOR_VERSION, &contextMajor);
    glGetIntegerv(GL_MINOR_VERSION, &contextMinor);
    std::cout << "\nOpenGL Context Version: " << contextMajor << "." << contextMinor << "\n";
    std::cout << "===================================\n";

    // Summary for bindless textures
    std::cout << "\n=== BINDLESS TEXTURE SUMMARY ===\n";
    if (glGetTextureHandleARB && hasBindless) {
        std::cout << "Bindless textures should work!\n";
    } else if (!hasBindless) {
        std::cout << "Hardware doesn't support bindless textures\n";
        std::cout << "  Consider using texture arrays or traditional binding\n";
    }

    std::cout << "================================\n";
}

