#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace planet {

inline bool g_termEnabled = false;

inline std::string Timestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream ss;
    ss << std::setfill('0')
       << std::setw(2) << tm.tm_hour << ":"
       << std::setw(2) << tm.tm_min << ":"
       << std::setw(2) << tm.tm_sec << "."
       << std::setw(3) << ms.count();
    return ss.str();
}

// Usage: LOG_INFO() << "[Engine] Initializing...";
// Usage: LOG_ERROR() << "[Engine] Failed: " << reason;

struct LogLine {
    std::ostringstream ss;
    bool error;
    LogLine(bool err) : error(err) {}
    ~LogLine() {
        if (g_termEnabled) {
            if (error) std::cerr << "[" << Timestamp() << "] ERROR " << ss.str() << std::endl;
            else       std::cout << "[" << Timestamp() << "] " << ss.str() << std::endl;
        }
    }
    template<typename T> LogLine& operator<<(const T& v) { ss << v; return *this; }
};

#define LOG_INFO()  planet::LogLine(false)
#define LOG_ERROR() planet::LogLine(true)

} // namespace planet
