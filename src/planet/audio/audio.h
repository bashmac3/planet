#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#ifndef PLANET_NO_AUDIO
#if defined(__APPLE__)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#endif

namespace planet {

class Audio {
public:
    static Audio& Instance();

    bool Init();
    void Shutdown();
    void Update();

    int PlaySound(const std::string& path, float volume = 1.0f, bool loop = false);
    void StopSound(int id);
    void PlayMusic(const std::string& path, float volume = 1.0f);
    void StopMusic();

    void SetMasterVolume(float volume);
    float GetMasterVolume() const { return m_masterVolume; }

    bool IsSoundPlaying(int id) const;
    void SetSoundVolume(int id, float volume);
    void SetSoundPitch(int id, float pitch);
    void StopAllSounds();
    void PauseSound(int id);
    void ResumeSound(int id);
    void PauseAllSounds();
    void ResumeAllSounds();
    float GetSoundDuration(int id) const;
    float GetSoundPosition(int id) const;
    void SetSoundPosition(int id, float position);

    bool IsMusicPlaying() const;

    void SetDopplerFactor(float factor);
    void SetSpeedOfSound(float speed);

private:
    Audio() = default;

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

#ifndef PLANET_NO_AUDIO
    struct AudioSource {
        ALuint source;
        ALuint buffer;
        bool isMusic = false;
    };

    ALCdevice* m_device = nullptr;
    ALCcontext* m_context = nullptr;

    int m_musicId = -1;
    std::unordered_map<int, AudioSource> m_sources;

    ALuint LoadWAV(const std::string& path);
    void DeleteSource(int id);
#else
    // Stub — no AL types available
#endif

    float m_masterVolume = 1.0f;
    int m_nextSourceId = 1;
};

} // namespace planet
