#include "planet/core/console.h"
#include "planet/core/engine.h"
#include "planet/core/scene.h"
#include "planet/core/camera.h"
#include "planet/core/window.h"
#include "planet/core/logger.h"
#include "planet/core/input.h"
#include "planet/render/renderer.h"
#include "planet/render/sprite_renderer.h"
#include "planet/render/text_renderer.h"
#include "planet/render/mesh.h"
#include "planet/render/model_renderer.h"
#include "planet/ecs/ecs.h"
#include "planet/ecs/component.h"
#include "planet/core/dexecl_engine.h"
#include "planet/physics/physics.h"
#include "planet/debug/debug_server.h"
#include "lua/lua_engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <chrono>

namespace planet {

Console& Console::Instance() {
    static Console instance;
    return instance;
}

void Console::Init() {
    RegisterBuiltinCommands();
    LOG_INFO() << "[Console] Initialized.";
}

void Console::Shutdown() {
    m_commands.clear();
    m_log.clear();
}

void Console::Toggle() {
    if (!m_enabled) return;
    m_open = !m_open;
    if (m_open) {
        Input::Instance().SetMouseLocked(false);
    } else {
        Input::Instance().SetMouseLocked(true);
    }
}

void Console::SetOpen(bool open) {
    m_open = open;
    if (!m_open) {
        Input::Instance().SetMouseLocked(true);
    }
}

void Console::AddChar(unsigned int codepoint) {
    if (!m_open) return;
    if (codepoint < 32) return;  // skip control chars except handled elsewhere
    char c = static_cast<char>(codepoint);
    m_input.insert(m_input.begin() + m_cursor, c);
    m_cursor++;
}

void Console::EraseChar() {
    if (m_cursor > 0) {
        m_input.erase(m_input.begin() + m_cursor - 1);
        m_cursor--;
    }
}

void Console::MoveCursor(int delta) {
    m_cursor = std::max(0, std::min(static_cast<int>(m_input.size()), m_cursor + delta));
}

void Console::HistoryUp() {
    if (m_history.empty()) return;
    if (m_historyIndex == -1) {
        m_historyIndex = static_cast<int>(m_history.size()) - 1;
    } else if (m_historyIndex > 0) {
        m_historyIndex--;
    }
    m_input = m_history[m_historyIndex];
    m_cursor = static_cast<int>(m_input.size());
}

void Console::HistoryDown() {
    if (m_historyIndex == -1) return;
    if (m_historyIndex < static_cast<int>(m_history.size()) - 1) {
        m_historyIndex++;
        m_input = m_history[m_historyIndex];
    } else {
        m_historyIndex = -1;
        m_input.clear();
    }
    m_cursor = static_cast<int>(m_input.size());
}

void Console::Update(double dt) {
    if (!m_open || !m_enabled) return;

    auto& input = Input::Instance();

    if (input.GetKeyDown(KeyCode::Escape)) {
        SetOpen(false);
        return;
    }

    if (input.GetKeyDown(KeyCode::Enter)) {
        if (!m_input.empty()) {
            Execute(m_input);
            m_history.push_back(m_input);
            if (m_history.size() > 100) m_history.erase(m_history.begin());
            m_historyIndex = -1;
            m_input.clear();
            m_cursor = 0;
        }
    }

    if (input.GetKeyDown(KeyCode::Backspace)) {
        EraseChar();
    }

    if (input.GetKeyDown(KeyCode::Delete)) {
        if (m_cursor < static_cast<int>(m_input.size())) {
            m_input.erase(m_input.begin() + m_cursor);
        }
    }

    if (input.GetKeyDown(KeyCode::Left)) MoveCursor(-1);
    if (input.GetKeyDown(KeyCode::Right)) MoveCursor(1);
    if (input.GetKeyDown(KeyCode::Home)) { m_cursor = 0; }
    if (input.GetKeyDown(KeyCode::End)) { m_cursor = static_cast<int>(m_input.size()); }

    if (input.GetKeyDown(KeyCode::Up)) HistoryUp();
    if (input.GetKeyDown(KeyCode::Down)) HistoryDown();
}

void Console::Render() {
    if (!m_open || !m_enabled) return;

    auto& sr = SpriteRenderer::Instance();
    auto& tr = TextRenderer::Instance();
    int w = Window::Instance().GetWidth();
    int h = Window::Instance().GetHeight();
    float panelH = 200.0f;

    // Background panel (white theme)
    sr.DrawSprite(glm::vec2(0, h - panelH), glm::vec2(static_cast<float>(w), panelH), glm::vec4(0.93f, 0.93f, 0.93f, 0.85f));
    sr.DrawSprite(glm::vec2(0, static_cast<float>(h - panelH)), glm::vec2(static_cast<float>(w), 2), glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));

