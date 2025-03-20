#include "window.h"
#include "../renderer/renderer.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "implot/implot.h"

// Constructor and Destructor
Window::Window(const char *title, int width, int height)
    : m_title(title), m_width(width), m_height(height), m_window(nullptr), m_renderer(nullptr) {}

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
    std::cerr << "[Warning] MacOS only supports up to OpenGL 4.1\n";
    std::cerr << "[Warning] defaulting to OpenGL 4.1, errors may occur.\n";
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    std::cout << "Running on OpenGL 4.3\n";
#endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    m_window = glfwCreateWindow(m_width, m_height, m_title, NULL, NULL);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << "\n";
        glfwTerminate();
        return false;
    }

    // Make the window's context current
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

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
    glfwSetWindowSize(m_window, m_width, m_height);

    // Get the framebuffer size (in pixels) instead of using the window size
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);

    // Set the OpenGL viewport based on framebuffer size
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    // Notify the renderer about the framebuffer resize to adjust the G-buffer
    if (m_renderer) {
        Renderer* renderer = static_cast<Renderer*>(m_renderer);
        renderer->resizeGBuffer(framebufferWidth, framebufferHeight);
    }
}

void Window::setTitle(const std::string& title) {
    glfwSetWindowTitle(m_window, title.c_str());
}

GLFWwindow* Window::getGLFWwindow() {
    return m_window;
}

/*
 * Attach a renderer for G-buffer resizing
 */
void Window::setRenderer(void* renderer) {
    m_renderer = renderer;
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
        renderer->resizeGBuffer(width, height);
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
