#include "planet/core/engine.h"
#include "planet/core/window.h"
#include "planet/core/input.h"
#include "planet/core/scene.h"
#include "planet/core/console.h"
#include "planet/core/logger.h"
#include "planet/render/renderer.h"
#include "planet/render/model_renderer.h"
#include "planet/render/sprite_renderer.h"
#include "planet/render/post_processor.h"
#ifdef PLANET_USE_FMOD
#include "planet/audio/fmod_audio.h"
#else
#include "planet/audio/audio.h"
#endif
#include "planet/physics/physics.h"
#include "planet/resource/resource_manager.h"
#include "planet/ecs/system.h"
#include "planet/core/timer.h"
#include "lua/lua_engine.h"

#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>

namespace planet {

Engine& Engine::Instance() {
    static Engine instance;
    return instance;
}

bool Engine::Init(const EngineConfig& config) {
    m_config = config;
    g_termEnabled = m_config.termOutput;

    LOG_INFO() << "[Engine] Initializing...";

    if (!Window::Instance().Create(m_config.windowWidth, m_config.windowHeight,
                                    m_config.windowTitle, m_config.fullscreen)) {
        LOG_ERROR() << "[Engine] Failed to create window";
        return false;
    }

    if (m_config.vsync) {
        glfwSwapInterval(1);
    }

    Input::Instance().Init(Window::Instance().GetGLFWWindow());

    Renderer::Instance().Init(Window::Instance().GetWidth(), Window::Instance().GetHeight());

    PostProcessor::Instance().Init(Window::Instance().GetWidth(), Window::Instance().GetHeight());

#ifdef PLANET_USE_FMOD
    FmodAudio::Instance().Init();
#else
    Audio::Instance().Init();
#endif

    Physics::Instance().Init();

    ResourceManager::Instance().Init(m_config.assetPath);

    if (!m_config.kerdataPath.empty()) {
        ResourceManager::Instance().LoadKerdata(m_config.kerdataPath);
        // Auto-detect main script from kerdata manifest
        if (ResourceManager::Instance().HasKerdata()) {
            std::vector<uint8_t> manifestData;
            if (ResourceManager::Instance().ReadKerdataFile("manifest.ini", manifestData)) {
                std::string manifest(manifestData.begin(), manifestData.end());
                size_t pos = manifest.find("main:");
                if (pos != std::string::npos) {
                    size_t end = manifest.find('\n', pos);
                    std::string mainScript = manifest.substr(pos + 5, end - pos - 5);
                    // trim whitespace
                    mainScript.erase(0, mainScript.find_first_not_of(" \t\r\n"));
                    mainScript.erase(mainScript.find_last_not_of(" \t\r\n") + 1);
                    if (!mainScript.empty()) {
                        m_config.scriptPath = mainScript;
                        LOG_INFO() << "[Engine] Kerdata main script: " << mainScript;
                    }
                }
            }
        }
    }

    if (!LuaRuntime::Instance().Init(m_config.scriptPath)) {
        LOG_ERROR() << "[Engine] Failed to initialize Lua engine!";
        return false;
    }

    Console::Instance().Init();
    Console::Instance().SetEnabled(m_config.consoleEnabled);

    m_running = true;
    LOG_INFO() << "[Engine] Initialization complete.";
    return true;
}

void Engine::Run() {
    Scene& scene = Scene::Instance();
    scene.Load();

    LuaRuntime::Instance().CallFunction("onStart");

    SystemManager::Instance().RegisterSystem(std::make_unique<MeshRenderSystem>());
    SystemManager::Instance().RegisterSystem(std::make_unique<PhysicsSystem>());

    auto& sysMgr = SystemManager::Instance();
    auto& console = Console::Instance();

    while (m_running && !Window::Instance().ShouldClose()) {
        double currentTime = glfwGetTime();
        m_deltaTime = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;
        m_elapsedTime += m_deltaTime;

        glfwPollEvents();

        // Console toggle via grave key
        if (Input::Instance().GetKeyDown(KeyCode::GraveAccent)) {
            console.Toggle();
            Input::Instance().ClearTypedChars();
        }

        // Feed typed characters to console
        for (auto ch : Input::Instance().GetTypedChars()) {
            console.AddChar(ch);
        }

        console.Update(m_deltaTime);

        if (!console.IsOpen()) {
            LuaRuntime::Instance().CallFunction("onUpdate", m_deltaTime);
        }

#ifdef PLANET_USE_FMOD
        FmodAudio::Instance().Update();
#else
        Audio::Instance().Update();
#endif

        Physics::Instance().Update(static_cast<float>(m_deltaTime));
        TimerManager::Instance().Update(m_deltaTime);
        scene.Update(m_deltaTime);

        Renderer::Instance().BeginFrame();

        auto* cam = scene.GetActiveCamera();
        if (cam) {
            Renderer::Instance().SetViewMatrix(cam->GetViewMatrix());
            float aspect = Window::Instance().GetAspectRatio();
            Renderer::Instance().SetProjectionMatrix(cam->GetProjectionMatrix(aspect));
        }

        PostProcessor::Instance().SetTime(static_cast<float>(m_elapsedTime));
        PostProcessor::Instance().BeginCapture();

        ModelRenderer::Instance().BeginFrame();
        sysMgr.UpdateAll(m_deltaTime);

        // Debug light vector overlay
        if (console.showLightVector) {
            static Mesh debugSphere(Mesh::CreateSphere(0.3f, 8));
            static Mesh debugCube(Mesh::CreateCube(0.2f));

            auto& renderer = Renderer::Instance();
            glm::vec3 lightDir = renderer.GetLightDirection() * 8.0f;
            glm::vec3 origin(0.0f, 0.5f, 0.0f);

            ModelRenderer::Instance().SubmitMesh(&debugSphere, nullptr,
                glm::translate(glm::mat4(1.0f), origin + lightDir),
                glm::vec4(1.0f, 0.85f, 0.1f, 1.0f));
            ModelRenderer::Instance().SubmitMesh(&debugCube, nullptr,
                glm::translate(glm::mat4(1.0f), origin),
                glm::vec4(1.0f, 0.2f, 0.2f, 1.0f));

            // Draw a line from origin to light endpoint using small cubes
            glm::vec3 dir = glm::normalize(lightDir);
            float len = glm::length(lightDir);
            for (float t = 0.2f; t < len; t += 0.5f) {
                ModelRenderer::Instance().SubmitMesh(&debugCube, nullptr,
                    glm::translate(glm::mat4(1.0f), origin + dir * t) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.05f, 0.05f)),
                    glm::vec4(1.0f, 0.85f, 0.1f, 0.7f));
            }
        }

