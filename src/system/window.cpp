#include "window.h"
#include "../renderer/renderer.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "implot/implot.h"

// Constructor and Destructor
Window::Window(const char *title, int width, int height)
    : m_title(title), m_width(width), m_height(height), m_window(nullptr), m_renderer(nullptr)
{
    // Initialize settings with constructor parameters
    m_settings.width = width;
    m_settings.height = height;
}

Window::~Window() {
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwTerminate();
}

/*
 * Initialization
 */
bool Window::init() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << "\n";
        return false;
    }

    // Set GLFW window hints for OpenGL version and profile
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    std::cerr << "[Warning] macOS only supports up to OpenGL 4.1 due to OpenGL deprecation.\n";
    std::cerr << "[Warning] Defaulting to OpenGL 4.1. Some OpenGL features may cause errors or not function properly.\n";
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Apply MSAA setting
    if (m_settings.msaaSamples > 0) {
        glfwWindowHint(GLFW_SAMPLES, m_settings.msaaSamples);
    }

    // Apply borderless setting if needed
    if (m_settings.borderless) {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }

    // Create a windowed mode window and its OpenGL context
    GLFWmonitor* monitor = m_settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

    m_window = glfwCreateWindow(m_width, m_height, m_title, monitor, NULL);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << "\n";
        glfwTerminate();
        return false;
    }

    // Make the window's context current
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

    // Apply VSync setting
    setVSync(m_settings.vsync);

    // Set this Window object as the user pointer for the GLFW window
    glfwSetWindowUserPointer(m_window, this);

    // Load all OpenGL function pointers with GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << "\n";
        return false;
    }

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup ImPlot extension
    ImPlot::CreateContext();

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Initialize ImGui for GLFW and OpenGL3
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cerr << "[Success] Running OpenGL " << major << "." << minor << " Core Profile\n";
    if (GLAD_GL_ARB_compute_shader) {
        std::cerr << "[Success] Compute Shaders are supported!" << std::endl;
    } else {
        std::cerr << "[Warning] Compute Shaders are NOT supported." << std::endl;
    }
    return true;
}

/*
 * Window State
 */
bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

/*
 * Rendering
 */
void Window::swapBuffersAndPollEvents() {
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

/*
 * Window Management
 */
void Window::resize(int newWidth, int newHeight) {
    m_width = newWidth;
    m_height = newHeight;
    m_settings.width = newWidth;
    m_settings.height = newHeight;
    glfwSetWindowSize(m_window, m_width, m_height);

    // Get the framebuffer size (in pixels) instead of using the window size
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);

    // Set the OpenGL viewport based on framebuffer size
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    // Notify the renderer about the framebuffer resize to adjust the G-buffer
    if (m_renderer) {
        Renderer* renderer = static_cast<Renderer*>(m_renderer);
        renderer->resize(framebufferWidth, framebufferHeight);
    }
}

void Window::setTitle(const std::string& title) {
    glfwSetWindowTitle(m_window, title.c_str());
}

GLFWwindow* Window::getGLFWwindow() {
    return m_window;
}

/*
 * Settings Management
 */
bool Window::applySettings(const config::GraphicsSettings& settings) {
    bool requiresRestart = false;

    // Check if any settings require a restart
    if (settings.msaaSamples != m_settings.msaaSamples) {
        requiresRestart = true;
    }

    if (settings.borderless != m_settings.borderless) {
        requiresRestart = true;
    }

    // Apply settings that can be changed at runtime
    if (settings.width != m_settings.width || settings.height != m_settings.height) {
        resize(settings.width, settings.height);
    }

    if (settings.vsync != m_settings.vsync) {
        setVSync(settings.vsync);
    }

    if (settings.fullscreen != m_settings.fullscreen) {
        setFullscreen(settings.fullscreen);
    }

    // Apply renderer settings if a renderer is attached
    if (m_renderer) {
        Renderer* renderer = static_cast<Renderer*>(m_renderer);
        // Assume the renderer has a method to apply settings
        bool rendererNeedsRestart = renderer->applySettings(settings);
        requiresRestart = requiresRestart || rendererNeedsRestart;
    }

    // Save the new settings
    m_settings = settings;

    return requiresRestart;
}

void Window::setVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
    m_settings.vsync = enabled;
}

void Window::setFullscreen(bool fullscreen) {
    if (fullscreen == m_settings.fullscreen) {
        return;
    }

    if (fullscreen) {
        // Get the primary monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // Save windowed dimensions before switching
        if (!m_settings.fullscreen) {
            glfwGetWindowSize(m_window, &m_settings.width, &m_settings.height);
        }

        // Switch to fullscreen
        glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        m_width = mode->width;
        m_height = mode->height;
    } else {
        // Switch back to windowed mode
        glfwSetWindowMonitor(m_window, nullptr, 100, 100, m_settings.width, m_settings.height, 0);
        m_width = m_settings.width;
        m_height = m_settings.height;
    }

    m_settings.fullscreen = fullscreen;

    // Re-apply vsync setting as it might be reset when changing monitors
    setVSync(m_settings.vsync);
}

void Window::setBorderless(bool borderless) {
    m_settings.borderless = borderless;
}

void Window::setMSAA(int samples) {
    m_settings.msaaSamples = samples;
}

void Window::setMaxFrameRate(int frameRate) {
    m_settings.maxFrameRate = frameRate;
}

/*
 * Attach a renderer for G-buffer resizing
 */
void Window::setRenderer(void* renderer) {
    m_renderer = renderer;

    // Apply initial settings to the renderer
    if (m_renderer) {
        Renderer* rendererPtr = static_cast<Renderer*>(m_renderer);
        rendererPtr->applySettings(m_settings);
    }

    // Ensure the G-buffer is set up when the renderer is attached
    resize(m_width, m_height);
}

// Static callback for when the window is resized
void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Update OpenGL viewport using the framebuffer size
    glViewport(0, 0, width, height);

    // Get the window pointer associated with this callback
    Window* windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (windowPtr && windowPtr->m_renderer) {
        Renderer* renderer = static_cast<Renderer*>(windowPtr->m_renderer);
        renderer->resize(width, height);
    }
}

/*
 * ImGui Frame Control
 */
void Window::beginImGuiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Window::endImGuiFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
