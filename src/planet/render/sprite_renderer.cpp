#include "planet/render/sprite_renderer.h"
#include "planet/render/renderer.h"
#include <glm/gtc/matrix_transform.hpp>

namespace planet {

SpriteRenderer& SpriteRenderer::Instance() {
    static SpriteRenderer instance;
    return instance;
}

void SpriteRenderer::Init() {
    m_shader = Renderer::Instance().GetSpriteShader();

    float vertices[] = {
        0.0f, 1.0f,  0.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f,  0.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,  1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));

    glBindVertexArray(0);
}

void SpriteRenderer::Shutdown() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

void SpriteRenderer::DrawSprite(Texture* texture, const glm::vec2& position,
                                const glm::vec2& size, float rotation,
                                const glm::vec4& tint, const glm::vec2& pivot) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::translate(model, glm::vec3(pivot.x * size.x, pivot.y * size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-pivot.x * size.x, -pivot.y * size.y, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    m_shader->Bind();
    m_shader->SetMat4("uProjection", Renderer::Instance().GetProjectionMatrix());
    m_shader->SetMat4("uModel", model);
    m_shader->SetVec4("uSpriteColor", tint);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (texture) {
        texture->Bind(0);
        m_shader->SetInt("uTexture", 0);
        m_shader->SetInt("uUseTexture", 1);
    } else {
        m_shader->SetInt("uUseTexture", 0);
    }

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    m_shader->Unbind();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void SpriteRenderer::DrawSprite(const glm::vec2& position, const glm::vec2& size,
                                const glm::vec4& color, float rotation) {
    DrawSprite(nullptr, position, size, rotation, color);
}

void SpriteRenderer::BeginBatch() {}
void SpriteRenderer::EndBatch() {}
void SpriteRenderer::Flush() {}

} // namespace planet