    // Log output area
    float logStartY = h - panelH + 6;
    int visibleLines = std::min(8, static_cast<int>(m_log.size()));
    for (int i = 0; i < visibleLines; i++) {
        int idx = static_cast<int>(m_log.size()) - visibleLines + i;
        std::string line = m_log[idx].text;
        if (line.size() > 100) line = line.substr(0, 97) + "...";

        glm::vec4 color;
        switch (m_log[idx].level) {
            case LogLevel::Error:   color = glm::vec4(0.75f, 0.0f, 0.0f, 0.9f); break;
            case LogLevel::Warning: color = glm::vec4(0.75f, 0.55f, 0.0f, 0.9f); break;
            case LogLevel::Debug:   color = glm::vec4(0.85f, 0.45f, 0.0f, 0.9f); break;
            default:                color = glm::vec4(0.15f, 0.15f, 0.15f, 0.9f); break;
        }
        tr.DrawString(line, 8, logStartY + i * 18, 0.7f, color);
    }

    // Input line
    std::ostringstream ps; ps << "] " << m_input; std::string prompt = ps.str();
    float inputY = h - 24;
    tr.DrawString(prompt, 8, inputY, 0.9f, glm::vec4(0.15f, 0.15f, 0.15f, 1.0f));

    // Blinking cursor
    double elapsed = Engine::Instance().GetElapsedTime();
    if (static_cast<int>(elapsed * 2) % 2 == 0) {
        std::string pre = "] " + m_input.substr(0, m_cursor);
        float cx = 8 + tr.MeasureStringWidth(pre, 0.9f);
        tr.DrawString("_", cx, inputY, 0.9f, glm::vec4(0.15f, 0.15f, 0.15f, 1.0f));
    }

    // Help hint
    tr.DrawString("` ~ toggle | ESC close | Enter execute",
                  10, h - 40, 0.7f, glm::vec4(0.5f, 0.5f, 0.5f, 0.6f));
}

void Console::Print(const std::string& msg) {
    Print(LogLevel::Info, msg);
}

void Console::PrintError(const std::string& msg) {
    Print(LogLevel::Error, msg);
}

void Console::PrintWarning(const std::string& msg) {
    Print(LogLevel::Warning, msg);
}

void Console::PrintDebug(const std::string& msg) {
    Print(LogLevel::Debug, msg);
}

void Console::Print(LogLevel level, const std::string& msg) {
    std::string label;
    switch (level) {
        case LogLevel::Error:   label = "ERR"; break;
        case LogLevel::Warning: label = "WRN"; break;
        case LogLevel::Debug:   label = "DBG"; break;
        default:                label = "INF"; break;
    }

    auto formatted = "[" + label + "] " + msg;
    m_log.push_back({formatted, level});
    if (m_log.size() > 100) m_log.pop_front();
    if (g_termEnabled) std::cout << msg << std::endl;
}

