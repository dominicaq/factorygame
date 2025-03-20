#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <vector>

namespace config {

// Graphics settings structure that combines window and renderer settings
struct GraphicsSettings {
    // Window settings
    int width = 1280;
    int height = 720;
    bool vsync = true;
    bool fullscreen = false;
    int msaaSamples = 4;
    int maxFrameRate = 144;
    bool borderless = false;

    // Renderer settings
    bool enableShadows = true;
    int shadowResolution = 1024;
    float shadowBias = 0.005f;
    int textureQuality = 2;      // 0=low, 1=medium, 2=high
    bool enableSSAO = true;
    bool enableBloom = true;
    float gamma = 2.2f;
    bool enablePostProcessing = true;
};

}

#endif // SETTINGS_H

