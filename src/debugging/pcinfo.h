#ifndef PCINFO_H
#define PCINFO_H

#include <string>
#include <GL/gl.h>
#include <iostream>

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

void printPCInfo() {
    std::cout << "---------- Machine Info ----------\n";
    std::cout << getCPUName() << "\n";
    std::cout << getGPUName() << "\n";
    std::cout << "----------------------------------\n";
}

#endif // PCINFO_H
