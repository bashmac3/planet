#include "planet/editor/log_console.h"
#include <ctime>
#include <cstdio>

namespace planet {

static double s_startTime = 0.0;

LogConsole& LogConsole::Instance() {
    static LogConsole instance;
    return instance;
}

void LogConsole::AddLog(const std::string& message, int level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    LogEntry entry;
    entry.message = message;
    entry.level = level;
    entry.time = s_startTime;
    m_entries.push_back(entry);
    if (m_entries.size() > kMaxEntries)
        m_entries.erase(m_entries.begin());
}

void LogConsole::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries.clear();
}

void EditorLog(const std::string& msg, int level) {
    LogConsole::Instance().AddLog(msg, level);
}

} // namespace planet
