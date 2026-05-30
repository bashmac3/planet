#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <glm/glm.hpp>

namespace planet {

class Entity;

class DebugServer {
public:
    static DebugServer& Instance();

    void Start(int port = 9876);
    void Stop();
    bool IsRunning() const { return m_running; }

    void Update();

    void NotifyScriptInvoke(Entity* entity, const std::string& scriptPath);

private:
    DebugServer() = default;
    ~DebugServer();

    DebugServer(const DebugServer&) = delete;
    DebugServer& operator=(const DebugServer&) = delete;

    void ServerThread(int port);
    void SendToClient(const std::string& data);
    std::string BuildSnapshotJson();
    std::string EntityToJson(Entity* e);
    std::string EscapeJson(const std::string& s);
    std::string Vec3ToJson(const glm::vec3& v);

    int m_port = 9876;
    std::atomic<bool> m_running{false};
    std::thread m_serverThread;
    int m_serverFd = -1;
    int m_clientFd = -1;
    std::mutex m_mutex;
    std::mutex m_sendMutex;
    std::vector<std::string> m_pendingEvents;
    double m_lastSnapshotTime = 0.0;
};

} // namespace planet
