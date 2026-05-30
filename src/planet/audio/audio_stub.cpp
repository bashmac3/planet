#include "planet/audio/audio.h"
#include "planet/core/logger.h"

namespace planet {

Audio& Audio::Instance() {
    static Audio instance;
    return instance;
}

bool Audio::Init() {
    LOG_INFO() << "[Audio] Audio disabled (Windows stub)";
    return true;
}

void Audio::Shutdown() {}
void Audio::Update() {}

int Audio::PlaySound(const std::string&, float, bool) { return -1; }
void Audio::StopSound(int) {}
bool Audio::IsSoundPlaying(int) const { return false; }
void Audio::SetSoundVolume(int, float) {}
void Audio::SetSoundPitch(int, float) {}
void Audio::StopAllSounds() {}
void Audio::PauseSound(int) {}
void Audio::ResumeSound(int) {}
void Audio::PauseAllSounds() {}
void Audio::ResumeAllSounds() {}
float Audio::GetSoundDuration(int) const { return 0.0f; }
float Audio::GetSoundPosition(int) const { return 0.0f; }
void Audio::SetSoundPosition(int, float) {}
bool Audio::IsMusicPlaying() const { return false; }
void Audio::PlayMusic(const std::string&, float) {}
void Audio::StopMusic() {}
void Audio::SetMasterVolume(float volume) { m_masterVolume = volume; }

} // namespace planet
