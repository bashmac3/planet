#include "planet/debug/debug_server.h"
#include "planet/core/engine.h"
#include "planet/core/scene.h"
#include "planet/ecs/ecs.h"
#include "planet/ecs/component.h"
#include "planet/core/logger.h"

#include <cstring>
#include <sstream>
#include <glm/glm.hpp>

#ifndef PLANET_BUILD_WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

namespace planet {

DebugServer& DebugServer::Instance() {
    static DebugServer instance;
    return instance;
}

DebugServer::~DebugServer() {
    Stop();
}

#ifndef PLANET_BUILD_WINDOWS
void DebugServer::Start(int port) {
    if (m_running) return;
    m_port = port;
    m_running = true;
    m_clientFd = -1;
    m_serverThread = std::thread(&DebugServer::ServerThread, this, port);
    LOG_INFO() << "[DebugServer] Started on port " << port;
}

void DebugServer::Stop() {
    if (!m_running) return;
    m_running = false;
    if (m_serverFd >= 0) {
        ::close(m_serverFd);
        m_serverFd = -1;
    }
    if (m_clientFd >= 0) {
        ::close(m_clientFd);
        m_clientFd = -1;
    }
    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }
    LOG_INFO() << "[DebugServer] Stopped";
}

void DebugServer::ServerThread(int port) {
    m_serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverFd < 0) {
        LOG_ERROR() << "[DebugServer] Failed to create socket";
        m_running = false;
        return;
    }

    int opt = 1;
    ::setsockopt(m_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(m_serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR() << "[DebugServer] Bind failed on port " << port;
        ::close(m_serverFd);
        m_serverFd = -1;
        m_running = false;
        return;
    }

    if (::listen(m_serverFd, 1) < 0) {
        LOG_ERROR() << "[DebugServer] Listen failed";
        ::close(m_serverFd);
        m_serverFd = -1;
        m_running = false;
        return;
    }

    while (m_running) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int fd = ::accept(m_serverFd, (struct sockaddr*)&clientAddr, &clientLen);
        if (fd < 0) {
            if (m_running) {
                LOG_ERROR() << "[DebugServer] Accept failed";
            }
            break;
        }

        LOG_INFO() << "[DebugServer] Client connected";
        m_clientFd = fd;

        // Send initial snapshot
        std::string snapshot = BuildSnapshotJson();
        SendToClient(snapshot);

        // Keep connection alive, send events as they come
        while (m_running && m_clientFd >= 0) {
            std::vector<std::string> events;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                events.swap(m_pendingEvents);
            }
            for (auto& evt : events) {
                SendToClient(evt);
            }

            // Re-send full snapshot periodically (every ~2 seconds)
            double t = Engine::Instance().GetElapsedTime();
            if (t - m_lastSnapshotTime > 2.0) {
                m_lastSnapshotTime = t;
                std::string snap = BuildSnapshotJson();
                // Prefix with ! to distinguish from events
                std::string msg = "!" + snap;
                SendToClient(msg);
            }

            ::usleep(50000); // 50ms poll interval
        }

        ::close(fd);
        m_clientFd = -1;
        LOG_INFO() << "[DebugServer] Client disconnected";
    }

    ::close(m_serverFd);
    m_serverFd = -1;
}

void DebugServer::SendToClient(const std::string& data) {
    std::lock_guard<std::mutex> lock(m_sendMutex);
    if (m_clientFd < 0) return;
    std::string line = data + "\n";
    ::send(m_clientFd, line.c_str(), line.size(), MSG_NOSIGNAL);
}
#else
void DebugServer::Start(int) {}
void DebugServer::Stop() {
    m_running = false;
    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }
}
void DebugServer::ServerThread(int) {}
void DebugServer::SendToClient(const std::string&) {}
#endif

void DebugServer::Update() {
    std::vector<std::string> events;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        events.swap(m_pendingEvents);
    }
    if (m_clientFd >= 0) {
        for (auto& evt : events) {
            SendToClient(evt);
        }
    }
}

