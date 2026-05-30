#include "planet/render/framebuffer.h"
#include "planet/core/logger.h"

namespace planet {

Framebuffer::Framebuffer() {}

Framebuffer::~Framebuffer() {
    Destroy();
}

bool Framebuffer::Init(int width, int height) {
    m_width = width;
    m_height = height;

    glad_glGenFramebuffers(1, &m_fbo);
    glad_glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glad_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

    glad_glGenRenderbuffers(1, &m_depthStencilRbo);
    glad_glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilRbo);
    glad_glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glad_glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencilRbo);

    if (glad_glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR() << "[Framebuffer] Failed to create framebuffer!";
        Destroy();
        return false;
    }

    glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);
    LOG_INFO() << "[Framebuffer] Created " << width << "x" << height;
    return true;
}

void Framebuffer::Resize(int width, int height) {
    if (width == m_width && height == m_height) return;
    Destroy();
    Init(width, height);
}

void Framebuffer::Destroy() {
    if (m_depthStencilRbo) { glad_glDeleteRenderbuffers(1, &m_depthStencilRbo); m_depthStencilRbo = 0; }
    if (m_colorTexture)     { glDeleteTextures(1, &m_colorTexture); m_colorTexture = 0; }
    if (m_fbo)             { glad_glDeleteFramebuffers(1, &m_fbo); m_fbo = 0; }
    m_width = 0;
    m_height = 0;
}

void Framebuffer::Bind() const {
    glad_glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::Unbind() const {
    glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace planet
