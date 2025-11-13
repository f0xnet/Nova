#include "NovaEngine/Resources/ResourceManager.hpp"
#include "NovaEngine/Resources/ResourceTypes.hpp"
#include "NovaEngine/Core/Logger.hpp"

#include <nlohmann/json.hpp>
#include <fstream>

namespace NovaEngine {

    ResourceManager::ResourceManager() {
        LOG_DEBUG("ResourceManager initialized");
    }

    ResourceManager::~ResourceManager() {
        clear();
        LOG_DEBUG("ResourceManager destroyed");
    }

    bool ResourceManager::loadFromJSON(const std::string& path) {
        LOG_INFO("Loading resource pack from JSON: {}", path);
        std::ifstream file(path);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open resource JSON: {}", path);
            return false;
        }

        nlohmann::json jsonData;
        try {
            file >> jsonData;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse JSON file: {}", e.what());
            return false;
        }

        ResourcePack pack;
        from_json(jsonData, pack);

        // Charger toutes les ressources
        bool allLoaded = true;
        
        for (const auto& tex : pack.textures) {
            if (!loadTexture(tex.id, tex.path)) {
                allLoaded = false;
            }
        }
        for (const auto& font : pack.fonts) {
            if (!loadFont(font.id, font.path)) {
                allLoaded = false;
            }
        }
        for (const auto& snd : pack.sounds) {
            if (!loadSoundBuffer(snd.id, snd.path)) {
                allLoaded = false;
            }
        }
        for (const auto& music : pack.musics) {
            if (!loadMusic(music.id, music.path)) {
                allLoaded = false;
            }
        }

        if (allLoaded) {
            LOG_INFO("Resource pack loaded successfully");
        } else {
            LOG_WARN("Resource pack loaded with some errors");
        }
        
        return allLoaded;
    }

    bool ResourceManager::loadTexture(const ID& id, const std::string& path) {
        // C++17 : utiliser find() au lieu de contains()
        if (m_textures.find(id) != m_textures.end()) {
            LOG_WARN("Texture '{}' already loaded", id);
            return false;
        }

        auto texture = std::make_unique<sf::Texture>();
        if (!texture->loadFromFile(path)) {
            LOG_ERROR("Failed to load texture '{}' from: {}", id, path);
            return false;
        }

        m_textures[id] = std::move(texture);
        LOG_DEBUG("Texture '{}' loaded from: {}", id, path);
        return true;
    }

    sf::Texture& ResourceManager::getTexture(const ID& id) {
        auto it = m_textures.find(id);
        if (it == m_textures.end()) {
            LOG_ERROR("Texture '{}' not found in ResourceManager", id);
            throw std::out_of_range("Texture not found: " + id);
        }
        return *(it->second);
    }

    bool ResourceManager::loadFont(const ID& id, const std::string& path) {
        if (m_fonts.find(id) != m_fonts.end()) {
            LOG_WARN("Font '{}' already loaded", id);
            return false;
        }

        auto font = std::make_unique<sf::Font>();
        if (!font->loadFromFile(path)) {
            LOG_ERROR("Failed to load font '{}' from: {}", id, path);
            return false;
        }

        m_fonts[id] = std::move(font);
        LOG_DEBUG("Font '{}' loaded from: {}", id, path);
        return true;
    }

    sf::Font& ResourceManager::getFont(const ID& id) {
        auto it = m_fonts.find(id);
        if (it == m_fonts.end()) {
            LOG_ERROR("Font '{}' not found in ResourceManager", id);
            throw std::out_of_range("Font not found: " + id);
        }
        return *(it->second);
    }

    bool ResourceManager::loadSoundBuffer(const ID& id, const std::string& path) {
        if (m_soundBuffers.find(id) != m_soundBuffers.end()) {
            LOG_WARN("SoundBuffer '{}' already loaded", id);
            return false;
        }

        auto buffer = std::make_unique<sf::SoundBuffer>();
        if (!buffer->loadFromFile(path)) {
            LOG_ERROR("Failed to load sound buffer '{}' from: {}", id, path);
            return false;
        }

        m_soundBuffers[id] = std::move(buffer);
        LOG_DEBUG("SoundBuffer '{}' loaded from: {}", id, path);
        return true;
    }

    sf::SoundBuffer& ResourceManager::getSoundBuffer(const ID& id) {
        auto it = m_soundBuffers.find(id);
        if (it == m_soundBuffers.end()) {
            LOG_ERROR("SoundBuffer '{}' not found in ResourceManager", id);
            throw std::out_of_range("SoundBuffer not found: " + id);
        }
        return *(it->second);
    }

    bool ResourceManager::loadMusic(const ID& id, const std::string& path) {
        if (m_musicPaths.find(id) != m_musicPaths.end()) {
            LOG_WARN("Music '{}' already registered", id);
            return false;
        }

        // Vérifier que le fichier existe
        std::ifstream testFile(path);
        if (!testFile.good()) {
            LOG_ERROR("Music file '{}' not found at: {}", id, path);
            return false;
        }
        testFile.close();

        m_musicPaths[id] = path;
        LOG_DEBUG("Music '{}' registered with path: {}", id, path);
        return true;
    }

    std::string ResourceManager::getMusicPath(const ID& id) const {
        auto it = m_musicPaths.find(id);
        if (it != m_musicPaths.end()) {
            return it->second;
        }
        LOG_ERROR("Music '{}' not found in ResourceManager", id);
        return "";
    }

    void ResourceManager::clear() {
        m_textures.clear();
        m_fonts.clear();
        m_soundBuffers.clear();
        m_musicPaths.clear();
        LOG_DEBUG("All resources cleared");
    }

} // namespace NovaEngine