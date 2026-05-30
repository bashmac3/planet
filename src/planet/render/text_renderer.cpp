#include "planet/render/text_renderer.h"
#include "planet/render/shader.h"
#include "planet/render/renderer.h"
#include "planet/render/font_data.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace planet {

static const char* TEXT_VERT = R"(
#version 410 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;
uniform mat4 uProjection;
out vec2 vTexCoord;
out vec4 vColor;
void main() {
    vTexCoord = aTexCoord;
    vColor = aColor;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

static const char* TEXT_FRAG = R"(
#version 410 core
in vec2 vTexCoord;
in vec4 vColor;
uniform sampler2D uTexture;
out vec4 FragColor;
void main() {
    float alpha = texture(uTexture, vTexCoord).r;
    FragColor = vec4(vColor.rgb, vColor.a * alpha);
}
)";

TextRenderer& TextRenderer::Instance() {
    static TextRenderer instance;
    return instance;
}

static bool LoadTTF(const char* path, std::vector<unsigned char>& buf) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz <= 0) { fclose(f); return false; }
    fseek(f, 0, SEEK_SET);
    buf.resize(static_cast<size_t>(sz));
    bool ok = fread(buf.data(), 1, sz, f) == static_cast<size_t>(sz);
    fclose(f);
    return ok;
}

