#include "NovaEngine/Audio/AudioManager.hpp"
#include "NovaEngine/Core/Logger.hpp"

namespace NovaEngine {

    AudioManager::AudioManager(ResourceManager& resourceManager)
        : m_resourceManager(resourceManager),
          m_soundPlayer(),
          m_musicPlayer() {
        LOG_INFO("AudioManager initialized");
    }

    AudioManager::~AudioManager() {
        m_musicPlayer.stop();
        LOG_INFO("AudioManager destroyed");
    }

    void AudioManager::playSound(const ID& id) {
        try {
            sf::SoundBuffer& buffer = m_resourceManager.getSoundBuffer(id);
            m_soundPlayer.play(buffer);
            LOG_DEBUG("Playing sound '{}'", id);
        } catch (const std::exception& e) {
            LOG_ERROR("Sound '{}' not found in ResourceManager: {}", id, e.what());
        }
    }

    void AudioManager::playMusic(const ID& id, bool loop) {
        std::string path = m_resourceManager.getMusicPath(id);
        if (path.empty()) {
            LOG_ERROR("Music '{}' not found or not registered in ResourceManager", id);
            return;
        }

        if (!m_musicPlayer.play(path, loop)) {
            LOG_ERROR("Failed to play music '{}'", id);
        } else {
            LOG_INFO("Playing music '{}' (loop: {})", id, loop);
        }
    }

    void AudioManager::stopMusic() {
        m_musicPlayer.stop();
        LOG_DEBUG("Music stopped via AudioManager");
    }

    void AudioManager::setSoundVolume(float volume) {
        m_soundPlayer.setVolume(volume);
        LOG_DEBUG("Sound volume set to {}", volume);
    }

    void AudioManager::setMusicVolume(float volume) {
        m_musicPlayer.setVolume(volume);
        LOG_DEBUG("Music volume set to {}", volume);
    }

} // namespace NovaEngine