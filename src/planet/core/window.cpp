#include "planet/core/window.h"
#include "planet/render/renderer.h"
#include <glad/glad.h>
#include "planet/core/logger.h"
#include <stb_image.h>

namespace planet {

Window& Window::Instance() {
    static Window instance;
    return instance;
}

bool Window::Create(int width, int height, const std::string& title, bool fullscreen) {
    if (!glfwInit()) {
        LOG_ERROR() << "[Window] Failed to initialize GLFW!";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    m_width = width;
    m_height = height;
    m_vsync = false;
    m_fullscreen = fullscreen;
    m_windowedW = width;
    m_windowedH = height;

    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    if (!m_window) {
        LOG_ERROR() << "[Window] Failed to create GLFW window!";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
    glfwSetWindowFocusCallback(m_window, WindowFocusCallback);

    if (fullscreen) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (mode) {
            glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
    }

    if (!gladLoadGL()) {
        LOG_ERROR() << "[Window] Failed to initialize GLAD!";
        return false;
    }

    glfwGetFramebufferSize(m_window, &m_width, &m_height);
    glViewport(0, 0, m_width, m_height);

    LOG_INFO() << "[Window] OpenGL " << glGetString(GL_VERSION);
    LOG_INFO() << "[Window] GPU: " << glGetString(GL_RENDERER);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void Window::Destroy() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::SwapBuffers() {
    glfwSwapBuffers(m_window);
}

void Window::SetTitle(const std::string& title) {
    glfwSetWindowTitle(m_window, title.c_str());
}

void Window::SetIcon(const std::string& iconPath) {
    int width, height, channels;
    unsigned char* data = stbi_load(iconPath.c_str(), &width, &height, &channels, 4);
    if (!data) {
        LOG_ERROR() << "[Window] Failed to load icon: " << iconPath;
        return;
    }
    GLFWimage icon;
    icon.width = width;
    icon.height = height;
    icon.pixels = data;
    glfwSetWindowIcon(m_window, 1, &icon);
    stbi_image_free(data);
}

void Window::SetVSync(bool enabled) {
    m_vsync = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

void Window::SetSize(int width, int height) {
    glfwSetWindowSize(m_window, width, height);
    m_width = width;
    m_height = height;
}

void Window::SetFullscreen(bool fullscreen) {
    m_fullscreen = fullscreen;
    if (fullscreen) {
        glfwGetWindowPos(m_window, &m_windowedX, &m_windowedY);
        glfwGetWindowSize(m_window, &m_windowedW, &m_windowedH);
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (mode) {
            glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
    } else {
        glfwSetWindowMonitor(m_window, nullptr, m_windowedX, m_windowedY, m_windowedW, m_windowedH, 0);
    }
}

void Window::Minimize() {
    glfwIconifyWindow(m_window);
}

void Window::Restore() {
    glfwRestoreWindow(m_window);
}

void Window::Maximize() {
    glfwMaximizeWindow(m_window);
}

void Window::Focus() {
    glfwFocusWindow(m_window);
}

bool Window::IsFocused() const {
    return glfwGetWindowAttrib(m_window, GLFW_FOCUSED) != 0;
}

bool Window::IsMinimized() const {
    return glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0;
}

void Window::SetOpacity(float opacity) {
    glfwSetWindowOpacity(m_window, opacity);
}

void Window::SetSizeLimits(int minW, int minH, int maxW, int maxH) {
    glfwSetWindowSizeLimits(m_window, minW, minH, maxW, maxH);
}

void Window::SetSwapInterval(int interval) {
    glfwSwapInterval(interval);
}

void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    Window::Instance().m_width = width;
    Window::Instance().m_height = height;
}

void Window::WindowFocusCallback(GLFWwindow* window, int focused) {
    // Can be extended for pause-on-focus-lost behavior
}

} // namespace planet
