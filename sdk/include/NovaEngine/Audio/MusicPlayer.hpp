#pragma once

#include "../Core/Types.hpp"
#include <SFML/Audio.hpp>
#include <memory>
#include <string>

namespace NovaEngine {

    /**
     * @brief Gère la lecture d’une musique de fond.
     */
    class MusicPlayer {
    public:
        MusicPlayer();
        ~MusicPlayer();

        /**
         * @brief Joue une musique depuis un fichier.
         * @param path Chemin vers le fichier audio.
         * @param loop Indique si la musique doit boucler.
         * @return true si succès.
         */
        bool play(const std::string& path, bool loop = true);

        /**
         * @brief Coupe la musique.
         */
        void stop();

        /**
         * @brief Change le volume de la musique (0.0 à 100.0).
         */
        void setVolume(float volume);

        /**
         * @brief Récupère le volume actuel.
         */
        float getVolume() const;

        /**
         * @brief Vérifie si une musique est en cours.
         */
        bool isPlaying() const;

    private:
        std::unique_ptr<sf::Music> m_music;
        float m_volume;
    };

} // namespace NovaEngine