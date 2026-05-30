#include "planet/audio/fmod_audio.h"
#include <fmod.h>
#include <fmod_errors.h>
#include <iostream>
#include "planet/core/logger.h"

namespace planet {

constexpr int FMOD_MAX_CHANNELS = 64;

FmodAudio& FmodAudio::Instance() {
    static FmodAudio instance;
    return instance;
}

bool FmodAudio::Init() {
    FMOD_RESULT result = FMOD::System_Create(&m_system);
    if (result != FMOD_OK) {
        LOG_ERROR() << "[FMOD] Failed to create system: " << FMOD_ErrorString(result);
        return false;
    }

    result = m_system->init(FMOD_MAX_CHANNELS, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK) {
        LOG_ERROR() << "[FMOD] Failed to initialize system: " << FMOD_ErrorString(result);
        m_system->release();
        m_system = nullptr;
        return false;
    }

    LOG_INFO() << "[FMOD] Audio system initialized (" << FMOD_MAX_CHANNELS << " channels).";
    return true;
}

void FmodAudio::Shutdown() {
    for (auto& [id, ch] : m_channels) {
        if (ch.channel) ch.channel->stop();
        if (ch.sound) ch.sound->release();
    }
    m_channels.clear();

    if (m_system) {
        m_system->close();
        m_system->release();
        m_system = nullptr;
    }

    LOG_INFO() << "[FMOD] Audio system shutdown.";
}

void FmodAudio::Update() {
    if (m_system) {
        m_system->update();
    }
}

static void checkFmodResult(FMOD_RESULT result, const char* context) {
    if (result != FMOD_OK) {
        LOG_ERROR() << "[FMOD] " << context << ": " << FMOD_ErrorString(result);
    }
}

int FmodAudio::PlaySound(const std::string& path, float volume, bool loop) {
    if (!m_system) return -1;

    FMOD::Sound* sound = nullptr;
    FMOD_MODE mode = FMOD_DEFAULT;
    if (loop) mode |= FMOD_LOOP_NORMAL;

    FMOD_RESULT result = m_system->createSound(path.c_str(), mode, nullptr, &sound);
    if (result != FMOD_OK) {
        LOG_ERROR() << "[FMOD] Failed to load sound '" << path << "': " << FMOD_ErrorString(result);
        return -1;
    }

    FMOD::Channel* channel = nullptr;
    result = m_system->playSound(sound, nullptr, false, &channel);
    if (result != FMOD_OK) {
        LOG_ERROR() << "[FMOD] Failed to play sound '" << path << "': " << FMOD_ErrorString(result);
        sound->release();
        return -1;
    }

    channel->setVolume(volume * m_masterVolume);

    int id = m_nextId++;
    m_channels[id] = {channel, sound, false};
    return id;
}

void FmodAudio::StopSound(int id) {
    auto it = m_channels.find(id);
    if (it != m_channels.end()) {
        if (it->second.channel) it->second.channel->stop();
        if (it->second.sound) it->second.sound->release();
        m_channels.erase(it);
    }
}

void FmodAudio::PlayMusic(const std::string& path, float volume) {
    StopMusic();
    m_musicId = PlaySound(path, volume, true);
    if (m_musicId >= 0) {
        m_channels[m_musicId].isMusic = true;
    }
}

void FmodAudio::StopMusic() {
    if (m_musicId >= 0) {
        StopSound(m_musicId);
        m_musicId = -1;
    }
}

void FmodAudio::SetMasterVolume(float volume) {
    m_masterVolume = volume;
    if (m_system) {
        FMOD::ChannelGroup* masterGroup = nullptr;
        m_system->getMasterChannelGroup(&masterGroup);
        if (masterGroup) {
            masterGroup->setVolume(volume);
        }
    }
}

bool FmodAudio::IsSoundPlaying(int id) const {
    auto it = m_channels.find(id);
    if (it == m_channels.end()) return false;
    bool playing = false;
    if (it->second.channel) it->second.channel->isPlaying(&playing);
    return playing;
}

void FmodAudio::SetSoundVolume(int id, float volume) {
    auto it = m_channels.find(id);
    if (it != m_channels.end() && it->second.channel) {
        it->second.channel->setVolume(volume * m_masterVolume);
    }
}

void FmodAudio::SetSoundPitch(int id, float pitch) {
    auto it = m_channels.find(id);
    if (it != m_channels.end() && it->second.channel) {
        it->second.channel->setPitch(pitch);
    }
}

void FmodAudio::StopAllSounds() {
    for (auto& [id, ch] : m_channels) {
        if (ch.channel) ch.channel->stop();
        if (ch.sound) ch.sound->release();
    }
    m_channels.clear();
    m_musicId = -1;
}

void FmodAudio::PauseSound(int id) {
    auto it = m_channels.find(id);
    if (it != m_channels.end() && it->second.channel) {
        it->second.channel->setPaused(true);
    }
}

void FmodAudio::ResumeSound(int id) {
    auto it = m_channels.find(id);
    if (it != m_channels.end() && it->second.channel) {
        it->second.channel->setPaused(false);
    }
}

void FmodAudio::PauseAllSounds() {
    for (auto& [id, ch] : m_channels) {
        if (ch.channel) ch.channel->setPaused(true);
    }
}

void FmodAudio::ResumeAllSounds() {
    for (auto& [id, ch] : m_channels) {
        if (ch.channel) ch.channel->setPaused(false);
    }
}

float FmodAudio::GetSoundDuration(int id) const {
    auto it = m_channels.find(id);
    if (it == m_channels.end() || !it->second.sound) return 0.0f;
    unsigned int length;
    it->second.sound->getLength(&length, FMOD_TIMEUNIT_MS);
    return static_cast<float>(length) / 1000.0f;
}

float FmodAudio::GetSoundPosition(int id) const {
    auto it = m_channels.find(id);
    if (it == m_channels.end() || !it->second.channel) return 0.0f;
    unsigned int pos;
    it->second.channel->getPosition(&pos, FMOD_TIMEUNIT_MS);
    return static_cast<float>(pos) / 1000.0f;
}

void FmodAudio::SetSoundPosition(int id, float position) {
    auto it = m_channels.find(id);
    if (it != m_channels.end() && it->second.channel) {
        unsigned int ms = static_cast<unsigned int>(position * 1000.0f);
        it->second.channel->setPosition(ms, FMOD_TIMEUNIT_MS);
    }
}

bool FmodAudio::IsMusicPlaying() const {
    if (m_musicId < 0) return false;
    return IsSoundPlaying(m_musicId);
}

} // namespace planet
