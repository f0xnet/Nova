#pragma once

#include "../Core/Types.hpp"
#include "../Core/Logger.hpp"
#include "../Resources/ResourceManager.hpp"
#include "SoundPlayer.hpp"
#include "MusicPlayer.hpp"

#include <memory>
#include <string>

namespace NovaEngine {

    /**
     * @brief AudioManager centralisant les contrôles du son et de la musique.
     * Utilise les ressources du ResourceManager.
     */
    class AudioManager {
    public:
        /**
         * @brief Crée un gestionnaire audio lié à un ResourceManager.
         */
        explicit AudioManager(ResourceManager& resourceManager);

        ~AudioManager();

        /**
         * @brief Joue un effet sonore (short).
         * @param id ID du son (doit être présent dans le ResourceManager).
         */
        void playSound(const ID& id);

        /**
         * @brief Joue une musique de fond.
         * @param id ID de la musique.
         * @param loop Si true, la musique boucle.
         */
        void playMusic(const ID& id, bool loop = true);

        /**
         * @brief Coupe la musique en cours.
         */
        void stopMusic();

        /**
         * @brief Change le volume général des sons.
         * @param volume de 0.0 à 100.0
         */
        void setSoundVolume(float volume);

        /**
         * @brief Change le volume de la musique.
         * @param volume de 0.0 à 100.0
         */
        void setMusicVolume(float volume);

    private:
        ResourceManager& m_resourceManager;
        SoundPlayer m_soundPlayer;
        MusicPlayer m_musicPlayer;
    };

} // namespace NovaEngine