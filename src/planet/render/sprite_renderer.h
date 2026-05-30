#pragma once

#include "planet/render/texture.h"
#include "planet/render/shader.h"
#include <glm/glm.hpp>

namespace planet {

class SpriteRenderer {
public:
    static SpriteRenderer& Instance();

    void Init();
    void Shutdown();

    void DrawSprite(Texture* texture, const glm::vec2& position, const glm::vec2& size,
                    float rotation = 0.0f, const glm::vec4& tint = glm::vec4(1.0f),
                    const glm::vec2& pivot = glm::vec2(0.5f));
    void DrawSprite(const glm::vec2& position, const glm::vec2& size,
                    const glm::vec4& color, float rotation = 0.0f);

    void BeginBatch();
    void EndBatch();
    void Flush();

private:
    SpriteRenderer() = default;

    SpriteRenderer(const SpriteRenderer&) = delete;
    SpriteRenderer& operator=(const SpriteRenderer&) = delete;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    Shader* m_shader = nullptr;
};

} // namespace planet
