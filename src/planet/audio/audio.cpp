#include "planet/audio/audio.h"
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include "planet/core/logger.h"

namespace planet {

#pragma pack(push, 1)
struct WAVHeader {
    char riff[4] = {'R','I','F','F'};
    uint32_t chunkSize;
    char wave[4] = {'W','A','V','E'};
    char fmt[4] = {'f','m','t',' '};
    uint32_t subchunk1Size = 16;
    uint16_t audioFormat = 1;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4] = {'d','a','t','a'};
    uint32_t dataSize;
};
#pragma pack(pop)

Audio& Audio::Instance() {
    static Audio instance;
    return instance;
}

bool Audio::Init() {
    m_device = alcOpenDevice(nullptr);
    if (!m_device) {
        LOG_ERROR() << "[Audio] Failed to open audio device!";
        return false;
    }

    m_context = alcCreateContext(m_device, nullptr);
    if (!m_context) {
        LOG_ERROR() << "[Audio] Failed to create audio context!";
        alcCloseDevice(m_device);
        return false;
    }

    alcMakeContextCurrent(m_context);

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    LOG_INFO() << "[Audio] Audio system initialized.";
    return true;
}

void Audio::Update() {
}

void Audio::Shutdown() {
    for (auto& [id, src] : m_sources) {
        alDeleteSources(1, &src.source);
        alDeleteBuffers(1, &src.buffer);
    }
    m_sources.clear();

    alcMakeContextCurrent(nullptr);
    if (m_context) alcDestroyContext(m_context);
    if (m_device) alcCloseDevice(m_device);

    LOG_INFO() << "[Audio] Audio system shutdown.";
}

ALuint Audio::LoadWAV(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR() << "[Audio] Failed to open: " << path;
        return 0;
    }

    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    if (std::strncmp(header.riff, "RIFF", 4) ||
        std::strncmp(header.wave, "WAVE", 4)) {
        LOG_ERROR() << "[Audio] Not a valid WAV file: " << path;
        return 0;
    }

    std::vector<char> data(header.dataSize);
    file.read(data.data(), header.dataSize);

    ALuint buffer;
    alGenBuffers(1, &buffer);

    ALenum format;
    if (header.numChannels == 1) {
        format = (header.bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    } else {
        format = (header.bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    }

    alBufferData(buffer, format, data.data(),
                 static_cast<ALsizei>(data.size()),
                 static_cast<ALsizei>(header.sampleRate));

    return buffer;
}

int Audio::PlaySound(const std::string& path, float volume, bool loop) {
    ALuint buffer = LoadWAV(path);
    if (!buffer) return -1;

    ALuint source;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, static_cast<ALint>(buffer));
    alSourcef(source, AL_GAIN, volume * m_masterVolume);
    alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcePlay(source);

    int id = m_nextSourceId++;
    m_sources[id] = {source, buffer, false};
    return id;
}

void Audio::StopSound(int id) {
    auto it = m_sources.find(id);
    if (it != m_sources.end()) {
        alSourceStop(it->second.source);
        DeleteSource(id);
    }
}

void Audio::PlayMusic(const std::string& path, float volume) {
    StopMusic();
    m_musicId = PlaySound(path, volume, true);
}

void Audio::StopMusic() {
    if (m_musicId >= 0) {
        StopSound(m_musicId);
        m_musicId = -1;
    }
}

void Audio::SetMasterVolume(float volume) {
    m_masterVolume = volume;
    alListenerf(AL_GAIN, volume);
}

void Audio::DeleteSource(int id) {
    auto it = m_sources.find(id);
    if (it != m_sources.end()) {
        alDeleteSources(1, &it->second.source);
        alDeleteBuffers(1, &it->second.buffer);
        m_sources.erase(it);
    }
}

bool Audio::IsSoundPlaying(int id) const {
    auto it = m_sources.find(id);
    if (it == m_sources.end()) return false;
    ALint state;
    alGetSourcei(it->second.source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

void Audio::SetSoundVolume(int id, float volume) {
    auto it = m_sources.find(id);
    if (it != m_sources.end()) {
        alSourcef(it->second.source, AL_GAIN, volume * m_masterVolume);
    }
}

void Audio::SetSoundPitch(int id, float pitch) {
    auto it = m_sources.find(id);
    if (it != m_sources.end()) {
        alSourcef(it->second.source, AL_PITCH, pitch);
    }
}

void Audio::StopAllSounds() {
    for (auto& [id, src] : m_sources) {
        alSourceStop(src.source);
    }
    m_sources.clear();
    m_musicId = -1;
}

void Audio::PauseSound(int id) {
    auto it = m_sources.find(id);
    if (it != m_sources.end()) {
        alSourcePause(it->second.source);
    }
}

void Audio::ResumeSound(int id) {
    auto it = m_sources.find(id);
    if (it != m_sources.end()) {
        alSourcePlay(it->second.source);
    }
}

void Audio::PauseAllSounds() {
    for (auto& [id, src] : m_sources) {
        alSourcePause(src.source);
    }
}

void Audio::ResumeAllSounds() {
    for (auto& [id, src] : m_sources) {
        alSourcePlay(src.source);
    }
}

float Audio::GetSoundDuration(int id) const {
    auto it = m_sources.find(id);
    if (it == m_sources.end()) return 0.0f;
    ALint sizeInBytes, bitsPerSample, channels, sampleRate;
    alGetBufferi(it->second.buffer, AL_SIZE, &sizeInBytes);
    alGetBufferi(it->second.buffer, AL_BITS, &bitsPerSample);
    alGetBufferi(it->second.buffer, AL_CHANNELS, &channels);
    alGetBufferi(it->second.buffer, AL_FREQUENCY, &sampleRate);
    if (bitsPerSample == 0 || channels == 0 || sampleRate == 0) return 0.0f;
    return static_cast<float>(sizeInBytes) * 8.0f / (bitsPerSample * channels * sampleRate);
}

float Audio::GetSoundPosition(int id) const {
    auto it = m_sources.find(id);
    if (it == m_sources.end()) return 0.0f;
    float pos;
    alGetSourcef(it->second.source, AL_SEC_OFFSET, &pos);
    return pos;
}

void Audio::SetSoundPosition(int id, float position) {
    auto it = m_sources.find(id);
    if (it != m_sources.end()) {
        alSourcef(it->second.source, AL_SEC_OFFSET, position);
    }
}

bool Audio::IsMusicPlaying() const {
    if (m_musicId < 0) return false;
    return IsSoundPlaying(m_musicId);
}

void Audio::SetDopplerFactor(float factor) {
    alDopplerFactor(factor);
}

void Audio::SetSpeedOfSound(float speed) {
    alSpeedOfSound(speed);
}

} // namespace planet
