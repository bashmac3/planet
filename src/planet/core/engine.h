#pragma once

#include <string>
#include <GLFW/glfw3.h>

namespace planet {

class KerdataArchive;

struct EngineConfig {
    int windowWidth = 1280;
    int windowHeight = 720;
    std::string windowTitle = "Planet";
    bool fullscreen = false;
    bool vsync = false;
    int targetFPS = 1000;
    std::string scriptPath = "scripts/main.lua";
    std::string assetPath = "assets/";
    std::string kerdataPath;
    bool termOutput = false;
    bool consoleEnabled = false;
};

class Engine {
public:
    static Engine& Instance();

    bool Init(const EngineConfig& config = EngineConfig{});
    void Run();
    void Shutdown();

    bool IsRunning() const { return m_running; }
    double GetDeltaTime() const { return m_deltaTime; }
    double GetElapsedTime() const { return m_elapsedTime; }
    const EngineConfig& GetConfig() const { return m_config; }

    void Quit();

    void SetTargetFPS(int fps);
    int GetTargetFPS() const { return m_config.targetFPS; }
    uint64_t GetFrameCount() const { return m_frameCount; }
    double GetAverageFPS() const;
    double GetTimeSinceStart() const { return glfwGetTime(); }

    void SetVSync(bool enabled);
    bool IsVSync() const { return m_config.vsync; }

    void SetFullscreen(bool fullscreen);
    bool IsFullscreen() const { return m_config.fullscreen; }

    void SetConsoleEnabled(bool enabled) { m_config.consoleEnabled = enabled; }
    void SetAssetPath(const std::string& path) { m_config.assetPath = path; }

private:
    Engine() = default;
    ~Engine() = default;

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    EngineConfig m_config;
    bool m_running = false;
    double m_deltaTime = 0.0;
    double m_elapsedTime = 0.0;
    double m_lastFrameTime = 0.0;
    uint64_t m_frameCount = 0;
};

} // namespace planet
