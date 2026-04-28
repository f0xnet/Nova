#include "NovaEngine/Resources/ResourceManager.hpp"
#include "NovaEngine/Resources/ResourceTypes.hpp"
#include "NovaEngine/Core/Logger.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <memory>

namespace NovaEngine {

    struct ResourceManager::Impl {
        std::unordered_map<ID, std::unique_ptr<sf::Texture>>     textures;
        std::unordered_map<ID, std::unique_ptr<sf::Font>>         fonts;
        std::unordered_map<ID, std::unique_ptr<sf::SoundBuffer>>  soundBuffers;
        std::unordered_map<ID, std::string>                       musicPaths;
    };

    ResourceManager::ResourceManager() : m_impl(std::make_unique<Impl>()) {
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

        bool allLoaded = true;

        for (const auto& tex : pack.textures) {
            if (!loadTexture(tex.id, tex.path)) allLoaded = false;
        }
        for (const auto& font : pack.fonts) {
            if (!loadFont(font.id, font.path)) allLoaded = false;
        }
        for (const auto& snd : pack.sounds) {
            if (!loadSoundBuffer(snd.id, snd.path)) allLoaded = false;
        }
        for (const auto& music : pack.musics) {
            if (!loadMusic(music.id, music.path)) allLoaded = false;
        }

        if (allLoaded)
            LOG_INFO("Resource pack loaded successfully");
        else
            LOG_WARN("Resource pack loaded with some errors");

        return allLoaded;
    }

    bool ResourceManager::loadTexture(const ID& id, const std::string& path) {
        if (m_impl->textures.find(id) != m_impl->textures.end()) {
            LOG_WARN("Texture '{}' already loaded", id);
            return false;
        }
        auto texture = std::make_unique<sf::Texture>();
        if (!texture->loadFromFile(path)) {
            LOG_ERROR("Failed to load texture '{}' from: {}", id, path);
            return false;
        }
        m_impl->textures[id] = std::move(texture);
        LOG_DEBUG("Texture '{}' loaded from: {}", id, path);
        return true;
    }

    bool ResourceManager::loadFont(const ID& id, const std::string& path) {
        if (m_impl->fonts.find(id) != m_impl->fonts.end()) {
            LOG_WARN("Font '{}' already loaded", id);
            return false;
        }
        auto font = std::make_unique<sf::Font>();
        if (!font->loadFromFile(path)) {
            LOG_ERROR("Failed to load font '{}' from: {}", id, path);
            return false;
        }
        m_impl->fonts[id] = std::move(font);
        LOG_DEBUG("Font '{}' loaded from: {}", id, path);
        return true;
    }

    bool ResourceManager::loadSoundBuffer(const ID& id, const std::string& path) {
        if (m_impl->soundBuffers.find(id) != m_impl->soundBuffers.end()) {
            LOG_WARN("SoundBuffer '{}' already loaded", id);
            return false;
        }
        auto buffer = std::make_unique<sf::SoundBuffer>();
        if (!buffer->loadFromFile(path)) {
            LOG_ERROR("Failed to load sound buffer '{}' from: {}", id, path);
            return false;
        }
        m_impl->soundBuffers[id] = std::move(buffer);
        LOG_DEBUG("SoundBuffer '{}' loaded from: {}", id, path);
        return true;
    }

    bool ResourceManager::loadMusic(const ID& id, const std::string& path) {
        if (m_impl->musicPaths.find(id) != m_impl->musicPaths.end()) {
            LOG_WARN("Music '{}' already registered", id);
            return false;
        }
        std::ifstream testFile(path);
        if (!testFile.good()) {
            LOG_ERROR("Music file '{}' not found at: {}", id, path);
            return false;
        }
        m_impl->musicPaths[id] = path;
        LOG_DEBUG("Music '{}' registered with path: {}", id, path);
        return true;
    }

    std::string ResourceManager::getMusicPath(const ID& id) const {
        auto it = m_impl->musicPaths.find(id);
        if (it != m_impl->musicPaths.end()) return it->second;
        LOG_ERROR("Music '{}' not found in ResourceManager", id);
        return "";
    }

    void ResourceManager::clear() {
        m_impl->textures.clear();
        m_impl->fonts.clear();
        m_impl->soundBuffers.clear();
        m_impl->musicPaths.clear();
        LOG_DEBUG("All resources cleared");
    }

} // namespace NovaEngine
