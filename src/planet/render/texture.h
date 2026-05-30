#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace planet {

class Texture {
public:
    Texture();
    ~Texture();

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    bool LoadFromFile(const std::string& path);
    bool CreateEmpty(int width, int height, GLenum format = GL_RGBA);

    void Bind(GLuint slot = 0) const;
    void Unbind() const;

    GLuint GetID() const { return m_textureID; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    void SetFilterMode(GLenum minFilter, GLenum magFilter);
    void SetWrapMode(GLenum wrapS, GLenum wrapT);

    bool SaveToFile(const std::string& path) const;
    glm::vec4 GetPixel(int x, int y) const;
    void SetPixel(int x, int y, const glm::vec4& color);
    void Resize(int width, int height);
    void Clear(const glm::vec4& color);

    GLenum GetInternalFormat() const { return m_internalFormat; }
    GLenum GetFormat() const { return m_format; }
    bool IsValid() const { return m_textureID != 0; }
    void SetData(const void* data, int width, int height, GLenum format = GL_RGBA);

    void GenerateMipmaps();

private:
    GLuint m_textureID = 0;
    int m_width = 0;
    int m_height = 0;
    GLenum m_internalFormat = GL_RGBA8;
    GLenum m_format = GL_RGBA;
};

} // namespace planet
