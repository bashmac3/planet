#include "planet/core/input.h"
#include <cstring>

namespace planet {

Input& Input::Instance() {
    static Input instance;
    return instance;
}

void Input::Init(GLFWwindow* window) {
    m_window = window;
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetCharCallback(window, CharCallback);

    std::memset(m_keys, 0, sizeof(m_keys));
    std::memset(m_keysPrev, 0, sizeof(m_keysPrev));
    std::memset(m_mouseButtons, 0, sizeof(m_mouseButtons));
    std::memset(m_mouseButtonsPrev, 0, sizeof(m_mouseButtonsPrev));
}

void Input::LateUpdate() {
    std::memcpy(m_keysPrev, m_keys, sizeof(m_keys));
    std::memcpy(m_mouseButtonsPrev, m_mouseButtons, sizeof(m_mouseButtons));
    m_mouseDelta = m_mousePosition - m_mousePrevPosition;
    m_mousePrevPosition = m_mousePosition;
    m_scrollDelta = 0.0f;
    m_typedChars.clear();
}

bool Input::GetKey(KeyCode key) const {
    return m_keys[static_cast<int>(key)];
}

bool Input::GetKeyDown(KeyCode key) const {
    int k = static_cast<int>(key);
    return m_keys[k] && !m_keysPrev[k];
}

bool Input::GetKeyUp(KeyCode key) const {
    int k = static_cast<int>(key);
    return !m_keys[k] && m_keysPrev[k];
}

bool Input::GetMouseButton(MouseButton button) const {
    return m_mouseButtons[static_cast<int>(button)];
}

bool Input::GetMouseButtonDown(MouseButton button) const {
    int b = static_cast<int>(button);
    return m_mouseButtons[b] && !m_mouseButtonsPrev[b];
}

bool Input::GetMouseButtonUp(MouseButton button) const {
    int b = static_cast<int>(button);
    return !m_mouseButtons[b] && m_mouseButtonsPrev[b];
}

void Input::SetMouseLocked(bool locked) {
    m_mouseLocked = locked;
    if (locked) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

bool Input::GetAnyKeyDown() const {
    for (int i = 0; i <= GLFW_KEY_LAST; i++) {
        if (m_keys[i] && !m_keysPrev[i]) return true;
    }
    return false;
}

bool Input::GetAnyKey() const {
    for (int i = 0; i <= GLFW_KEY_LAST; i++) {
        if (m_keys[i]) return true;
    }
    return false;
}

bool Input::GetAnyMouseButtonDown() const {
    for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i++) {
        if (m_mouseButtons[i] && !m_mouseButtonsPrev[i]) return true;
    }
    return false;
}

const std::string Input::GetKeyName(KeyCode key) const {
    const char* name = glfwGetKeyName(static_cast<int>(key), 0);
    if (name) return std::string(name);
    return "";
}

void Input::KeyCallback(GLFWwindow*, int key, int, int action, int) {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        if (action == GLFW_PRESS) Input::Instance().m_keys[key] = true;
        else if (action == GLFW_RELEASE) Input::Instance().m_keys[key] = false;
    }
}

void Input::MouseButtonCallback(GLFWwindow*, int button, int action, int) {
    if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) {
        if (action == GLFW_PRESS) Input::Instance().m_mouseButtons[button] = true;
        else if (action == GLFW_RELEASE) Input::Instance().m_mouseButtons[button] = false;
    }
}

void Input::CursorPosCallback(GLFWwindow*, double xpos, double ypos) {
    Input::Instance().m_mousePosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
}

void Input::ScrollCallback(GLFWwindow*, double, double yoffset) {
    Input::Instance().m_scrollDelta = static_cast<float>(yoffset);
}

void Input::CharCallback(GLFWwindow*, unsigned int codepoint) {
    Input::Instance().m_typedChars.push_back(codepoint);
}

} // namespace planet
