#include "planet/mapper/mapper.h"
#include "planet/core/engine.h"
#include "planet/core/window.h"
#include "planet/core/input.h"
#include "planet/core/scene.h"
#include "planet/core/logger.h"
#include "planet/render/renderer.h"
#include "planet/render/model_renderer.h"
#include "planet/render/sprite_renderer.h"
#include "planet/render/text_renderer.h"
#include "planet/render/post_processor.h"
#include "planet/physics/physics.h"
#include "planet/resource/resource_manager.h"
#include "planet/ecs/system.h"

#include <GLFW/glfw3.h>

int main(int argc, char** argv) {
    using namespace planet;

    EngineConfig config;
    config.windowWidth = 800;
    config.windowHeight = 600;
    config.windowTitle = "Planet Mapper - 3D Viewport";

    if (!Window::Instance().Create(config.windowWidth, config.windowHeight,
                                    config.windowTitle, false)) {
        LOG_ERROR() << "[Mapper] Failed to create window";
        return 1;
    }
    glfwHideWindow(Window::Instance().GetGLFWWindow());
    glfwSwapInterval(0);

    Input::Instance().Init(Window::Instance().GetGLFWWindow());
    Renderer::Instance().Init(config.windowWidth, config.windowHeight);
    PostProcessor::Instance().Init(config.windowWidth, config.windowHeight);
    Physics::Instance().Init();
    ResourceManager::Instance().Init(config.assetPath);
    SpriteRenderer::Instance().Init();
    TextRenderer::Instance().Init();

    SystemManager::Instance().RegisterSystem(std::make_unique<MeshRenderSystem>());

    Renderer::Instance().ClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    Renderer::Instance().SetAmbientColor(glm::vec3(0.4f, 0.4f, 0.5f));

    Mapper::Instance().Init(argc, argv);
    Mapper::Instance().Run();
    Mapper::Instance().Shutdown();

    SpriteRenderer::Instance().Shutdown();
    TextRenderer::Instance().Shutdown();
    Scene::Instance().Unload();
    Physics::Instance().Shutdown();
    PostProcessor::Instance().Shutdown();
    Renderer::Instance().Shutdown();
    ResourceManager::Instance().Shutdown();
    Window::Instance().Destroy();

    return 0;
}