        ModelRenderer::Instance().EndFrame();

        PostProcessor::Instance().EndCapture();

        // 2D overlay
        {
            float w = static_cast<float>(Window::Instance().GetWidth());
            float h = static_cast<float>(Window::Instance().GetHeight());
            glm::mat4 orthoProj = glm::ortho(0.0f, w, h, 0.0f, -1.0f, 1.0f);
            Renderer::Instance().SetProjectionMatrix(orthoProj);
            Renderer::Instance().SetViewMatrix(glm::mat4(1.0f));
        }

        if (!console.IsOpen()) {
            LuaRuntime::Instance().CallFunction("onDraw");
        }

        // Draw instance info overlay
        if (console.showInstanceInfo && !console.IsOpen()) {
            Entity* target = console.RaycastEntity();
            if (target) {
                auto pos = target->GetPosition();
                auto scale = target->GetScale();
                std::stringstream ss;
                ss << target->GetName() << " [" << pos.x << "," << pos.y << "," << pos.z << "] s:("
                   << scale.x << "," << scale.y << "," << scale.z << ")";
                std::string info = ss.str();

                auto& sr = SpriteRenderer::Instance();
                int w = Window::Instance().GetWidth();
                sr.DrawSprite(glm::vec2(w / 2.0f - 150, 60), glm::vec2(300, 24), glm::vec4(0, 0, 0, 0.7f));
                // Draw info line indicator
                for (size_t i = 0; i < info.size(); i++) {
                    if (info[i] != ' ') {
                        float x = w / 2.0f - 140 + i * 8;
                        sr.DrawSprite(glm::vec2(x, 68), glm::vec2(7, 2), glm::vec4(0, 0.9f, 0.2f, 0.9f));
                    }
                }
            }
        }

        if (console.IsOpen()) {
            console.Render();
        }

        Window::Instance().SwapBuffers();

        Input::Instance().LateUpdate();

        m_frameCount++;

        if (m_config.targetFPS > 0) {
            double targetFrameTime = 1.0 / m_config.targetFPS;
            double frameTime = glfwGetTime() - currentTime;
            if (frameTime < targetFrameTime) {
                double sleepTime = targetFrameTime - frameTime;
                if (sleepTime > 0) {
                    std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
                }
            }
        }
    }
}

void Engine::Shutdown() {
    LOG_INFO() << "[Engine] Shutting down...";

    LuaRuntime::Instance().CallFunction("onShutdown");
    LuaRuntime::Instance().Shutdown();

    Console::Instance().Shutdown();

    Scene::Instance().Unload();
    Physics::Instance().Shutdown();
#ifdef PLANET_USE_FMOD
    FmodAudio::Instance().Shutdown();
#else
    Audio::Instance().Shutdown();
#endif
    Renderer::Instance().Shutdown();
    PostProcessor::Instance().Shutdown();
    ResourceManager::Instance().Shutdown();
    Window::Instance().Destroy();

    LOG_INFO() << "[Engine] Shutdown complete.";
}

void Engine::Quit() {
    m_running = false;
}

void Engine::SetTargetFPS(int fps) {
    m_config.targetFPS = fps;
}

double Engine::GetAverageFPS() const {
    if (m_deltaTime > 0.0) {
        return 1.0 / m_deltaTime;
    }
    return 0.0;
}

void Engine::SetVSync(bool enabled) {
    m_config.vsync = enabled;
    Window::Instance().SetVSync(enabled);
}

void Engine::SetFullscreen(bool fullscreen) {
    m_config.fullscreen = fullscreen;
    Window::Instance().SetFullscreen(fullscreen);
}

} // namespace planet
