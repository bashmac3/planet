#include "planet/render/renderer.h"
#include "planet/render/shader.h"
#include "planet/render/sprite_renderer.h"
#include "planet/render/model_renderer.h"
#include "planet/render/text_renderer.h"
#include "planet/core/logger.h"

namespace planet {

static const char* DEFAULT_VERTEX = R"(
#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec4 aColor;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vTexCoord;
out vec4 vColor;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vFragPos = worldPos.xyz;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vTexCoord = aTexCoord;
    vColor = aColor;
    gl_Position = uProjection * uView * worldPos;
}
)";

static const char* DEFAULT_FRAGMENT = R"(
#version 410 core
in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uTexture;
uniform vec4 uColor;
uniform bool uUseTexture;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform bool uUnlit;
uniform bool uFogEnabled;
uniform vec3 uFogColor;
uniform float uFogDensity;
uniform float uFogStart;
uniform float uFogEnd;
uniform vec3 uViewPos;
uniform bool uFlashlightOn;
uniform vec3 uFlashlightPos;
uniform vec3 uFlashlightDir;
uniform vec3 uFlashlightColor;
uniform float uFlashlightCutoff;

uniform int uPointLightCount;
uniform vec3 uPointLightPositions[16];
uniform vec3 uPointLightColors[16];
uniform float uPointLightRanges[16];

out vec4 FragColor;

void main() {
    vec4 texColor = uUseTexture ? texture(uTexture, vTexCoord) : vec4(1.0);
    vec4 litColor;
    if (uUnlit) {
        litColor = texColor * vColor * uColor;
    } else {
        vec3 ambient = uAmbientColor * 0.3;
        vec3 norm = normalize(vNormal);
        vec3 lightDir = normalize(uLightDir);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = uLightColor * diff * 0.7;
        vec3 result = (ambient + diffuse) * texColor.rgb * vColor.rgb * uColor.rgb;

        if (uFlashlightOn) {
            vec3 toFrag = vFragPos - uFlashlightPos;
            float fragDist = length(toFrag);
            vec3 toFragDir = normalize(toFrag);
            float theta = dot(-toFragDir, normalize(uFlashlightDir));
            if (theta > uFlashlightCutoff) {
                float intensity = smoothstep(uFlashlightCutoff, uFlashlightCutoff + 0.08, theta);
                float attenuation = 1.0 / (1.0 + 0.05 * fragDist + 0.009 * fragDist * fragDist);
                vec3 flashlight = uFlashlightColor * intensity * attenuation;
                result += flashlight * texColor.rgb * vColor.rgb * uColor.rgb;
            }
        }

        for (int i = 0; i < uPointLightCount; i++) {
            vec3 lightVec = uPointLightPositions[i] - vFragPos;
            float dist = length(lightVec);
            if (dist < uPointLightRanges[i]) {
                vec3 lightDir = normalize(lightVec);
                float diff = max(dot(norm, lightDir), 0.0);
                float attenuation = 1.0 - (dist / uPointLightRanges[i]);
                attenuation = attenuation * attenuation;
                result += uPointLightColors[i] * diff * attenuation * 0.5;
            }
        }

        litColor = vec4(result, texColor.a * vColor.a * uColor.a);
    }

    if (uFogEnabled) {
        float dist = length(vFragPos - uViewPos);
        float fogFactor = exp(-uFogDensity * dist);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        litColor.rgb = mix(uFogColor, litColor.rgb, fogFactor);
    }

    FragColor = litColor;
}
)";

static const char* SPRITE_VERTEX = R"(
#version 410 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;

uniform mat4 uProjection;
uniform mat4 uModel;

out vec2 vTexCoord;
out vec4 vColor;

void main() {
    vTexCoord = aTexCoord;
    vColor = aColor;
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}
)";

static const char* SPRITE_FRAGMENT = R"(
#version 410 core
in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uTexture;
uniform bool uUseTexture;
uniform vec4 uSpriteColor;

