#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <deque>

namespace planet {

enum class LogLevel {
    Info,
    Warning,
    Error,
    Debug
};

struct LogEntry {
    std::string text;
    LogLevel level;
};

class Entity;

struct ConsoleCommand {
    std::string name;
    std::string prefix;   // "do", "sr", "cr"
    std::string args;     // argument description
    std::string desc;
    std::function<void(const std::string& args)> handler;
};

class Console {
public:
    static Console& Instance();

    void Init();
    void Shutdown();
    void Update(double dt);
    void Render();

    void Toggle();
    bool IsOpen() const { return m_open; }
    void SetOpen(bool open);
    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled) { m_enabled = enabled; }

    void AddChar(unsigned int codepoint);
    void AddCommand(const ConsoleCommand& cmd);
    void Execute(const std::string& line);
    void Print(const std::string& msg);
    void Print(LogLevel level, const std::string& msg);
    void PrintError(const std::string& msg);
    void PrintWarning(const std::string& msg);
    void PrintDebug(const std::string& msg);

    Entity* RaycastEntity() const;

    bool CheatsEnabled() const { return m_cheatsEnabled; }
    void SetCheatsEnabled(bool enabled) { m_cheatsEnabled = enabled; }

    bool showLightVector = false;
    bool showInstanceInfo = false;
    bool noclipEnabled = false;

private:
    Console() = default;

    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;

    void RegisterBuiltinCommands();
    void EraseChar();
    void MoveCursor(int delta);
    void HistoryUp();
    void HistoryDown();

    bool m_open = false;
    std::string m_input;
    int m_cursor = 0;
    std::vector<std::string> m_history;
    int m_historyIndex = -1;
    std::deque<LogEntry> m_log;  // last N log lines
    bool m_cheatsEnabled = false;
    bool m_enabled = false;

    std::unordered_map<std::string, ConsoleCommand> m_commands;
};

} // namespace planet