void TextRenderer::Init() {
    std::vector<unsigned char> atlas(ATLAS_W * ATLAS_H, 0);
    std::vector<unsigned char> ttfBuf;

    static const char* fontPaths[] = {
        "fonts/NotoSansMono-Regular.ttf",
        "/usr/share/fonts/noto/NotoSansMono-Regular.ttf",
    };
    bool fontLoaded = false;
    for (const char* fp : fontPaths) {
        if (LoadTTF(fp, ttfBuf)) {
            fontLoaded = true;
            break;
        }
    }

    if (fontLoaded) {
        stbtt_fontinfo info;
        if (stbtt_InitFont(&info, ttfBuf.data(), 0)) {
            float scale = stbtt_ScaleForPixelHeight(&info, FONT_HEIGHT);
            int ascent;
            stbtt_GetFontVMetrics(&info, &ascent, nullptr, nullptr);
            m_bakedAscent = ascent * scale;

            stbtt_bakedchar chardata[NUM_GLYPHS];
            int result = stbtt_BakeFontBitmap(ttfBuf.data(), 0, FONT_HEIGHT,
                atlas.data(), ATLAS_W, ATLAS_H, FIRST_CHAR, NUM_GLYPHS, chardata);

            if (result > 0) {
                m_ttfLoaded = true;
                float sumAdvance = 0;
                for (int i = 0; i < NUM_GLYPHS; i++) {
                    m_glyphs[i].u0 = chardata[i].x0 / (float)ATLAS_W;
                    m_glyphs[i].v0 = chardata[i].y0 / (float)ATLAS_H;
                    m_glyphs[i].u1 = chardata[i].x1 / (float)ATLAS_W;
                    m_glyphs[i].v1 = chardata[i].y1 / (float)ATLAS_H;
                    m_glyphs[i].xoff = chardata[i].xoff;
                    m_glyphs[i].yoff = chardata[i].yoff;
                    m_glyphs[i].advance = chardata[i].xadvance;
                    sumAdvance += chardata[i].xadvance;
                }
                m_avgAdvance = sumAdvance / NUM_GLYPHS;

                glGenTextures(1, &m_fontTexID);
                glBindTexture(GL_TEXTURE_2D, m_fontTexID);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, ATLAS_W, ATLAS_H, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }

    if (!m_ttfLoaded) {
        static const int CHAR_W = 8;
        static const int CHAR_H = 16;
        static const int CHARS_PER_ROW = 16;
        static const int TEX_W = CHAR_W * CHARS_PER_ROW;
        static const int TEX_H = CHAR_H * ((NUM_GLYPHS + CHARS_PER_ROW - 1) / CHARS_PER_ROW);

        atlas.resize(TEX_W * TEX_H, 0);

        for (int c = 0; c < NUM_GLYPHS; c++) {
            int tileX = c % CHARS_PER_ROW;
            int tileY = c / CHARS_PER_ROW;
            for (int row = 0; row < CHAR_H; row++) {
                unsigned char bits = g_font8x16[c][row];
                for (int col = 0; col < CHAR_W; col++) {
                    if (bits & (0x80 >> col)) {
                        int px = tileX * CHAR_W + col;
                        int py = tileY * CHAR_H + row;
                        atlas[py * TEX_W + px] = 255;
                    }
                }
            }
        }

        m_bakedAscent = static_cast<float>(CHAR_H);
        for (int i = 0; i < NUM_GLYPHS; i++) {
            int tileX = i % CHARS_PER_ROW;
            int tileY = i / CHARS_PER_ROW;
            m_glyphs[i].u0 = (tileX * CHAR_W + 0.5f) / TEX_W;
            m_glyphs[i].v0 = (tileY * CHAR_H + 0.5f) / TEX_H;
            m_glyphs[i].u1 = ((tileX + 1) * CHAR_W - 0.5f) / TEX_W;
            m_glyphs[i].v1 = ((tileY + 1) * CHAR_H - 0.5f) / TEX_H;
            m_glyphs[i].xoff = 0.0f;
            m_glyphs[i].yoff = -static_cast<float>(CHAR_H);
            m_glyphs[i].advance = static_cast<float>(CHAR_W);
        }

        glGenTextures(1, &m_fontTexID);
        glBindTexture(GL_TEXTURE_2D, m_fontTexID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, TEX_W, TEX_H, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    m_shader = std::make_unique<Shader>();
    m_shader->LoadFromSource(TEXT_VERT, TEXT_FRAG);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 16384 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
    glBindVertexArray(0);
}

void TextRenderer::Shutdown() {
    m_shader.reset();
    if (m_fontTexID) glDeleteTextures(1, &m_fontTexID);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

void TextRenderer::DrawString(const std::string& text, float x, float y, float scale,
                               const glm::vec4& color) {
    if (text.empty() || !m_shader || !m_fontTexID) return;

    size_t len = text.size();
    if (len > 128) len = 128;

    std::vector<float> vertices;
    vertices.reserve(len * 48);

    float pen_x = x;
    float baseline_y = y + m_bakedAscent * scale;

    for (size_t i = 0; i < len; i++) {
        int ch = static_cast<unsigned char>(text[i]);
        if (ch < FIRST_CHAR || ch > 126) ch = FIRST_CHAR;
        int idx = ch - FIRST_CHAR;

        const GlyphInfo& g = m_glyphs[idx];

        float qx0 = pen_x + g.xoff * scale;
        float qy0 = baseline_y + g.yoff * scale;
        float w = (g.u1 - g.u0) * ATLAS_W * scale;
        float h = (g.v1 - g.v0) * ATLAS_H * scale;
        float qx1 = qx0 + w;
        float qy1 = qy0 + h;

        float verts[] = {
            qx0, qy1, g.u0, g.v1, color.r, color.g, color.b, color.a,
            qx0, qy0, g.u0, g.v0, color.r, color.g, color.b, color.a,
            qx1, qy0, g.u1, g.v0, color.r, color.g, color.b, color.a,

            qx0, qy1, g.u0, g.v1, color.r, color.g, color.b, color.a,
            qx1, qy0, g.u1, g.v0, color.r, color.g, color.b, color.a,
            qx1, qy1, g.u1, g.v1, color.r, color.g, color.b, color.a,
        };

        vertices.insert(vertices.end(), verts, verts + 48);
        pen_x += g.advance * scale;
    }

    m_shader->Bind();
    m_shader->SetMat4("uProjection", Renderer::Instance().GetProjectionMatrix());

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fontTexID);
    m_shader->SetInt("uTexture", 0);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(len * 6));

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    m_shader->Unbind();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void TextRenderer::DrawStringCentered(const std::string& text, float centerX, float y,
                                       float scale, const glm::vec4& color) {
    // Rough width estimate using first glyph's advance
    float width = 0;
    for (size_t i = 0; i < text.size(); i++) {
        int ch = static_cast<unsigned char>(text[i]);
        if (ch < FIRST_CHAR || ch > 126) ch = FIRST_CHAR;
        width += m_glyphs[ch - FIRST_CHAR].advance * scale;
    }
    DrawString(text, centerX - width * 0.5f, y, scale, color);
}

float TextRenderer::MeasureStringWidth(const std::string& text, float scale) const {
    float width = 0;
    for (size_t i = 0; i < text.size(); i++) {
        int ch = static_cast<unsigned char>(text[i]);
        if (ch < FIRST_CHAR || ch > 126) ch = FIRST_CHAR;
        width += m_glyphs[ch - FIRST_CHAR].advance * scale;
    }
    return width;
}

} // namespace planet