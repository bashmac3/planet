#pragma once

#include "planet/render/mesh.h"
#include "planet/render/shader.h"
#include "planet/render/texture.h"
#include <glm/glm.hpp>
#include <vector>

namespace planet {

class ModelRenderer {
public:
    struct RenderCommand {
        Mesh* mesh = nullptr;
        Texture* texture = nullptr;
        glm::mat4 transform{1.0f};
        glm::vec4 color{1.0f};
        bool unlit = false;
    };

    static ModelRenderer& Instance();

    void Init();
    void Shutdown();

    void SubmitMesh(Mesh* mesh, Texture* texture, const glm::mat4& transform,
                    const glm::vec4& color = glm::vec4(1.0f), bool unlit = false);
    void SubmitWireframe(Mesh* mesh, const glm::mat4& transform,
                         const glm::vec4& color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

    void BeginFrame();
    void EndFrame();
    void Flush();

private:
    ModelRenderer() = default;

    ModelRenderer(const ModelRenderer&) = delete;
    ModelRenderer& operator=(const ModelRenderer&) = delete;

    std::vector<RenderCommand> m_opaqueQueue;
    std::vector<RenderCommand> m_transparentQueue;
    std::vector<RenderCommand> m_wireframeQueue;

    Shader* m_defaultShader = nullptr;
    Shader* m_wireframeShader = nullptr;
};

} // namespace planet
