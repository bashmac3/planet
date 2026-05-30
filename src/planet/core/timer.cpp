#include "planet/core/timer.h"
#include <algorithm>

namespace planet {

TimerManager& TimerManager::Instance() {
    static TimerManager instance;
    return instance;
}

int TimerManager::AddTimer(double interval, std::function<void()> callback, bool looping) {
    auto timer = std::make_unique<Timer>();
    timer->id = m_nextId++;
    timer->interval = interval;
    timer->callback = std::move(callback);
    timer->looping = looping;
    timer->oneshot = !looping;
    int id = timer->id;
    m_timers.push_back(std::move(timer));
    return id;
}

void TimerManager::RemoveTimer(int id) {
    auto it = std::find_if(m_timers.begin(), m_timers.end(),
        [id](const auto& t) { return t->id == id; });
    if (it != m_timers.end()) {
        m_timers.erase(it);
    }
}

void TimerManager::ClearAll() {
    m_timers.clear();
}

void TimerManager::Update(double dt) {
    for (auto& timer : m_timers) {
        if (!timer->active) continue;
        timer->elapsed += dt;
        if (timer->elapsed >= timer->interval) {
            if (timer->callback) {
                timer->callback();
            }
            if (timer->looping) {
                timer->elapsed -= timer->interval;
            } else {
                timer->active = false;
            }
        }
    }
    m_timers.erase(std::remove_if(m_timers.begin(), m_timers.end(),
        [](const auto& t) { return !t->active && t->oneshot; }), m_timers.end());
}

bool TimerManager::IsTimerActive(int id) const {
    for (const auto& t : m_timers) {
        if (t->id == id) return t->active;
    }
    return false;
}

int TimerManager::AddDelayedCall(double delay, std::function<void()> callback) {
    return AddTimer(delay, std::move(callback), false);
}

void TimerManager::PauseTimer(int id) {
    for (auto& t : m_timers) {
        if (t->id == id) { t->active = false; break; }
    }
}

void TimerManager::ResumeTimer(int id) {
    for (auto& t : m_timers) {
        if (t->id == id) { t->active = true; break; }
    }
}

void FrameTimer::Tick(double dt) {
    frameCount++;
    elapsed += dt;
    if (elapsed >= 0.5) {
        fps = frameCount / elapsed;
        minFps = std::min(minFps, fps);
        maxFps = std::max(maxFps, fps);
        frameCount = 0;
        elapsed = 0.0;
    }
}

void FrameTimer::Reset() {
    frameCount = 0;
    elapsed = 0.0;
    fps = 0.0;
    minFps = 1e6;
    maxFps = 0.0;
}

} // namespace planet
