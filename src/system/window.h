#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Window {
public:
    // Constructor and Destructor
    Window(const char *title, unsigned int width, unsigned int height);
    ~Window();

    /*
     * Initialization
     */
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
    void setTitle(const char *newTitle);
    void resize(unsigned int newWidth, unsigned int newHeight);
    void getSize(int &width, int &height) const;

private:
    // GLFW window object
    GLFWwindow *m_window;

    // Window settings
    const char *title;
    unsigned int width;
    unsigned int height;

    // Static callback function for resizing
    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
};

#endif
