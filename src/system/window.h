#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Window {
public:
    // Constructor and Destructor
    Window(const char *title, int width, int height);
    ~Window();

    bool init();

    /*
     * Input Handling
     */
    void processInput();

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

    /*
     * Attach a renderer for G-buffer resizing
     */
    void setRenderer(void* renderer);

private:
    // GLFW window object
    GLFWwindow* m_window;

    // Window settings
    const char* m_title;
    int m_width;
    int m_height;

    // Renderer associated with this window (for resizing G-buffer)
    void* m_renderer;

    // Static callback function for resizing
    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
};

#endif
