#include "NovaEngine/Core/ConfigManager.hpp"
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace NovaEngine {

    ConfigManager::ConfigManager() {
        LOG_DEBUG("ConfigManager created");
    }

    ConfigManager::~ConfigManager() {
        LOG_DEBUG("ConfigManager destroyed");
    }

    ConfigManager& ConfigManager::getInstance() {
        static ConfigManager instance;
        return instance;
    }

    bool ConfigManager::initializeGlobalConfig(const std::string& configPath) {
        auto& instance = getInstance();
        
        // Créer le fichier de config par défaut s'il n'existe pas
        if (!std::filesystem::exists(configPath)) {
            LOG_INFO("Config file not found, creating default: {}", configPath);
            if (!instance.createDefaultConfig(configPath)) {
                LOG_ERROR("Failed to create default config file");
                return false;
            }
        }
        return instance.loadFromFile(configPath);
    }

    bool ConfigManager::loadFromFile(const std::string& configPath) {
        LOG_INFO("Loading configuration from: {}", configPath);
        
        if (!parseIniFile(configPath)) {
            LOG_ERROR("Failed to parse config file: {}", configPath);
            return false;
        }
        
        // Charger toutes les sections
        loadDisplaySection();
        loadAudioSection();
        loadInputSection();
        loadDebugSection();
        loadGameSection();
        
        // Valider et corriger les valeurs
        clampValues();
        
        LOG_INFO("Configuration loaded successfully");
        debugPrint();
        return true;
    }

    bool ConfigManager::saveToFile(const std::string& configPath) const {
        LOG_INFO("Saving configuration to: {}", configPath);
        
        // Créer le répertoire si nécessaire
        std::filesystem::path filePath(configPath);
        if (!createDirectoryIfNotExists(filePath.parent_path().string())) {
            LOG_ERROR("Failed to create config directory");
            return false;
        }
        
        std::ofstream file(configPath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open config file for writing: {}", configPath);
            return false;
        }
        
        file << "# NovaEngine Configuration File\n";
        file << "# Generated automatically - Edit carefully\n\n";
        
        saveDisplaySection(file);
        saveAudioSection(file);
        saveInputSection(file);
        saveDebugSection(file);
        saveGameSection(file);
        
        file.close();
        LOG_INFO("Configuration saved successfully");
        return true;
    }

    bool ConfigManager::createDefaultConfig(const std::string& configPath) const {
        LOG_INFO("Creating default configuration: {}", configPath);
        
        // Créer le répertoire si nécessaire
        std::filesystem::path filePath(configPath);
        if (!createDirectoryIfNotExists(filePath.parent_path().string())) {
            LOG_ERROR("Failed to create config directory");
            return false;
        }
        
        std::ofstream file(configPath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to create default config file: {}", configPath);
            return false;
        }
        
        file << "# NovaEngine Configuration File\n";
        file << "# Edit these values to customize your game experience\n\n";
        
        file << "[Display]\n";
        file << "# Window resolution\n";
        file << "width=1920\n";
        file << "height=1080\n\n";
        file << "# Display mode\n";
        file << "fullscreen=false\n";
        file << "vsync=true\n";
        file << "frameRateLimit=60\n";
        file << "antialiasingLevel=0\n\n";
        file << "# Native resolution for UI scaling\n";
        file << "nativeWidth=3840\n";
        file << "nativeHeight=2160\n\n";
        
        file << "[Audio]\n";
        file << "# Volume levels (0-100)\n";
        file << "masterVolume=100.0\n";
        file << "musicVolume=80.0\n";
        file << "soundVolume=90.0\n";
        file << "muteAll=false\n";
        file << "audioDevice=default\n\n";
        
        file << "[Input]\n";
        file << "# Mouse settings\n";
        file << "mouseSensitivity=1.0\n";
        file << "invertMouse=false\n\n";
        file << "# Key bindings\n";
        file << "moveUp=W,Up\n";
        file << "moveDown=S,Down\n";
        file << "moveLeft=A,Left\n";
        file << "moveRight=D,Right\n";
        file << "interact=E,Enter,Space\n";
        file << "menu=Escape,M\n\n";
        
        file << "[Debug]\n";
        file << "enableLogging=true\n";
        file << "logLevel=INFO\n";
        file << "logFile=logs/nova.log\n";
        file << "showFPS=false\n";
        file << "showDebugInfo=false\n";
        file << "enableProfiler=false\n\n";
        
        file << "[Game]\n";
        file << "language=en\n";
        file << "playerName=Player\n";
        file << "autoSave=true\n";
        file << "autoSaveInterval=300\n";
        file << "savePath=saves/\n";
        
        file.close();
        LOG_INFO("Default configuration created successfully");
        return true;
    }

    bool ConfigManager::parseIniFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("Cannot open config file: {}", filepath);
            return false;
        }
        
        m_iniData.clear();
        std::string currentSection;
        std::string line;
        u32 lineNumber = 0;
        
        while (std::getline(file, line)) {
            lineNumber++;
            line = trim(line);
            
            // Ignorer lignes vides et commentaires
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }
            
            // Section [SectionName]
            if (line.front() == '[' && line.back() == ']') {
                currentSection = line.substr(1, line.length() - 2);
                currentSection = trim(currentSection);
                LOG_TRACE("Found section: [{}]", currentSection);
                continue;
            }
            
            // Clé=Valeur
            size_t equalPos = line.find('=');
            if (equalPos != std::string::npos) {
                std::string key = trim(line.substr(0, equalPos));
                std::string value = trim(line.substr(equalPos + 1));
                
                if (currentSection.empty()) {
                    LOG_WARN("Key '{}' found outside of any section at line {}", key, lineNumber);
                    continue;
                }
                
                m_iniData[currentSection][key] = value;
                LOG_TRACE("Config: [{}] {} = {}", currentSection, key, value);
            } else {
                LOG_WARN("Invalid line format at line {}: {}", lineNumber, line);
            }
        }
        
        file.close();
        return true;
    }

    void ConfigManager::loadDisplaySection() {
        m_displayConfig.width = stringToUint(getValue("Display", "width", "1920"));
        m_displayConfig.height = stringToUint(getValue("Display", "height", "1080"));
        m_displayConfig.fullscreen = stringToBool(getValue("Display", "fullscreen", "false"));
        m_displayConfig.vsync = stringToBool(getValue("Display", "vsync", "true"));
        m_displayConfig.frameRateLimit = stringToUint(getValue("Display", "frameRateLimit", "60"));
        m_displayConfig.antialiasingLevel = stringToUint(getValue("Display", "antialiasingLevel", "0"));
        m_displayConfig.nativeWidth = stringToUint(getValue("Display", "nativeWidth", "3840"));
        m_displayConfig.nativeHeight = stringToUint(getValue("Display", "nativeHeight", "2160"));
        
        LOG_DEBUG("Display config loaded: {}x{} (fullscreen: {}, vsync: {})",
                 m_displayConfig.width, m_displayConfig.height,
                 m_displayConfig.fullscreen, m_displayConfig.vsync);
    }

    void ConfigManager::loadAudioSection() {
        m_audioConfig.masterVolume = stringToFloat(getValue("Audio", "masterVolume", "100.0"));
        m_audioConfig.musicVolume = stringToFloat(getValue("Audio", "musicVolume", "80.0"));
        m_audioConfig.soundVolume = stringToFloat(getValue("Audio", "soundVolume", "90.0"));
        m_audioConfig.muteAll = stringToBool(getValue("Audio", "muteAll", "false"));
        m_audioConfig.audioDevice = getValue("Audio", "audioDevice", "default");
        
        LOG_DEBUG("Audio config loaded: Master={}%, Music={}%, Sound={}%",
                 m_audioConfig.masterVolume, m_audioConfig.musicVolume, m_audioConfig.soundVolume);
    }

    void ConfigManager::loadInputSection() {
        m_inputConfig.mouseSensitivity = stringToFloat(getValue("Input", "mouseSensitivity", "1.0"));
        m_inputConfig.invertMouse = stringToBool(getValue("Input", "invertMouse", "false"));
        
        // Charger les key bindings
        m_inputConfig.keyBindings.clear();
        
        struct DefaultBinding {
            std::string action;
            std::string keys;
        };
        
        std::vector<DefaultBinding> defaultBindings = {
            {"moveUp", "W,Up"},
            {"moveDown", "S,Down"},
            {"moveLeft", "A,Left"},
            {"moveRight", "D,Right"},
            {"interact", "E,Enter,Space"},
            {"menu", "Escape,M"}
        };
        
        for (const auto& binding : defaultBindings) {
            std::string keys = getValue("Input", binding.action, binding.keys);
            InputConfig::KeyBinding keyBinding;
            keyBinding.action = binding.action;
            keyBinding.keys = stringToVector(keys);
            m_inputConfig.keyBindings.push_back(keyBinding);
        }
        
        LOG_DEBUG("Input config loaded: {} key bindings, mouse sensitivity: {}",
                 m_inputConfig.keyBindings.size(), m_inputConfig.mouseSensitivity);
    }

    void ConfigManager::loadDebugSection() {
        m_debugConfig.enableLogging = stringToBool(getValue("Debug", "enableLogging", "true"));
        m_debugConfig.logLevel = getValue("Debug", "logLevel", "INFO");
        m_debugConfig.logFile = getValue("Debug", "logFile", "logs/nova.log");
        m_debugConfig.showFPS = stringToBool(getValue("Debug", "showFPS", "false"));
        m_debugConfig.showDebugInfo = stringToBool(getValue("Debug", "showDebugInfo", "false"));
        m_debugConfig.enableProfiler = stringToBool(getValue("Debug", "enableProfiler", "false"));
        
        LOG_DEBUG("Debug config loaded: Logging={}, Level={}, ShowFPS={}",
                 m_debugConfig.enableLogging, m_debugConfig.logLevel, m_debugConfig.showFPS);
    }

    void ConfigManager::loadGameSection() {
        m_gameConfig.language = getValue("Game", "language", "en");
        m_gameConfig.playerName = getValue("Game", "playerName", "Player");
        m_gameConfig.autoSave = stringToBool(getValue("Game", "autoSave", "true"));
        m_gameConfig.autoSaveInterval = stringToUint(getValue("Game", "autoSaveInterval", "300"));
        m_gameConfig.savePath = getValue("Game", "savePath", "saves/");
        
        LOG_DEBUG("Game config loaded: Language={}, Player={}, AutoSave={}",
                 m_gameConfig.language, m_gameConfig.playerName, m_gameConfig.autoSave);
    }

    void ConfigManager::saveDisplaySection(std::ofstream& file) const {
        file << "[Display]\n";
        file << "width=" << m_displayConfig.width << "\n";
        file << "height=" << m_displayConfig.height << "\n";
        file << "fullscreen=" << boolToString(m_displayConfig.fullscreen) << "\n";
        file << "vsync=" << boolToString(m_displayConfig.vsync) << "\n";
        file << "frameRateLimit=" << m_displayConfig.frameRateLimit << "\n";
        file << "antialiasingLevel=" << m_displayConfig.antialiasingLevel << "\n";
        file << "nativeWidth=" << m_displayConfig.nativeWidth << "\n";
        file << "nativeHeight=" << m_displayConfig.nativeHeight << "\n\n";
    }

    void ConfigManager::saveAudioSection(std::ofstream& file) const {
        file << "[Audio]\n";
        file << "masterVolume=" << m_audioConfig.masterVolume << "\n";
        file << "musicVolume=" << m_audioConfig.musicVolume << "\n";
        file << "soundVolume=" << m_audioConfig.soundVolume << "\n";
        file << "muteAll=" << boolToString(m_audioConfig.muteAll) << "\n";
        file << "audioDevice=" << m_audioConfig.audioDevice << "\n\n";
    }

    void ConfigManager::saveInputSection(std::ofstream& file) const {
        file << "[Input]\n";
        file << "mouseSensitivity=" << m_inputConfig.mouseSensitivity << "\n";
        file << "invertMouse=" << boolToString(m_inputConfig.invertMouse) << "\n";
        
        for (const auto& binding : m_inputConfig.keyBindings) {
            file << binding.action << "=" << vectorToString(binding.keys) << "\n";
        }
        file << "\n";
    }

    void ConfigManager::saveDebugSection(std::ofstream& file) const {
        file << "[Debug]\n";
        file << "enableLogging=" << boolToString(m_debugConfig.enableLogging) << "\n";
        file << "logLevel=" << m_debugConfig.logLevel << "\n";
        file << "logFile=" << m_debugConfig.logFile << "\n";
        file << "showFPS=" << boolToString(m_debugConfig.showFPS) << "\n";
        file << "showDebugInfo=" << boolToString(m_debugConfig.showDebugInfo) << "\n";
        file << "enableProfiler=" << boolToString(m_debugConfig.enableProfiler) << "\n\n";
    }

    void ConfigManager::saveGameSection(std::ofstream& file) const {
        file << "[Game]\n";
        file << "language=" << m_gameConfig.language << "\n";
        file << "playerName=" << m_gameConfig.playerName << "\n";
        file << "autoSave=" << boolToString(m_gameConfig.autoSave) << "\n";
        file << "autoSaveInterval=" << m_gameConfig.autoSaveInterval << "\n";
        file << "savePath=" << m_gameConfig.savePath << "\n\n";
    }

    bool ConfigManager::validateDisplayConfig() const {
        if (m_displayConfig.width < 640 || m_displayConfig.width > 7680) {
            LOG_ERROR("Invalid display width: {}", m_displayConfig.width);
            return false;
        }
        if (m_displayConfig.height < 480 || m_displayConfig.height > 4320) {
            LOG_ERROR("Invalid display height: {}", m_displayConfig.height);
            return false;
        }
        if (m_displayConfig.frameRateLimit > 300) {
            LOG_WARN("Very high frame rate limit: {}", m_displayConfig.frameRateLimit);
        }
        return true;
    }

    bool ConfigManager::validateAudioConfig() const {
        return m_audioConfig.masterVolume >= 0.0f && m_audioConfig.masterVolume <= 100.0f &&
               m_audioConfig.musicVolume >= 0.0f && m_audioConfig.musicVolume <= 100.0f &&
               m_audioConfig.soundVolume >= 0.0f && m_audioConfig.soundVolume <= 100.0f;
    }

    void ConfigManager::clampValues() {
        // Display
        m_displayConfig.width = std::clamp(m_displayConfig.width, 640u, 7680u);
        m_displayConfig.height = std::clamp(m_displayConfig.height, 480u, 4320u);
        m_displayConfig.frameRateLimit = std::clamp(m_displayConfig.frameRateLimit, 30u, 300u);
        m_displayConfig.antialiasingLevel = std::clamp(m_displayConfig.antialiasingLevel, 0u, 16u);
        
        // Audio
        m_audioConfig.masterVolume = std::clamp(m_audioConfig.masterVolume, 0.0f, 100.0f);
        m_audioConfig.musicVolume = std::clamp(m_audioConfig.musicVolume, 0.0f, 100.0f);
        m_audioConfig.soundVolume = std::clamp(m_audioConfig.soundVolume, 0.0f, 100.0f);
        
        // Input
        m_inputConfig.mouseSensitivity = std::clamp(m_inputConfig.mouseSensitivity, 0.1f, 5.0f);
        
        // Game
        m_gameConfig.autoSaveInterval = std::clamp(m_gameConfig.autoSaveInterval, 60u, 3600u);
    }

    void ConfigManager::debugPrint() const {
        LOG_DEBUG("=== Configuration Summary ===");
        LOG_DEBUG("Display: {}x{} (Fullscreen: {}, VSync: {})",
                 m_displayConfig.width, m_displayConfig.height,
                 m_displayConfig.fullscreen, m_displayConfig.vsync);
        LOG_DEBUG("Audio: Master={}%, Music={}%, Sound={}%",
                 m_audioConfig.masterVolume, m_audioConfig.musicVolume, m_audioConfig.soundVolume);
        LOG_DEBUG("Debug: Logging={}, Level={}, ShowFPS={}",
                 m_debugConfig.enableLogging, m_debugConfig.logLevel, m_debugConfig.showFPS);
        LOG_DEBUG("=============================");
    }

    // Helper functions
    std::string ConfigManager::trim(const std::string& str) const {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    bool ConfigManager::stringToBool(const std::string& str) const {
        std::string lower = str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower == "true" || lower == "1" || lower == "yes" || lower == "on";
    }

    f32 ConfigManager::stringToFloat(const std::string& str) const {
        try {
            return std::stof(str);
        } catch (const std::exception&) {
            LOG_WARN("Invalid float value: '{}'", str);
            return 0.0f;
        }
    }

    u32 ConfigManager::stringToUint(const std::string& str) const {
        try {
            return static_cast<u32>(std::stoul(str));
        } catch (const std::exception&) {
            LOG_WARN("Invalid uint value: '{}'", str);
            return 0;
        }
    }

    std::vector<std::string> ConfigManager::stringToVector(const std::string& str, char delimiter) const {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string item;
        
        while (std::getline(ss, item, delimiter)) {
            item = trim(item);
            if (!item.empty()) {
                result.push_back(item);
            }
        }
        
        return result;
    }

    std::string ConfigManager::boolToString(bool value) const {
        return value ? "true" : "false";
    }

    std::string ConfigManager::vectorToString(const std::vector<std::string>& vec, char delimiter) const {
        std::string result;
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) result += delimiter;
            result += vec[i];
        }
        return result;
    }

    bool ConfigManager::createDirectoryIfNotExists(const std::string& path) const {
        if (path.empty()) return true;
        
        try {
            if (!std::filesystem::exists(path)) {
                std::filesystem::create_directories(path);
                LOG_DEBUG("Created directory: {}", path);
            }
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create directory '{}': {}", path, e.what());
            return false;
        }
    }

    std::string ConfigManager::getValue(const std::string& section, const std::string& key, const std::string& defaultValue) const {
        auto sectionIt = m_iniData.find(section);
        if (sectionIt != m_iniData.end()) {
            auto keyIt = sectionIt->second.find(key);
            if (keyIt != sectionIt->second.end()) {
                return keyIt->second;
            }
        }
        return defaultValue;
    }

} // namespace NovaEngine