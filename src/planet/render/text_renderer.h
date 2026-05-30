#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <memory>

namespace planet {

class Shader;

class TextRenderer {
public:
    struct GlyphInfo {
        float u0, v0, u1, v1;
        float xoff, yoff;
        float advance;
    };

    static constexpr int NUM_GLYPHS = 95;
    static constexpr int FIRST_CHAR = 32;
    static constexpr float FONT_HEIGHT = 24.0f;
    static constexpr int ATLAS_W = 512;
    static constexpr int ATLAS_H = 512;

    static TextRenderer& Instance();

    void Init();
    void Shutdown();

    void DrawString(const std::string& text, float x, float y, float scale = 1.0f,
                    const glm::vec4& color = glm::vec4(0.0f, 1.0f, 0.2f, 1.0f));
    void DrawStringCentered(const std::string& text, float centerX, float y,
                            float scale = 1.0f,
                            const glm::vec4& color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    float MeasureStringWidth(const std::string& text, float scale = 1.0f) const;

    float GetCharWidth() const { return m_avgAdvance; }
    float GetCharHeight() const { return 16.0f; }

private:
    TextRenderer() = default;

    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_fontTexID = 0;
    std::unique_ptr<Shader> m_shader;

    GlyphInfo m_glyphs[NUM_GLYPHS]{};
    float m_bakedAscent = 0.0f;
    float m_avgAdvance = 8.0f;
    bool m_ttfLoaded = false;
};

} // namespace planet
