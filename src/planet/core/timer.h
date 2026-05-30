#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>

namespace planet {

struct Timer {
    double interval = 1.0;
    double elapsed = 0.0;
    bool looping = true;
    bool active = true;
    std::function<void()> callback;
    int id = 0;
    bool oneshot = false;

    void Reset() { elapsed = 0.0; active = true; }
    void Stop() { active = false; }
    void Start() { active = true; elapsed = 0.0; }
};

class TimerManager {
public:
    static TimerManager& Instance();

    int AddTimer(double interval, std::function<void()> callback, bool looping = true);
    void RemoveTimer(int id);
    void ClearAll();
    void Update(double dt);
    bool IsTimerActive(int id) const;

    int AddDelayedCall(double delay, std::function<void()> callback);
    void PauseTimer(int id);
    void ResumeTimer(int id);

private:
    TimerManager() = default;
    std::vector<std::unique_ptr<Timer>> m_timers;
    int m_nextId = 1;
};

struct FrameTimer {
    int frameCount = 0;
    double elapsed = 0.0;
    double fps = 0.0;
    double minFps = 1e6;
    double maxFps = 0.0;

    void Tick(double dt);
    void Reset();
};

} // namespace planet
