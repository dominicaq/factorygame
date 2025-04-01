#ifndef PCINFO_H
#define PCINFO_H

#include <string>
#include <iostream>
#include <GLFW/glfw3.h>

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

    // OpenGL context version (for GLFW)
    std::cout << "\nOpenGL Context Version: " << major << "." << minor << "\n";

    std::cout << "===================================\n";
}

#endif // PCINFO_H
