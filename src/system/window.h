#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "config/settings.h"

class Window {
public:
    Window(const char *title, int width, int height);
    ~Window();

    bool init();

    /*
     * Window State
     */
    bool shouldClose() const;

    /*
     * Rendering
     */
    void swapBuffersAndPollEvents();

    /*
     * Window Management
     */
    void resize(int newWidth, int newHeight);
    void setTitle(const std::string& title);
    GLFWwindow* getGLFWwindow();

    /*
     * Settings Management
     */
    const config::GraphicsSettings& getSettings() const { return m_settings; }
    bool applySettings(const config::GraphicsSettings& settings);

    // Individual window setting methods
    void setVSync(bool enabled);
    void setFullscreen(bool fullscreen);
    void setBorderless(bool borderless);
    void setMSAA(int samples);
    void setMaxFrameRate(int frameRate);

    /*
     * Attach a renderer for G-buffer resizing
     */
    void setRenderer(void* renderer);

    /*
     * ImGUI
     */
    void beginImGuiFrame();
    void endImGuiFrame();

private:
    // GLFW window object
    GLFWwindow* m_window;

    // Window settings
    const char* m_title;
    int m_width;
    int m_height;
    config::GraphicsSettings m_settings;

    // Renderer associated with this window (for resizing G-buffer)
    void* m_renderer;

    // Static callback function for resizing
    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
};

#endif