void Console::Execute(const std::string& line) {
    Print("] " + line);

    // Parse command name and args
    std::string cmdName = line;
    std::string args;
    size_t spacePos = line.find(' ');
    if (spacePos != std::string::npos) {
        cmdName = line.substr(0, spacePos);
        args = line.substr(spacePos + 1);
    }

    // Try registered command first
    auto it = m_commands.find(cmdName);
    if (it != m_commands.end()) {
        it->second.handler(args);
        return;
    }

    // Try as Lua code
    auto& lua = LuaRuntime::Instance();
    lua_State* L = lua.GetState();
    if (!L) return;

    std::string wrapped = "return " + line;
    if (luaL_loadstring(L, wrapped.c_str()) == 0) {
        if (lua_pcall(L, 0, 1, 0) == 0) {
                if (!lua_isnil(L, -1)) {
                    const char* result = lua_tostring(L, -1);
                    Print(result ? result : "(non-string result)");
                }
        } else {
            // Try without wrap
            lua_pop(L, 1);
            if (luaL_loadstring(L, line.c_str()) == 0) {
                if (lua_pcall(L, 0, 1, 0) == 0 && !lua_isnil(L, -1)) {
                    const char* result = lua_tostring(L, -1);
                    Print(result ? result : "(non-string result)");
                }
            } else {
                Print("Error: " + std::string(lua_tostring(L, -1)));
            }
        }
    } else {
        Print("Unknown command: " + cmdName);
    }
    lua_settop(L, 0);
}

Entity* Console::RaycastEntity() const {
    auto* cam = Scene::Instance().GetActiveCamera();
    if (!cam) return nullptr;

    // Ray from camera position forward
    glm::vec3 origin = cam->GetPosition();
    glm::vec3 dir = glm::normalize(cam->GetTarget() - cam->GetPosition());

    Entity* closest = nullptr;
    float closestDist = 1000.0f;

    for (auto& entity : Scene::Instance().GetEntities()) {
        auto* meshComp = entity->GetComponent<MeshComponent>();
        if (!meshComp || !meshComp->visible) continue;

        // Simple sphere intersection using entity scale as radius
        auto pos = entity->GetPosition();
        auto scale = entity->GetScale();
        float radius = std::max({scale.x, scale.y, scale.z}) * 0.7f;

        glm::vec3 oc = origin - pos;
        float a = glm::dot(dir, dir);
        float b = 2.0f * glm::dot(oc, dir);
        float c = glm::dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;

        if (discriminant >= 0) {
            float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
            if (t > 0 && t < closestDist) {
                closestDist = t;
                closest = entity.get();
            }
        }
    }

    return closest;
}

