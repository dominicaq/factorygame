#ifndef VIEW_FRAME_BUFFERS_H
#define VIEW_FRAME_BUFFERS_H

#include "engine.h"

class ViewFrameBuffers : public Script {
public:
    void cycleModes() {
        if (inputManager.isKeyPressed(GLFW_KEY_1)) {
            // Turn off debug mode
            DEBUG_view_framebuffers = -1;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_2)) {
            // Position
            DEBUG_view_framebuffers = 0;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_3)) {
            // Normal
            DEBUG_view_framebuffers = 1;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_4)) {
            // Albedo
            DEBUG_view_framebuffers = 2;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_5)) {
            // Depth
            DEBUG_view_framebuffers = 3;
        }
    }

    void update(float deltaTime) override {
        cycleModes();
    }
};

#endif // VIEW_FRAME_BUFFERS_H
