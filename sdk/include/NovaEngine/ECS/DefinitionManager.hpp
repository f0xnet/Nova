#pragma once

#include "../Backend/Core/BackendTypes.hpp"
#include "../Core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fstream>
#include <string>

namespace NovaEngine {

/**
 * @brief Manages entity definitions loaded from JSON files
 *
 * The DefinitionManager loads all entity definitions at startup and keeps them
 * in memory. When scenes are loaded, they reference these definitions by ID.
 *
 * This implements the two-tier JSON system:
 * - Tier 1: Definition files (Sprites.json, Lights.json, etc.) - loaded once at startup
 * - Tier 2: Scene files (interior_1.json, etc.) - reference definitions by ID
 */
class DefinitionManager {
private:
    std::unordered_map<ID, nlohmann::json> m_spriteDefinitions;
    std::unordered_map<ID, nlohmann::json> m_lightDefinitions;
    std::unordered_map<ID, nlohmann::json> m_animationDefinitions;
    std::unordered_map<ID, nlohmann::json> m_audioDefinitions;

public:
    /**
     * @brief Load all definition files
     * @param definitionsPath Base path to definitions folder (default: "assets/data/definitions/")
     * @return true if all files loaded successfully
     */
    bool loadDefinitions(const std::string& definitionsPath = "assets/data/definitions/") {
        LOG_INFO("Loading entity definitions from: {}", definitionsPath);

        bool success = true;
        success &= loadSpriteDefinitions(definitionsPath + "Sprites.json");
        success &= loadLightDefinitions(definitionsPath + "Lights.json");
        success &= loadAnimationDefinitions(definitionsPath + "Animations.json");
        success &= loadAudioDefinitions(definitionsPath + "Audio.json");

        if (success) {
            LOG_INFO("All entity definitions loaded successfully");
            LOG_INFO("  - Sprites: {}", m_spriteDefinitions.size());
            LOG_INFO("  - Lights: {}", m_lightDefinitions.size());
            LOG_INFO("  - Animations: {}", m_animationDefinitions.size());
            LOG_INFO("  - Audio: {}", m_audioDefinitions.size());
        } else {
            LOG_ERROR("Failed to load some entity definitions");
        }

        return success;
    }

    /**
     * @brief Get a sprite definition by ID
     * @param id The sprite ID
     * @return Pointer to the JSON definition, or nullptr if not found
     */
    const nlohmann::json* getSpriteDefinition(const ID& id) const {
        auto it = m_spriteDefinitions.find(id);
        if (it != m_spriteDefinitions.end()) {
            return &it->second;
        }
        LOG_WARN("Sprite definition not found: {}", id);
        return nullptr;
    }

    /**
     * @brief Get a light definition by ID
     * @param id The light ID
     * @return Pointer to the JSON definition, or nullptr if not found
     */
    const nlohmann::json* getLightDefinition(const ID& id) const {
        auto it = m_lightDefinitions.find(id);
        if (it != m_lightDefinitions.end()) {
            return &it->second;
        }
        LOG_WARN("Light definition not found: {}", id);
        return nullptr;
    }

    /**
     * @brief Get an animation definition by ID
     * @param id The animation ID
     * @return Pointer to the JSON definition, or nullptr if not found
     */
    const nlohmann::json* getAnimationDefinition(const ID& id) const {
        auto it = m_animationDefinitions.find(id);
        if (it != m_animationDefinitions.end()) {
            return &it->second;
        }
        LOG_WARN("Animation definition not found: {}", id);
        return nullptr;
    }

    /**
     * @brief Get an audio definition by ID
     * @param id The audio ID
     * @return Pointer to the JSON definition, or nullptr if not found
     */
    const nlohmann::json* getAudioDefinition(const ID& id) const {
        auto it = m_audioDefinitions.find(id);
        if (it != m_audioDefinitions.end()) {
            return &it->second;
        }
        LOG_WARN("Audio definition not found: {}", id);
        return nullptr;
    }

