#pragma once
#include "../Interfaces/IAudioBackend.hpp"
#include "SFMLConversions.hpp"
#include <SFML/Audio.hpp>
#include <memory>
#include <vector>

namespace NovaEngine {
class SFMLResourceBackend;

class SFMLAudioBackend : public IAudioBackend {
public:
    SFMLAudioBackend();
    ~SFMLAudioBackend() override;
    
    void setResourceBackend(SFMLResourceBackend* res) { m_resources = res; }
    
    bool initialize() override;
    void shutdown() override;
    
    void playSound(SoundHandle handle, f32 volume, f32 pitch, bool loop) override;
    void stopSound(SoundHandle handle) override;
    void stopAllSounds() override;
    void setSoundVolume(f32 volume) override;
    f32 getSoundVolume() const override;
    SoundStatus getSoundStatus(SoundHandle handle) const override;
    
    void playMusic(MusicHandle handle, bool loop) override;
    void stopMusic() override;
    void pauseMusic() override;
    void resumeMusic() override;
    void setMusicVolume(f32 volume) override;
    f32 getMusicVolume() const override;
    SoundStatus getMusicStatus() const override;
    
private:
    SFMLResourceBackend* m_resources;
    std::vector<std::unique_ptr<sf::Sound>> m_activeSounds;
    std::unique_ptr<sf::Music> m_currentMusic;
    f32 m_soundVolume = 100.0f;
    f32 m_musicVolume = 100.0f;
};
}
