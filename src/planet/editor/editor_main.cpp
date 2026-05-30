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
#include "planet/ecs/ecs.h"
#include "planet/ecs/component.h"
#include "planet/core/timer.h"
#include "planet/editor/editor.h"
#include "planet/editor/log_console.h"
#include "planet/editor/script_editor.h"

#include <GLFW/glfw3.h>
#include <memory>

namespace planet {

static std::unique_ptr<Mesh> g_planeMesh;
static std::unique_ptr<Mesh> g_cubeMesh;
static std::unique_ptr<Mesh> g_sphereMesh;

static void CreateDefaultScene() {
    Scene& scene = Scene::Instance();
    scene.Load();

    g_planeMesh = std::make_unique<Mesh>(Mesh::CreatePlane(20, 20));
    g_cubeMesh = std::make_unique<Mesh>(Mesh::CreateCube(1.0f));
    g_sphereMesh = std::make_unique<Mesh>(Mesh::CreateSphere(0.5f, 16));

    Entity* ground = scene.CreateEntity("Ground");
    auto* gm = ground->AddComponent<MeshComponent>();
    gm->mesh = g_planeMesh.get();
    gm->color = glm::vec4(0.4f, 0.4f, 0.45f, 1.0f);
    ground->SetPosition(glm::vec3(0, -0.5f, 0));

    Entity* cube = scene.CreateEntity("Cube");
    auto* cm = cube->AddComponent<MeshComponent>();
    cm->mesh = g_cubeMesh.get();
    cm->color = glm::vec4(0.8f, 0.2f, 0.3f, 1.0f);
    cube->SetPosition(glm::vec3(0, 0.5f, 0));

    Entity* sphere = scene.CreateEntity("Sphere");
    auto* sm = sphere->AddComponent<MeshComponent>();
    sm->mesh = g_sphereMesh.get();
    sm->color = glm::vec4(0.2f, 0.6f, 0.3f, 1.0f);
    sphere->SetPosition(glm::vec3(2, 0.5f, 1));

    Entity* light = scene.CreateEntity("Sun");
    auto* lc = light->AddComponent<LightComponent>();
    lc->type = LightComponent::Directional;
    lc->color = glm::vec3(1, 0.95f, 0.85f);
    light->SetEulerAngles(glm::vec3(45, -30, 0));
}

} // namespace planet

// ── Entry point ────────────────────────────────────────────────

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    using namespace planet;

    EngineConfig config;
    config.windowWidth = 800;
    config.windowHeight = 600;
    config.windowTitle = "Planet Editor - Viewport";

    LOG_INFO() << "[Editor] Starting Planet TUI Editor...";

    // Create GLFW window (hidden by default, shown via F8)
    if (!Window::Instance().Create(config.windowWidth, config.windowHeight,
                                    config.windowTitle, false)) {
        LOG_ERROR() << "[Editor] Failed to create window";
        return 1;
    }
    glfwHideWindow(Window::Instance().GetGLFWWindow());
    glfwSwapInterval(0); // no vsync in editor

    // Init engine subsystems
    Input::Instance().Init(Window::Instance().GetGLFWWindow());
    Renderer::Instance().Init(config.windowWidth, config.windowHeight);
    PostProcessor::Instance().Init(config.windowWidth, config.windowHeight);
    Physics::Instance().Init();
    ResourceManager::Instance().Init(config.assetPath);
    SpriteRenderer::Instance().Init();
    TextRenderer::Instance().Init();

    Editor::Instance().Init(Window::Instance().GetGLFWWindow());
    EditorLog("TUI Editor initialized", 0);

    SystemManager::Instance().RegisterSystem(std::make_unique<MeshRenderSystem>());
    CreateDefaultScene();

    Renderer::Instance().ClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    Renderer::Instance().SetAmbientColor(glm::vec3(0.4f, 0.4f, 0.5f));
    Renderer::Instance().SetLightDirection(glm::vec3(0.5f, -0.8f, -0.3f));

    // Open a demo script
    ScriptEditor::Instance().NewFile();

    // Run TUI
    Editor::Instance().Run();

    // Cleanup
    Editor::Instance().Shutdown();
    SpriteRenderer::Instance().Shutdown();
    TextRenderer::Instance().Shutdown();
    Scene::Instance().Unload();
    g_planeMesh.reset();
    g_cubeMesh.reset();
    g_sphereMesh.reset();
    Physics::Instance().Shutdown();
    PostProcessor::Instance().Shutdown();
    Renderer::Instance().Shutdown();
    ResourceManager::Instance().Shutdown();
    Window::Instance().Destroy();

    LOG_INFO() << "[Editor] TUI Editor shut down.";
    return 0;
}