    /**
     * @brief Check if a sprite definition exists
     */
    bool hasSpriteDefinition(const ID& id) const {
        return m_spriteDefinitions.find(id) != m_spriteDefinitions.end();
    }

    /**
     * @brief Check if a light definition exists
     */
    bool hasLightDefinition(const ID& id) const {
        return m_lightDefinitions.find(id) != m_lightDefinitions.end();
    }

    /**
     * @brief Check if an animation definition exists
     */
    bool hasAnimationDefinition(const ID& id) const {
        return m_animationDefinitions.find(id) != m_animationDefinitions.end();
    }

    /**
     * @brief Check if an audio definition exists
     */
    bool hasAudioDefinition(const ID& id) const {
        return m_audioDefinitions.find(id) != m_audioDefinitions.end();
    }

private:
    /**
     * @brief Load sprite definitions from JSON file
     */
    bool loadSpriteDefinitions(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            LOG_WARN("Sprite definitions file not found: {} (skipping)", path);
            return true;  // Not critical if file doesn't exist
        }

        try {
            nlohmann::json data;
            file >> data;

            if (data.contains("sprites") && data["sprites"].is_array()) {
                for (const auto& sprite : data["sprites"]) {
                    if (sprite.contains("id")) {
                        ID id = sprite["id"];
                        m_spriteDefinitions[id] = sprite;
                        LOG_DEBUG("Loaded sprite definition: {}", id);
                    }
                }
            }

            LOG_INFO("Loaded {} sprite definitions from {}", m_spriteDefinitions.size(), path);
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse sprite definitions: {}", e.what());
            return false;
        }
    }

    /**
     * @brief Load light definitions from JSON file
     */
    bool loadLightDefinitions(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            LOG_WARN("Light definitions file not found: {} (skipping)", path);
            return true;
        }

        try {
            nlohmann::json data;
            file >> data;

            if (data.contains("lights") && data["lights"].is_array()) {
                for (const auto& light : data["lights"]) {
                    if (light.contains("id")) {
                        ID id = light["id"];
                        m_lightDefinitions[id] = light;
                        LOG_DEBUG("Loaded light definition: {}", id);
                    }
                }
            }

            LOG_INFO("Loaded {} light definitions from {}", m_lightDefinitions.size(), path);
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse light definitions: {}", e.what());
            return false;
        }
    }

    /**
     * @brief Load animation definitions from JSON file
     */
    bool loadAnimationDefinitions(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            LOG_WARN("Animation definitions file not found: {} (skipping)", path);
            return true;
        }

        try {
            nlohmann::json data;
            file >> data;

            if (data.contains("animations") && data["animations"].is_array()) {
                for (const auto& anim : data["animations"]) {
                    if (anim.contains("id")) {
                        ID id = anim["id"];
                        m_animationDefinitions[id] = anim;
                        LOG_DEBUG("Loaded animation definition: {}", id);
                    }
                }
            }

            LOG_INFO("Loaded {} animation definitions from {}", m_animationDefinitions.size(), path);
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse animation definitions: {}", e.what());
            return false;
        }
    }

    /**
     * @brief Load audio definitions from JSON file
     */
    bool loadAudioDefinitions(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            LOG_WARN("Audio definitions file not found: {} (skipping)", path);
            return true;
        }

        try {
            nlohmann::json data;
            file >> data;

            if (data.contains("sounds") && data["sounds"].is_array()) {
                for (const auto& sound : data["sounds"]) {
                    if (sound.contains("id")) {
                        ID id = sound["id"];
                        m_audioDefinitions[id] = sound;
                        LOG_DEBUG("Loaded audio definition: {}", id);
                    }
                }
            }

            LOG_INFO("Loaded {} audio definitions from {}", m_audioDefinitions.size(), path);
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse audio definitions: {}", e.what());
            return false;
        }
    }
};

} // namespace NovaEngine
