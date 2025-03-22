#pragma once

#include "engine.h"

class ViewFrameBuffers : public Script {
public:
    void cycleModes() {
        if (inputManager.isKeyPressed(GLFW_KEY_1)) {
            // Turn off debug mode
            DEBUG_CTX.mode = -1;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_2)) {
            // Position
            DEBUG_CTX.mode = 0;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_3)) {
            // Normal
            DEBUG_CTX.mode = 1;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_4)) {
            // Albedo
            DEBUG_CTX.mode = 2;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_5)) {
            // Metallic
            DEBUG_CTX.mode = 3;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_6)) {
            // Roughness
            DEBUG_CTX.mode = 4;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_7)) {
            // Ambient Occlusion
            DEBUG_CTX.mode = 5;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_8)) {
            // Depth
            DEBUG_CTX.mode = 6;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_9)) {
            // Depth slices
            DEBUG_CTX.mode = 7;
        }
    }

    void update(const float& deltaTime) override {
        cycleModes();
    }
};
