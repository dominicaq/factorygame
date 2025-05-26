#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <vector>

namespace config {

#define SHADOW_MAX_CASCADES 6
#define TEXTURE_POOL_SIZE 256

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
        float shadowBias = 0.005f;

        int shadowResolution = 1024; // was 1024, need to implement different size resolutions later
        int directionalLightResolution = 4096; // not being used (yet)

        // Cascade shadow settings
        struct CascadeSettings {
            int numCascades = 3;  // One more cascade for better quality
            float cascadeSplitLambda = 0.95f; // [0.1f - 1.0f] range
        } cascades;
    } shadows;
};

}

#endif // SETTINGS_H

