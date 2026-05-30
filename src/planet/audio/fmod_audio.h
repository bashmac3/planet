#pragma once

#include <string>
#include <unordered_map>

namespace FMOD {
    class System;
    class Sound;
    class Channel;
}

namespace planet {

class FmodAudio {
public:
    static FmodAudio& Instance();

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

private:
    FmodAudio() = default;
    FmodAudio(const FmodAudio&) = delete;
    FmodAudio& operator=(const FmodAudio&) = delete;

    struct ActiveChannel {
        FMOD::Channel* channel = nullptr;
        FMOD::Sound* sound = nullptr;
        bool isMusic = false;
    };

    FMOD::System* m_system = nullptr;
    float m_masterVolume = 1.0f;
    int m_nextId = 1;
    std::unordered_map<int, ActiveChannel> m_channels;
    int m_musicId = -1;
};

} // namespace planet
