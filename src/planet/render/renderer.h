#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <memory>
#include <stack>

namespace planet {

class Shader;

class Renderer {
public:
    static Renderer& Instance();

    void Init(int width, int height);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void SetViewMatrix(const glm::mat4& view);
    void SetProjectionMatrix(const glm::mat4& proj);

    const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }

    void PushMatrix(const glm::mat4& matrix);
    void PopMatrix();
    const glm::mat4& GetTopMatrix() const;

    Shader* GetDefaultShader() const { return m_defaultShader.get(); }
    Shader* GetSpriteShader() const { return m_spriteShader.get(); }

    void ClearColor(float r, float g, float b, float a);
    void GetClearColor(float& r, float& g, float& b, float& a) const;
    void SetRenderMode(bool wireframe);
    bool IsWireframe() const { return m_wireframe; }

    void SetLightDirection(const glm::vec3& dir) { m_lightDir = glm::normalize(dir); }
    void SetLightColor(const glm::vec3& color) { m_lightColor = color; }
    void SetAmbientColor(const glm::vec3& color) { m_ambientColor = color; }

    const glm::vec3& GetLightDirection() const { return m_lightDir; }
    const glm::vec3& GetLightColor() const { return m_lightColor; }
    const glm::vec3& GetAmbientColor() const { return m_ambientColor; }

    void SetFogColor(const glm::vec3& color) { m_fogColor = color; }
    void SetFogDensity(float density) { m_fogDensity = density; }
    void SetFogStart(float start) { m_fogStart = start; }
    void SetFogEnd(float end) { m_fogEnd = end; }
    void SetFogEnabled(bool enabled) { m_fogEnabled = enabled; }

    const glm::vec3& GetFogColor() const { return m_fogColor; }
    float GetFogDensity() const { return m_fogDensity; }
    float GetFogStart() const { return m_fogStart; }
    float GetFogEnd() const { return m_fogEnd; }
    bool IsFogEnabled() const { return m_fogEnabled; }

    void SetFlashlightEnabled(bool enabled) { m_flashlightEnabled = enabled; }
    void SetFlashlightPos(const glm::vec3& pos) { m_flashlightPos = pos; }
    void SetFlashlightDir(const glm::vec3& dir) { m_flashlightDir = glm::normalize(dir); }
    void SetFlashlightColor(const glm::vec3& color) { m_flashlightColor = color; }
    void SetFlashlightCutoff(float cutoff) { m_flashlightCutoff = cutoff; }

    bool IsFlashlightEnabled() const { return m_flashlightEnabled; }
    const glm::vec3& GetFlashlightPos() const { return m_flashlightPos; }
    const glm::vec3& GetFlashlightDir() const { return m_flashlightDir; }
    const glm::vec3& GetFlashlightColor() const { return m_flashlightColor; }
    float GetFlashlightCutoff() const { return m_flashlightCutoff; }

    // --- POINT LIGHTS ---
    struct PointLight {
        glm::vec3 position{0.0f};
        glm::vec3 color{1.0f};
        float range = 5.0f;
    };
    static const int MAX_POINT_LIGHTS = 16;

    int AddPointLight(const glm::vec3& pos, const glm::vec3& color, float range);
    void RemovePointLight(int index);
    void ClearPointLights();
    void SetPointLightPosition(int index, const glm::vec3& pos);
    void SetPointLightColor(int index, const glm::vec3& color);
    int GetPointLightCount() const { return m_pointLightCount; }
    const PointLight* GetPointLights() const { return m_pointLights; }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    class Texture* GetAppleTexture() const { return m_appleTexture.get(); }

private:
    Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    int m_width = 0;
    int m_height = 0;

    glm::mat4 m_viewMatrix{1.0f};
    glm::mat4 m_projectionMatrix{1.0f};

    std::stack<glm::mat4> m_matrixStack;
    std::unique_ptr<Shader> m_defaultShader;
    std::unique_ptr<Shader> m_spriteShader;

    glm::vec4 m_clearColor{0.1f, 0.1f, 0.15f, 1.0f};

    glm::vec3 m_lightDir{0.5f, -0.8f, -0.3f};
    glm::vec3 m_lightColor{1.0f, 0.95f, 0.85f};
    glm::vec3 m_ambientColor{0.35f, 0.38f, 0.45f};

    glm::vec3 m_fogColor{0.6f, 0.6f, 0.6f};
    float m_fogDensity = 0.02f;
    float m_fogStart = 5.0f;
    float m_fogEnd = 50.0f;
    bool m_fogEnabled = false;

    bool m_flashlightEnabled = false;
    glm::vec3 m_flashlightPos{0.0f, 0.0f, 0.0f};
    glm::vec3 m_flashlightDir{0.0f, 0.0f, -1.0f};
    glm::vec3 m_flashlightColor{1.0f, 0.95f, 0.8f};
    float m_flashlightCutoff = 0.85f;

    bool m_wireframe = false;

    PointLight m_pointLights[MAX_POINT_LIGHTS];
    int m_pointLightCount = 0;

    std::unique_ptr<class Texture> m_appleTexture;
};

} // namespace planet
