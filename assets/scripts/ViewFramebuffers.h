#pragma once

#include "engine.h"

class ViewFrameBuffers : public Script {
public:
    void cycleModes() {
        if (inputManager.isKeyPressed(GLFW_KEY_1)) {
            // Turn off debug mode
            DEBUG_PASS_MODE = -1;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_2)) {
            // Position
            DEBUG_PASS_MODE = 0;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_3)) {
            // Normal
            DEBUG_PASS_MODE = 1;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_4)) {
            // Albedo
            DEBUG_PASS_MODE = 2;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_5)) {
            // Depth
            DEBUG_PASS_MODE = 3;
        }
    }

    void update(float deltaTime) override {
        cycleModes();
    }
};
