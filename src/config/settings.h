#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <vector>

namespace config {

#define SHADOW_MAX_CASCADES 6

// Graphics settings structure that combines window and renderer settings
struct GraphicsSettings {
    // Display settings
    struct DisplaySettings {
        int width = 1280;
        int height = 720;
        bool vsync = true;
        bool fullscreen = false;
        bool borderless = false;
        int maxFrameRate = 144;
    } display;

    // Quality settings
    struct QualitySettings {
        int msaaSamples = 4;
        int textureQuality = 2;  // 0=low, 1=medium, 2=high
        float gamma = 2.2f;
    } quality;

    // Post-processing settings
    struct PostProcessSettings {
        bool enablePostProcessing = true;
        bool enableSSAO = true;
        bool enableBloom = true;
    } postProcess;

    // Shadow settings
    struct ShadowSettings {
        bool enableShadows = true;
        int shadowResolution = 1024;
        float shadowBias = 0.005f;

        // Cascade shadow settings
        struct CascadeSettings {
            int numCascades = 4;
            float cascadeNearPlanes[4] = { 1.0f, 40.0f, 190.0f, 450.0f };
            float cascadeFarPlanes[4] = { 50.0f, 200.0f, 500.0f, 1000.0f };
            float cascadeSizeMultipliers[4] = { 0.5f, 1.0f, 2.0f, 4.0f };
            float cascadeCenterPositions[4] = { 25.0f, 120.0f, 350.0f, 725.0f };
            float shadowOrthoSize = 50.0f;
        } cascades;
    } shadows;
};

}

#endif // SETTINGS_H

