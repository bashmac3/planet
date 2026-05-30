#include "planet/render/model_renderer.h"
#include "planet/render/renderer.h"
#include <glm/gtc/type_ptr.hpp>

namespace planet {

ModelRenderer& ModelRenderer::Instance() {
    static ModelRenderer instance;
    return instance;
}

void ModelRenderer::Init() {
    m_defaultShader = Renderer::Instance().GetDefaultShader();
    m_wireframeShader = Renderer::Instance().GetDefaultShader();
}

void ModelRenderer::Shutdown() {
    m_opaqueQueue.clear();
    m_transparentQueue.clear();
    m_wireframeQueue.clear();
}

void ModelRenderer::SubmitMesh(Mesh* mesh, Texture* texture, const glm::mat4& transform,
                                const glm::vec4& color, bool unlit) {
    RenderCommand cmd;
    cmd.mesh = mesh;
    cmd.texture = texture;
    cmd.transform = transform;
    cmd.color = color;
    cmd.unlit = unlit;

    if (color.a < 1.0f) {
        m_transparentQueue.push_back(cmd);
    } else {
        m_opaqueQueue.push_back(cmd);
    }
}

void ModelRenderer::SubmitWireframe(Mesh* mesh, const glm::mat4& transform,
                                     const glm::vec4& color) {
    RenderCommand cmd;
    cmd.mesh = mesh;
    cmd.transform = transform;
    cmd.color = color;
    m_wireframeQueue.push_back(cmd);
}

void ModelRenderer::BeginFrame() {
    m_opaqueQueue.clear();
    m_transparentQueue.clear();
    m_wireframeQueue.clear();
}

void ModelRenderer::EndFrame() {
    Flush();
}

void ModelRenderer::Flush() {
    Renderer& renderer = Renderer::Instance();

    auto renderCommands = [&](const std::vector<RenderCommand>& queue, bool wireframe) {
        Shader* shader = m_defaultShader;
        shader->Bind();
        shader->SetMat4("uView", renderer.GetViewMatrix());
        shader->SetMat4("uProjection", renderer.GetProjectionMatrix());
        shader->SetVec3("uLightDir", renderer.GetLightDirection());
        shader->SetVec3("uLightColor", renderer.GetLightColor());
        shader->SetVec3("uAmbientColor", renderer.GetAmbientColor());
        shader->SetInt("uFogEnabled", renderer.IsFogEnabled() ? 1 : 0);
        shader->SetVec3("uFogColor", renderer.GetFogColor());
        shader->SetFloat("uFogDensity", renderer.GetFogDensity());
        shader->SetFloat("uFogStart", renderer.GetFogStart());
        shader->SetFloat("uFogEnd", renderer.GetFogEnd());
        shader->SetVec3("uViewPos", glm::vec3(glm::inverse(renderer.GetViewMatrix())[3]));
        shader->SetInt("uFlashlightOn", renderer.IsFlashlightEnabled() ? 1 : 0);
        shader->SetVec3("uFlashlightPos", renderer.GetFlashlightPos());
        shader->SetVec3("uFlashlightDir", renderer.GetFlashlightDir());
        shader->SetVec3("uFlashlightColor", renderer.GetFlashlightColor());
        shader->SetFloat("uFlashlightCutoff", renderer.GetFlashlightCutoff());

        int lightCount = renderer.GetPointLightCount();
        shader->SetInt("uPointLightCount", lightCount);
        const auto* pointLights = renderer.GetPointLights();
        for (int i = 0; i < lightCount; i++) {
            std::string base = "uPointLightPositions[" + std::to_string(i) + "]";
            shader->SetVec3(base, pointLights[i].position);
            base = "uPointLightColors[" + std::to_string(i) + "]";
            shader->SetVec3(base, pointLights[i].color);
            base = "uPointLightRanges[" + std::to_string(i) + "]";
            shader->SetFloat(base, pointLights[i].range);
        }

        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        glDisable(GL_CULL_FACE);

        for (const auto& cmd : queue) {
            if (!cmd.mesh) continue;

            shader->SetMat4("uModel", cmd.transform);
            shader->SetVec4("uColor", cmd.color);
            shader->SetInt("uUnlit", cmd.unlit ? 1 : 0);

            if (cmd.texture) {
                cmd.texture->Bind(0);
                shader->SetInt("uTexture", 0);
                shader->SetInt("uUseTexture", 1);
            } else {
                shader->SetInt("uUseTexture", 0);
            }

            cmd.mesh->Draw();
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    };

    renderCommands(m_opaqueQueue, false);
    renderCommands(m_transparentQueue, false);
    renderCommands(m_wireframeQueue, true);

    m_defaultShader->Unbind();
}

} // namespace planet