void DebugServer::NotifyScriptInvoke(Entity* entity, const std::string& scriptPath) {
    if (!m_running || m_clientFd < 0) return;
    std::ostringstream json;
    json << "{";
    json << "\"type\":\"invoke\",";
    json << "\"entity_id\":" << (uintptr_t)entity << ",";
    json << "\"script\":\"" << EscapeJson(scriptPath) << "\",";
    json << "\"timestamp\":" << Engine::Instance().GetElapsedTime();
    json << "}";
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pendingEvents.push_back(json.str());
}

std::string DebugServer::EscapeJson(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c;
        }
    }
    return out;
}

std::string DebugServer::Vec3ToJson(const glm::vec3& v) {
    std::ostringstream ss;
    ss << "[" << v.x << "," << v.y << "," << v.z << "]";
    return ss.str();
}

std::string DebugServer::EntityToJson(Entity* e) {
    std::ostringstream json;
    json << "{";
    json << "\"id\":" << (uintptr_t)e << ",";
    json << "\"name\":\"" << EscapeJson(e->GetName()) << "\",";
    json << "\"active\":" << (e->IsActive() ? "true" : "false") << ",";
    json << "\"parent_id\":" << (e->GetParent() ? std::to_string((uintptr_t)e->GetParent()) : "null") << ",";
    json << "\"pos\":" << Vec3ToJson(e->GetPosition()) << ",";
    json << "\"scale\":" << Vec3ToJson(e->GetScale()) << ",";

    json << "\"components\":[";
    bool first = true;
    if (e->GetComponent<MeshComponent>()) {
        if (!first) json << ","; first = false;
        auto* mc = e->GetComponent<MeshComponent>();
        json << "{\"type\":\"MeshComponent\",\"color\":[" << mc->color.x << "," << mc->color.y << "," << mc->color.z << "," << mc->color.w << "],\"visible\":" << (mc->visible ? "true" : "false") << "}";
    }
    if (e->GetComponent<SpriteComponent>()) {
        if (!first) json << ","; first = false;
        auto* sc = e->GetComponent<SpriteComponent>();
        json << "{\"type\":\"SpriteComponent\",\"size\":[" << sc->size.x << "," << sc->size.y << "],\"visible\":" << (sc->visible ? "true" : "false") << "}";
    }
    if (e->GetComponent<RigidbodyComponent>()) {
        if (!first) json << ","; first = false;
        auto* rb = e->GetComponent<RigidbodyComponent>();
        json << "{\"type\":\"RigidbodyComponent\",\"mass\":" << rb->mass << ",\"useGravity\":" << (rb->useGravity ? "true" : "false") << "}";
    }
    if (e->GetComponent<ColliderComponent>()) {
        if (!first) json << ","; first = false;
        json << "{\"type\":\"ColliderComponent\"}";
    }
    if (e->GetComponent<ScriptComponent>()) {
        if (!first) json << ","; first = false;
        auto* sc = e->GetComponent<ScriptComponent>();
        json << "{\"type\":\"ScriptComponent\",\"script\":\"" << EscapeJson(sc->scriptPath) << "\"}";
    }
    if (e->GetComponent<LightComponent>()) {
        if (!first) json << ","; first = false;
        auto* lc = e->GetComponent<LightComponent>();
        json << "{\"type\":\"LightComponent\",\"color\":[" << lc->color.x << "," << lc->color.y << "," << lc->color.z << "],\"intensity\":" << lc->intensity << "}";
    }
    if (e->GetComponent<CameraComponent>()) {
        if (!first) json << ","; first = false;
        auto* cc = e->GetComponent<CameraComponent>();
        json << "{\"type\":\"CameraComponent\",\"fov\":" << cc->fov << "}";
    }
    json << "]";

    json << "}";
    return json.str();
}

std::string DebugServer::BuildSnapshotJson() {
    std::ostringstream json;
    json << "{";
    json << "\"type\":\"snapshot\",";
    json << "\"entities\":[";
    auto& entities = Scene::Instance().GetEntities();
    bool first = true;
    for (auto& e : entities) {
        if (!first) json << ",";
        first = false;
        json << EntityToJson(e.get());
    }
    json << "]}";
    return json.str();
}

} // namespace planet
