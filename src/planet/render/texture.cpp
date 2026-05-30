#include "planet/render/texture.h"
#include "planet/core/logger.h"
#include <glm/glm.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace planet {

Texture::Texture() {}

Texture::~Texture() {
    if (m_textureID) {
        glDeleteTextures(1, &m_textureID);
    }
}

Texture::Texture(Texture&& other) noexcept
    : m_textureID(other.m_textureID), m_width(other.m_width), m_height(other.m_height) {
    other.m_textureID = 0;
    other.m_width = 0;
    other.m_height = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (m_textureID) glDeleteTextures(1, &m_textureID);
        m_textureID = other.m_textureID;
        m_width = other.m_width;
        m_height = other.m_height;
        other.m_textureID = 0;
        other.m_width = 0;
        other.m_height = 0;
    }
    return *this;
}

bool Texture::LoadFromFile(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);

    int channels;
    unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &channels, 0);
    if (!data) {
        LOG_ERROR() << "[Texture] Failed to load: " << path;
        return false;
    }

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    GLint internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;

    if (m_textureID) glDeleteTextures(1, &m_textureID);

    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height,
                  0, format, GL_UNSIGNED_BYTE, data);

    bool pot = ((m_width & (m_width - 1)) == 0) && ((m_height & (m_height - 1)) == 0);
    if (pot) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pot ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

bool Texture::CreateEmpty(int width, int height, GLenum format) {
    m_width = width;
    m_height = height;

    if (m_textureID) glDeleteTextures(1, &m_textureID);

    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height,
                 0, format, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void Texture::Bind(GLuint slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetFilterMode(GLenum minFilter, GLenum magFilter) {
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(magFilter));
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetWrapMode(GLenum wrapS, GLenum wrapT) {
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrapT));
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::SaveToFile(const std::string& path) const {
    return false; // Not implemented - would require stb_image_write
}

glm::vec4 Texture::GetPixel(int x, int y) const {
    return glm::vec4(1.0f); // Not directly supported without render-to-texture
}

void Texture::SetPixel(int x, int y, const glm::vec4& color) {
    // Not directly supported - would need to re-upload
}

void Texture::Resize(int width, int height) {
    CreateEmpty(width, height, m_format);
}

void Texture::Clear(const glm::vec4& color) {
    // Not directly supported
}

void Texture::SetData(const void* data, int width, int height, GLenum format) {
    m_width = width;
    m_height = height;
    m_format = format;
    if (!m_textureID) glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::GenerateMipmaps() {
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace planet