void Console::RegisterBuiltinCommands() {
    // do_show_light_vector
    AddCommand({"show_light_vector", "do", "1/0/true/false",
        "Toggle light direction debug overlay",
        [this](const std::string& args) {
            if (args == "1" || args == "true" || args == "enable") {
                if (!m_cheatsEnabled) { Print("Cheats must be enabled (sr_cheats 1)"); return; };
                showLightVector = true;
                Print("Light vector overlay: ON");
            } else if (args == "0" || args == "false" || args == "disable") {
                showLightVector = false;
                Print("Light vector overlay: OFF");
            } else {
                showLightVector = !showLightVector;
                if (showLightVector && !m_cheatsEnabled) {
                    showLightVector = false;
                    Print("Cheats must be enabled (sr_cheats 1)");
                } else {
                    Print(std::string("Light vector overlay: ") + (showLightVector ? "ON" : "OFF"));
                }
            }
        }});

    // do_instance_info
    AddCommand({"instance_info", "do", "1/0/true/false",
        "Show info about looked-at entity",
        [this](const std::string& args) {
            if (args == "1" || args == "true" || args == "enable") {
                if (!m_cheatsEnabled) { Print("Cheats must be enabled (sr_cheats 1)"); return; };
                showInstanceInfo = true;
                Print("Instance info overlay: ON");
            } else if (args == "0" || args == "false" || args == "disable") {
                showInstanceInfo = false;
                Print("Instance info overlay: OFF");
            } else {
                showInstanceInfo = !showInstanceInfo;
                if (showInstanceInfo && !m_cheatsEnabled) {
                    showInstanceInfo = false;
                    Print("Cheats must be enabled (sr_cheats 1)");
                } else {
                    Print(std::string("Instance info overlay: ") + (showInstanceInfo ? "ON" : "OFF"));
                }
            }
        }});

    // do_clear_all
    AddCommand({"clear_all", "do", "",
        "Destroy all scene entities",
        [this](const std::string&) {
            if (!m_cheatsEnabled) { Print("Cheats must be enabled (sr_cheats 1)"); return; };
            auto& scene = Scene::Instance();
            int count = 0;
            while (!scene.GetEntities().empty()) {
                scene.DestroyEntity(scene.GetEntities().back().get());
                count++;
            }
            Print("Cleared " + std::to_string(count) + " entities");
        }});

    // do_noclip
    AddCommand({"noclip", "do", "",
        "Toggle noclip mode (no collision)",
        [this](const std::string&) {
            if (!m_cheatsEnabled) { Print("Cheats must be enabled (sr_cheats 1)"); return; };
            noclipEnabled = !noclipEnabled;
            Print(std::string("Noclip: ") + (noclipEnabled ? "ON" : "OFF"));
        }});

    // sr_cheats
    AddCommand({"cheats", "sr", "1/0/true/false",
        "Enable/disable cheat commands and debug overlays",
        [this](const std::string& args) {
            if (args == "1" || args == "true" || args == "enable") {
                m_cheatsEnabled = true;
                Print("Cheats: ENABLED");
            } else if (args == "0" || args == "false" || args == "disable") {
                m_cheatsEnabled = false;
                showLightVector = false;
                showInstanceInfo = false;
                Print("Cheats: DISABLED (overlays hidden)");
            } else {
                Print("Usage: sr_cheats 1/0/true/false");
            }
        }});

    // cr_ commands - client replicated
    AddCommand({"fov", "cr", "<degrees>",
        "Set camera FOV",
        [this](const std::string& args) {
            float fov = std::stof(args.empty() ? "75" : args);
            auto* cam = Scene::Instance().GetActiveCamera();
            if (cam) { cam->SetFOV(fov); Print("FOV set to " + std::to_string(fov)); };
        }});

    AddCommand({"speed", "cr", "<speed>",
        "Set player movement speed",
        [this](const std::string& args) {
            float val = args.empty() ? 6.0f : std::stof(args);
            DexeclEngine::Instance().SetVariable("player_speed", val);
            Print("Player speed: " + std::to_string(val));
        }});

    AddCommand({"sensitivity", "cr", "<value>",
        "Set mouse sensitivity",
        [this](const std::string& args) {
            float val = args.empty() ? 0.002f : std::stof(args);
            DexeclEngine::Instance().SetVariable("player_sensitivity", val);
            Print("Mouse sensitivity: " + std::to_string(val));
        }});

    // sr_debug_graph - start/stop debug grapher server
    AddCommand({"debug_graph", "sr", "[port]",
        "Start/stop the debug grapher TCP server (default port 9876)",
        [this](const std::string& args) {
            if (DebugServer::Instance().IsRunning()) {
                DebugServer::Instance().Stop();
                Print("Debug grapher server stopped");
            } else {
                int port = 9876;
                if (!args.empty()) port = std::stoi(args);
                DebugServer::Instance().Start(port);
                Print("Debug grapher server started on port " + std::to_string(port));
            }
        }});

    // sr_ commands - server replicated
    AddCommand({"time", "sr", "<hour>",
        "Set time of day (0-24)",
        [this](const std::string& args) {
            Print("sr_time: " + (args.empty() ? std::string("current") : args) + " (time system not yet implemented)");
        }});

    AddCommand({"gravity", "sr", "<value>",
        "Set gravity strength",
        [this](const std::string& args) {
            if (!m_cheatsEnabled) { Print("Cheats must be enabled"); return; };
            float g = args.empty() ? -20.0f : std::stof(args);
            Physics::Instance().SetGravity(glm::vec3(0, g, 0));
            Print("Gravity set to " + std::to_string(g));
        }});

    // Help (man-style)
    AddCommand({"help", "do", "<command>",
        "Show detailed help for a console command",
        [this](const std::string& args) {
            std::string query = args;
            query.erase(0, query.find_first_not_of(" \t"));
            query.erase(query.find_last_not_of(" \t") + 1);

            if (query.empty()) {
                Print("Usage: help <command>");
                Print("Type 'list' for all available commands.");
                return;
            }

            auto it = m_commands.find(query);
            if (it == m_commands.end()) {
                for (auto& [name, cmd] : m_commands) {
                    if (name == query || cmd.name == query ||
                        (cmd.prefix + "_" + cmd.name) == query) {
                        it = m_commands.find(name);
                        break;
                    }
                }
            }
            if (it != m_commands.end()) {
                auto& cmd = it->second;
                Print("");
                Print("  NAME");
                Print("    " + cmd.prefix + "_" + cmd.name +
                      (cmd.args.empty() ? "" : (" " + cmd.args)));
                Print("");
                Print("  DESCRIPTION");
                Print("    " + cmd.desc);
                Print("");
                Print("  PREFIX");
                Print("    " + cmd.prefix + "  (shortcut: " + cmd.name + ")");
            } else {
                Print("No help entry for: " + query);
            }
        }});

    // quit
    AddCommand({"quit", "cr", "",
        "Exit the engine",
        [this](const std::string&) {
            Engine::Instance().Quit();
        }});

    // clear
    AddCommand({"clear", "cr", "",
        "Clear console log",
        [this](const std::string&) {
            m_log.clear();
        }});

    // ---- DEXECL commands ----

    AddCommand({"dexec", "cr", "<filename>",
        "Execute a .dexl script file (client-side)",
        [this](const std::string& args) {
            std::string path = args.empty() ? "dexecl/start.dexl" : args;
            if (path.find('/') == std::string::npos) path = "dexecl/" + path;
            DexeclEngine::Instance().ExecuteFile(path);
        }});

    AddCommand({"dexec", "sr", "<filename>",
        "Execute a .dexl script file (server-side)",
        [this](const std::string& args) {
            std::string path = args.empty() ? "dexecl/start.dexl" : args;
            if (path.find('/') == std::string::npos) path = "dexecl/" + path;
            DexeclEngine::Instance().ExecuteFile(path);
        }});

    // ---- Entity commands ----

    AddCommand({"entity_list", "cr", "",
        "List all entities in the scene",
        [this](const std::string&) {
            auto& entities = Scene::Instance().GetEntities();
            if (entities.empty()) {
                Print("No entities in scene");
                return;
            }
            Print("=== Entities (" + std::to_string(entities.size()) + ") ===");
            for (auto& e : entities) {
                auto pos = e->GetPosition();
                std::string line = "  " + e->GetName() + " pos(" + std::to_string(static_cast<int>(pos.x)) + "," + std::to_string(static_cast<int>(pos.y)) + "," + std::to_string(static_cast<int>(pos.z)) + ")";
                if (!e->IsActive()) line += " [inactive]";
                Print(line);
            }
        }});

    AddCommand({"entity_info", "cr", "<name>",
        "Show detailed info about an entity",
        [this](const std::string& args) {
            if (args.empty()) { Print("Usage: entity_info <name>"); return; };
            auto& scene = Scene::Instance();
            Entity* ent = scene.FindEntity(args);
            if (!ent) {
                Print("Entity not found: " + args);
                return;
            }
            auto pos = ent->GetPosition();
            auto scl = ent->GetScale();
            Print("Entity: " + ent->GetName());
                        Print("  Active: " + std::string(ent->IsActive() ? "yes" : "no"));
            Print("  Pos: " + std::to_string(pos.x) + " " + std::to_string(pos.y) + " " + std::to_string(pos.z));
            Print("  Scale: " + std::to_string(scl.x) + " " + std::to_string(scl.y) + " " + std::to_string(scl.z));
            auto tags = ent->GetTags();
            if (!tags.empty()) {
                std::string tagStr;
                for (auto& t : tags) tagStr += t + " ";
                Print("  Tags: " + tagStr);
            }
        }});

    AddCommand({"entity_destroy", "do", "<name>",
        "Destroy a named entity (requires cheats)",
        [this](const std::string& args) {
            if (!m_cheatsEnabled) { Print("Cheats must be enabled (sr_cheats 1)"); return; };
            if (args.empty()) { Print("Usage: entity_destroy <name>"); return; };
            auto& scene = Scene::Instance();
            Entity* ent = scene.FindEntity(args);
            if (!ent) { Print("Entity not found: " + args); return; };
            scene.DestroyEntity(ent);
            Print("Destroyed entity: " + args);
        }});

    // ---- Variable commands ----

    AddCommand({"var", "do", "<name> [<value>]",
        "Get or set a DEXECL variable",
        [this](const std::string& args) {
            std::istringstream iss(args);
            std::string name, value;
            iss >> name;
            std::getline(iss, value);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            if (value.empty()) {
                if (DexeclEngine::Instance().HasVariable(name)) {
                                        Print(name + " = " + std::to_string(DexeclEngine::Instance().GetVariable(name)));
                } else {
                    Print("Variable not set: " + name);
                }
            } else {
                DexeclEngine::Instance().SetVariable(name, static_cast<float>(std::atof(value.c_str())));
                                Print(name + " = " + std::to_string(DexeclEngine::Instance().GetVariable(name)));
            }
        }});

    AddCommand({"vars", "do", "",
        "List all DEXECL variables",
        [this](const std::string&) {
            auto& vars = DexeclEngine::Instance().GetVariables();
            if (vars.empty()) {
                Print("No variables set");
                return;
            }
            Print("=== DEXECL Variables ===");
            for (auto& [name, val] : vars) {
                Print("  " + name + " = " + std::to_string(val));
            }
        }});

    // ---- Input / Control commands ----

    AddCommand({"bind", "cr", "<key> <command>",
        "Bind a key to a console command (coming soon)",
        [this](const std::string& args) {
            Print("bind: " + args + " (key binding system not yet implemented)");
        }});

    AddCommand({"unbind", "cr", "<key>",
        "Unbind a key",
        [this](const std::string& args) {
            Print("unbind: " + args + " (key binding system not yet implemented)");
        }});

    // ---- Physics tweaks ----

    AddCommand({"gravity", "cr", "<value>",
        "Set gravity strength (client-side)",
        [this](const std::string& args) {
            float g = args.empty() ? -28.0f : std::stof(args);
            Physics::Instance().SetGravity(glm::vec3(0, g, 0));
            DexeclEngine::Instance().SetVariable("gravity", g);
            Print("Gravity set to " + std::to_string(g));
        }});

    // ---- Player commands ----

    AddCommand({"player_speed", "cr", "<value>",
        "Set player movement speed via DEXECL variable",
        [this](const std::string& args) {
            if (!args.empty()) {
                DexeclEngine::Instance().SetVariable("speed", std::stof(args));
            }
                        Print("Player speed: " + std::to_string(DexeclEngine::Instance().GetVariable("speed", 8.0f)));
        }});

    AddCommand({"echo", "cr", "<message>",
        "Print a message to the console",
        [this](const std::string& args) {
            Print(args);
        }});

    // ---- System ----

    AddCommand({"exec", "cr", "<filename>",
        "Execute a .dexl script (alias for cr_dexec)",
        [this](const std::string& args) {
            std::string path = args;
            if (path.find('/') == std::string::npos) path = "dexecl/" + path;
            if (path.find(".dexl") == std::string::npos) path += ".dexl";
            DexeclEngine::Instance().ExecuteFile(path);
        }});

}

void Console::AddCommand(const ConsoleCommand& cmd) {
    m_commands[cmd.prefix + "_" + cmd.name] = cmd;
    // Also allow shortcut without prefix
    m_commands[cmd.name] = cmd;
}

} // namespace planet
