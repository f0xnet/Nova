#pragma once

#include "../Core/Types.hpp"
#include "../Core/Logger.hpp"
#include "ResourceTypes.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <memory>
#include <string>

namespace NovaEngine {

    /**
     * @brief Gestionnaire centralisé de ressources (textures, polices, sons, musiques).
     * Ressources identifiées par un ID unique.
     */
    class ResourceManager {
    public:
        ResourceManager();
        ~ResourceManager();

        /**
         * @brief Charge toutes les ressources depuis un fichier JSON.
         * @param path Chemin vers le fichier JSON.
         * @return true si le chargement a réussi.
         */
        bool loadFromJSON(const std::string& path);

        // === TEXTURES ===
        bool loadTexture(const ID& id, const std::string& path);
        sf::Texture& getTexture(const ID& id);

        // === FONTS ===
        bool loadFont(const ID& id, const std::string& path);
        sf::Font& getFont(const ID& id);

        // === SONS ===
        bool loadSoundBuffer(const ID& id, const std::string& path);
        sf::SoundBuffer& getSoundBuffer(const ID& id);

        // === MUSIQUES ===
        bool loadMusic(const ID& id, const std::string& path);
        std::string getMusicPath(const ID& id) const;

        /**
         * @brief Nettoie toutes les ressources chargées.
         */
        void clear();

    private:
        std::unordered_map<ID, std::unique_ptr<sf::Texture>> m_textures;
        std::unordered_map<ID, std::unique_ptr<sf::Font>> m_fonts;
        std::unordered_map<ID, std::unique_ptr<sf::SoundBuffer>> m_soundBuffers;
        std::unordered_map<ID, std::string> m_musicPaths;
    };

} // namespace NovaEngine