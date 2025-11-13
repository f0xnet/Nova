#include "NovaEngine/Audio/SoundPlayer.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include <algorithm>

namespace NovaEngine {

    SoundPlayer::SoundPlayer()
        : m_volume(100.0f) {
        LOG_DEBUG("SoundPlayer initialized");
    }

    SoundPlayer::~SoundPlayer() {
        m_activeSounds.clear();
        LOG_DEBUG("SoundPlayer destroyed");
    }

    void SoundPlayer::play(const sf::SoundBuffer& buffer) {
        // Nettoyer les sons terminés avant d'ajouter un nouveau
        m_activeSounds.erase(
            std::remove_if(m_activeSounds.begin(), m_activeSounds.end(),
                [](const std::unique_ptr<sf::Sound>& s) {
                    return s->getStatus() == sf::Sound::Stopped;
                }),
            m_activeSounds.end()
        );

        // Créer et jouer le nouveau son
        auto sound = std::make_unique<sf::Sound>();
        sound->setBuffer(buffer);
        sound->setVolume(m_volume);
        sound->play();

        m_activeSounds.push_back(std::move(sound));

        LOG_TRACE("Sound played (active sounds: {})", m_activeSounds.size());
    }

    void SoundPlayer::setVolume(float volume) {
        m_volume = std::clamp(volume, 0.0f, 100.0f);
        
        // Appliquer le volume à tous les sons actifs
        for (auto& sound : m_activeSounds) {
            sound->setVolume(m_volume);
        }
        
        LOG_DEBUG("Sound volume set to {}", m_volume);
    }

    float SoundPlayer::getVolume() const {
        return m_volume;
    }

} // namespace NovaEngine