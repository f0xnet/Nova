#pragma once

#include "NovaEngine/Core/Types.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>

namespace NovaEngine {

    /**
     * @brief Structure pour les paramètres d'affichage
     */
    struct DisplayConfig {
        u32 width = 1920;
        u32 height = 1080;
        bool fullscreen = false;
        bool vsync = true;
        u32 frameRateLimit = 60;
        u32 antialiasingLevel = 0;
        
        // Résolution native pour le rescaling
        u32 nativeWidth = 3840;
        u32 nativeHeight = 2160;
        
        DisplayConfig() = default;
    };

    /**
     * @brief Structure pour les paramètres audio
     */
    struct AudioConfig {
        f32 masterVolume = 100.0f;
        f32 musicVolume = 80.0f;
        f32 soundVolume = 90.0f;
        bool muteAll = false;
        std::string audioDevice = "default";
        
        AudioConfig() = default;
    };

    /**
     * @brief Structure pour les paramètres de contrôles
     */
    struct InputConfig {
        struct KeyBinding {
            std::string action;
            std::vector<std::string> keys;
        };
        
        std::vector<KeyBinding> keyBindings;
        f32 mouseSensitivity = 1.0f;
        bool invertMouse = false;
        
        InputConfig() = default;
    };

    /**
     * @brief Structure pour les paramètres de debug
     */
    struct DebugConfig {
        bool enableLogging = true;
        std::string logLevel = "INFO";
        std::string logFile = "logs/nova.log";
        bool showFPS = false;
        bool showDebugInfo = false;
        bool enableProfiler = false;
        
        DebugConfig() = default;
    };

    /**
     * @brief Structure pour les paramètres de jeu
     */
    struct GameConfig {
        std::string language = "en";
        std::string playerName = "Player";
        bool autoSave = true;
        u32 autoSaveInterval = 300; // secondes
        std::string savePath = "saves/";
        
        GameConfig() = default;
    };

    /**
     * @brief Gestionnaire de configuration centralisé
     */
    class ConfigManager {
    public:
        ConfigManager();
        ~ConfigManager();

        /**
         * @brief Charge la configuration depuis un fichier .ini
         */
        bool loadFromFile(const std::string& configPath = "config/engine.ini");

        /**
         * @brief Sauvegarde la configuration actuelle vers un fichier .ini
         */
        bool saveToFile(const std::string& configPath = "config/engine.ini") const;

        /**
         * @brief Crée un fichier de configuration par défaut
         */
        bool createDefaultConfig(const std::string& configPath = "config/engine.ini") const;

        // Accès aux configurations
        const DisplayConfig& getDisplayConfig() const { return m_displayConfig; }
        const AudioConfig& getAudioConfig() const { return m_audioConfig; }
        const InputConfig& getInputConfig() const { return m_inputConfig; }
        const DebugConfig& getDebugConfig() const { return m_debugConfig; }
        const GameConfig& getGameConfig() const { return m_gameConfig; }

        // Modification des configurations
        DisplayConfig& getDisplayConfig() { return m_displayConfig; }
        AudioConfig& getAudioConfig() { return m_audioConfig; }
        InputConfig& getInputConfig() { return m_inputConfig; }
        DebugConfig& getDebugConfig() { return m_debugConfig; }
        GameConfig& getGameConfig() { return m_gameConfig; }

        // Validation des paramètres
        bool validateDisplayConfig() const;
        bool validateAudioConfig() const;
        void clampValues();

        // Utilitaires
        std::string getConfigString() const;
        void debugPrint() const;

        // Méthodes statiques pour accès global
        static ConfigManager& getInstance();
        static bool initializeGlobalConfig(const std::string& configPath = "config/engine.ini");

    private:
        DisplayConfig m_displayConfig;
        AudioConfig m_audioConfig;
        InputConfig m_inputConfig;
        DebugConfig m_debugConfig;
        GameConfig m_gameConfig;

        // Parser INI simple
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_iniData;
        
        bool parseIniFile(const std::string& filepath);
        void writeIniFile(const std::string& filepath) const;
        
        // Helpers pour parsing
        std::string trim(const std::string& str) const;
        bool stringToBool(const std::string& str) const;
        f32 stringToFloat(const std::string& str) const;
        u32 stringToUint(const std::string& str) const;
        std::vector<std::string> stringToVector(const std::string& str, char delimiter = ',') const;
        
        // Helpers pour écriture
        std::string boolToString(bool value) const;
        std::string vectorToString(const std::vector<std::string>& vec, char delimiter = ',') const;
        
        // Chargement des sections
        void loadDisplaySection();
        void loadAudioSection();
        void loadInputSection();
        void loadDebugSection();
        void loadGameSection();
        
        // Sauvegarde des sections
        void saveDisplaySection(std::ofstream& file) const;
        void saveAudioSection(std::ofstream& file) const;
        void saveInputSection(std::ofstream& file) const;
        void saveDebugSection(std::ofstream& file) const;
        void saveGameSection(std::ofstream& file) const;
        
        // Utilitaires
        bool createDirectoryIfNotExists(const std::string& path) const;
        std::string getValue(const std::string& section, const std::string& key, const std::string& defaultValue = "") const;
    };

    /**
     * @brief Macros pour accès rapide à la configuration
     */
    #define DISPLAY_CONFIG ::NovaEngine::ConfigManager::getInstance().getDisplayConfig()
    #define AUDIO_CONFIG ::NovaEngine::ConfigManager::getInstance().getAudioConfig()
    #define INPUT_CONFIG ::NovaEngine::ConfigManager::getInstance().getInputConfig()
    #define DEBUG_CONFIG ::NovaEngine::ConfigManager::getInstance().getDebugConfig()
    #define GAME_CONFIG ::NovaEngine::ConfigManager::getInstance().getGameConfig()

} // namespace NovaEngine