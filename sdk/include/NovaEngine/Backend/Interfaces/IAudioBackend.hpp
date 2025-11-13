#pragma once
#include "../Core/BackendTypes.hpp"

namespace NovaEngine {
class IAudioBackend {
public:
    virtual ~IAudioBackend() = default;
    
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    virtual void playSound(SoundHandle handle, f32 volume = 1.0f, f32 pitch = 1.0f, bool loop = false) = 0;
    virtual void stopSound(SoundHandle handle) = 0;
    virtual void stopAllSounds() = 0;
    virtual void setSoundVolume(f32 volume) = 0;
    virtual f32 getSoundVolume() const = 0;
    virtual SoundStatus getSoundStatus(SoundHandle handle) const = 0;
    
    virtual void playMusic(MusicHandle handle, bool loop = true) = 0;
    virtual void stopMusic() = 0;
    virtual void pauseMusic() = 0;
    virtual void resumeMusic() = 0;
    virtual void setMusicVolume(f32 volume) = 0;
    virtual f32 getMusicVolume() const = 0;
    virtual SoundStatus getMusicStatus() const = 0;
};
}
