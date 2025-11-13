#include "NovaEngine/Backend/SFML/SFMLAudioBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLResourceBackend.hpp"
#include <algorithm>

namespace NovaEngine {
SFMLAudioBackend::SFMLAudioBackend() : m_resources(nullptr) {}
SFMLAudioBackend::~SFMLAudioBackend() { shutdown(); }

bool SFMLAudioBackend::initialize() { 
    return true; 
}

void SFMLAudioBackend::shutdown() {
    stopAllSounds();
    stopMusic();
}

void SFMLAudioBackend::playSound(SoundHandle handle, f32 volume, f32 pitch, bool loop) {
    if(!m_resources || handle == INVALID_HANDLE) return;
    
    sf::SoundBuffer* buffer = m_resources->getSoundBuffer(handle);
    if(!buffer) return;
    
    auto sound = std::make_unique<sf::Sound>();
    sound->setBuffer(*buffer);
    
    f32 finalVolume = volume * m_soundVolume;
    if(finalVolume < 0.0f) finalVolume = 0.0f;
    if(finalVolume > 100.0f) finalVolume = 100.0f;
    sound->setVolume(finalVolume);
    
    if(pitch <= 0.0f) pitch = 1.0f;
    sound->setPitch(pitch);
    
    sound->setLoop(loop);
    sound->play();
    
    m_activeSounds.push_back(std::move(sound));
    
    m_activeSounds.erase(
        std::remove_if(m_activeSounds.begin(), m_activeSounds.end(),
            [](const auto& s) { return s->getStatus() == sf::Sound::Stopped; }),
        m_activeSounds.end()
    );
}

void SFMLAudioBackend::stopSound(SoundHandle) {}

void SFMLAudioBackend::stopAllSounds() { 
    m_activeSounds.clear(); 
}

void SFMLAudioBackend::setSoundVolume(f32 volume) { 
    m_soundVolume = volume * 100.0f; 
    if(m_soundVolume < 0.0f) m_soundVolume = 0.0f;
    if(m_soundVolume > 100.0f) m_soundVolume = 100.0f;
}

f32 SFMLAudioBackend::getSoundVolume() const { 
    return m_soundVolume / 100.0f; 
}

SoundStatus SFMLAudioBackend::getSoundStatus(SoundHandle) const { 
    return SoundStatus::Stopped; 
}

void SFMLAudioBackend::playMusic(MusicHandle handle, bool loop) {
    if(!m_resources) return;
    String path = m_resources->getMusicPath(handle);
    if(path.empty()) return;
    
    if(m_currentMusic) m_currentMusic->stop();
    m_currentMusic = std::make_unique<sf::Music>();
    
    if(m_currentMusic->openFromFile(path)) {
        m_currentMusic->setLoop(loop);
        m_currentMusic->setVolume(m_musicVolume);
        m_currentMusic->play();
    } else {
        m_currentMusic.reset();
    }
}

void SFMLAudioBackend::stopMusic() { 
    if(m_currentMusic) m_currentMusic->stop(); 
}

void SFMLAudioBackend::pauseMusic() { 
    if(m_currentMusic) m_currentMusic->pause(); 
}

void SFMLAudioBackend::resumeMusic() { 
    if(m_currentMusic) m_currentMusic->play(); 
}

void SFMLAudioBackend::setMusicVolume(f32 volume) { 
    m_musicVolume = volume * 100.0f; 
    if(m_musicVolume < 0.0f) m_musicVolume = 0.0f;
    if(m_musicVolume > 100.0f) m_musicVolume = 100.0f;
    if(m_currentMusic) m_currentMusic->setVolume(m_musicVolume); 
}

f32 SFMLAudioBackend::getMusicVolume() const { 
    return m_musicVolume / 100.0f; 
}

SoundStatus SFMLAudioBackend::getMusicStatus() const { 
    return m_currentMusic ? SFMLConv::fromSFMLMusicStatus(m_currentMusic->getStatus()) : SoundStatus::Stopped; 
}
}
