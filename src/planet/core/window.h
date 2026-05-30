#pragma once

#define GLFW_INCLUDE_NONE

#include <string>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace planet {

class Window {
public:
    static Window& Instance();

    bool Create(int width, int height, const std::string& title, bool fullscreen = false);
    void Destroy();
    bool ShouldClose() const;
    void SwapBuffers();

    GLFWwindow* GetGLFWWindow() const { return m_window; }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    float GetAspectRatio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }

    void SetTitle(const std::string& title);
    void SetIcon(const std::string& iconPath);
    void SetVSync(bool enabled);
    bool IsVSync() const { return m_vsync; }

    void SetSize(int width, int height);
    void SetFullscreen(bool fullscreen);
    bool IsFullscreen() const { return m_fullscreen; }

    void Minimize();
    void Restore();
    void Maximize();
    void Focus();
    bool IsFocused() const;
    bool IsMinimized() const;
    void SetOpacity(float opacity);
    void SetSizeLimits(int minW, int minH, int maxW, int maxH);

    void SetSwapInterval(int interval);

    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void WindowFocusCallback(GLFWwindow* window, int focused);

private:
    Window() = default;
    ~Window() = default;

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    GLFWwindow* m_window = nullptr;
    int m_width = 0;
    int m_height = 0;
    bool m_vsync = false;
    bool m_fullscreen = false;
    int m_windowedX = 0, m_windowedY = 0;
    int m_windowedW = 1280, m_windowedH = 720;
};

} // namespace planet