out vec4 FragColor;

void main() {
    vec4 texColor = uUseTexture ? texture(uTexture, vTexCoord) : vec4(1.0);
    FragColor = texColor * vColor * uSpriteColor;
}
)";

Renderer& Renderer::Instance() {
    static Renderer instance;
    return instance;
}

void Renderer::Init(int width, int height) {
    m_width = width;
    m_height = height;

    m_defaultShader = std::make_unique<Shader>();
    m_defaultShader->LoadFromSource(DEFAULT_VERTEX, DEFAULT_FRAGMENT);

    m_spriteShader = std::make_unique<Shader>();
    m_spriteShader->LoadFromSource(SPRITE_VERTEX, SPRITE_FRAGMENT);

    m_matrixStack.push(glm::mat4(1.0f));
    m_viewMatrix = glm::mat4(1.0f);
    m_projectionMatrix = glm::ortho(0.0f, static_cast<float>(width),
                                      static_cast<float>(height), 0.0f, -1.0f, 1.0f);

    SpriteRenderer::Instance().Init();
    ModelRenderer::Instance().Init();
    TextRenderer::Instance().Init();

    LOG_INFO() << "[Renderer] Initialized (" << width << "x" << height << ")";

    // Preload apple texture
    {
        auto tex = std::make_unique<Texture>();
        if (tex->LoadFromFile("assets/apple.png")) {
            m_appleTexture = std::move(tex);
            LOG_INFO() << "[Renderer] Apple texture: " << m_appleTexture->GetWidth() << "x" << m_appleTexture->GetHeight();
        }
    }
}

void Renderer::Shutdown() {
    m_defaultShader.reset();
    m_spriteShader.reset();
    LOG_INFO() << "[Renderer] Shutdown";
}

void Renderer::BeginFrame() {
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
}

void Renderer::EndFrame() {
}

void Renderer::SetViewMatrix(const glm::mat4& view) {
    m_viewMatrix = view;
}

void Renderer::SetProjectionMatrix(const glm::mat4& proj) {
    m_projectionMatrix = proj;
}

void Renderer::PushMatrix(const glm::mat4& matrix) {
    m_matrixStack.push(matrix);
}

void Renderer::PopMatrix() {
    if (m_matrixStack.size() > 1) {
        m_matrixStack.pop();
    }
}

const glm::mat4& Renderer::GetTopMatrix() const {
    return m_matrixStack.top();
}

void Renderer::ClearColor(float r, float g, float b, float a) {
    m_clearColor = glm::vec4(r, g, b, a);
}

void Renderer::GetClearColor(float& r, float& g, float& b, float& a) const {
    r = m_clearColor.r;
    g = m_clearColor.g;
    b = m_clearColor.b;
    a = m_clearColor.a;
}

void Renderer::SetRenderMode(bool wireframe) {
    m_wireframe = wireframe;
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
}

int Renderer::AddPointLight(const glm::vec3& pos, const glm::vec3& color, float range) {
    if (m_pointLightCount >= MAX_POINT_LIGHTS) return -1;
    int idx = m_pointLightCount++;
    m_pointLights[idx] = {pos, color, range};
    return idx;
}

void Renderer::RemovePointLight(int index) {
    if (index < 0 || index >= m_pointLightCount) return;
    m_pointLights[index] = m_pointLights[m_pointLightCount - 1];
    m_pointLightCount--;
}

void Renderer::ClearPointLights() {
    m_pointLightCount = 0;
}

void Renderer::SetPointLightPosition(int index, const glm::vec3& pos) {
    if (index < 0 || index >= m_pointLightCount) return;
    m_pointLights[index].position = pos;
}

void Renderer::SetPointLightColor(int index, const glm::vec3& color) {
    if (index < 0 || index >= m_pointLightCount) return;
    m_pointLights[index].color = color;
}

} // namespace planet
