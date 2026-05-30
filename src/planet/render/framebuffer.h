#pragma once

#include <glad/glad.h>

namespace planet {

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    bool Init(int width, int height);
    void Resize(int width, int height);
    void Destroy();

    void Bind() const;
    void Unbind() const;

    GLuint GetColorTexture() const { return m_colorTexture; }
    GLuint GetFboId() const { return m_fbo; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    GLuint m_fbo = 0;
    GLuint m_colorTexture = 0;
    GLuint m_depthStencilRbo = 0;
    int m_width = 0;
    int m_height = 0;
};

} // namespace planet
