#pragma once

#include <string>
#include <vector>
#include <mutex>

namespace planet {

struct LogEntry {
    std::string message;
    int level = 0;
    double time = 0.0;
};

class LogConsole {
public:
    static LogConsole& Instance();

    void AddLog(const std::string& message, int level = 0);
    void Clear();
    void Show(bool* open = nullptr);

    const std::vector<LogEntry>& GetEntries() const { return m_entries; }

private:
    LogConsole() = default;

    std::vector<LogEntry> m_entries;
    std::mutex m_mutex;
    bool m_autoScroll = true;
    bool m_scrollLocked = false;
    int m_filterLevel = 0;
    char m_filterText[256] = {};
    static constexpr size_t kMaxEntries = 5000;
};

void EditorLog(const std::string& msg, int level = 0);

} // namespace planet
