#pragma once

#include "../Core/Types.hpp"
#include <SFML/Audio.hpp>
#include <queue>
#include <memory>

namespace NovaEngine {

    /**
     * @brief Joue des sons courts (effets) via un système de pool de sf::Sound.
     */
    class SoundPlayer {
    public:
        SoundPlayer();
        ~SoundPlayer();

        /**
         * @brief Joue un son à partir d’un buffer donné.
         * @param buffer Référence vers le buffer sonore à jouer.
         */
        void play(const sf::SoundBuffer& buffer);

        /**
         * @brief Définit le volume global des sons (0.0 à 100.0).
         */
        void setVolume(float volume);

        /**
         * @brief Récupère le volume actuel.
         */
        float getVolume() const;

    private:
        std::vector<std::unique_ptr<sf::Sound>> m_activeSounds;
        float m_volume;
    };

} // namespace NovaEngine