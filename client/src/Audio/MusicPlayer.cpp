#include "NovaEngine/Audio/MusicPlayer.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include <algorithm>

namespace NovaEngine {

    MusicPlayer::MusicPlayer()
        : m_music(std::make_unique<sf::Music>()), m_volume(100.0f) {
        LOG_DEBUG("MusicPlayer initialized");
    }

    MusicPlayer::~MusicPlayer() {
        stop();
        LOG_DEBUG("MusicPlayer destroyed");
    }

    bool MusicPlayer::play(const std::string& path, bool loop) {
        if (!m_music->openFromFile(path)) {
            LOG_ERROR("Failed to load music from file: {}", path);
            return false;
        }

        m_music->setLoop(loop);
        m_music->setVolume(m_volume);
        m_music->play();

        LOG_INFO("Playing music: {} (loop: {})", path, loop);
        return true;
    }

    void MusicPlayer::stop() {
        if (m_music->getStatus() != sf::Music::Stopped) {
            m_music->stop();
            LOG_INFO("Music stopped");
        }
    }

    void MusicPlayer::setVolume(float volume) {
        m_volume = std::clamp(volume, 0.0f, 100.0f);
        m_music->setVolume(m_volume);
        LOG_DEBUG("Music volume set to {}", m_volume);
    }

    float MusicPlayer::getVolume() const {
        return m_volume;
    }

    bool MusicPlayer::isPlaying() const {
        return m_music->getStatus() == sf::Music::Playing;
    }

} // namespace NovaEngine