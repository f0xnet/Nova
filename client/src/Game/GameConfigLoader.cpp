#include "GameConfigLoader.hpp"
#include <NovaEngine/Core/ConfigManager.hpp>
#include <NovaEngine/Core/Logger.hpp>
#include <NovaEngine/Backend/Core/BackendTypes.hpp>

namespace Game {

NovaEngine::Application::Config GameConfigLoader::createConfig() {
    // Try to load from file first
    NovaEngine::Application::Config config = loadFromFile();

    // If loading failed, use defaults
    if (config.windowTitle.empty()) {
        config = getDefaultConfig();
        logConfigurationLoaded(false);
    } else {
        logConfigurationLoaded(true);
    }

    return config;
}

NovaEngine::Application::Config GameConfigLoader::loadFromFile() {
    using namespace NovaEngine;

    Application::Config config;
    ConfigManager& configMgr = ConfigManager::getInstance();

    // Attempt to load configuration file
    if (!configMgr.loadFromFile("config.json")) {
        return config; // Return empty config on failure
    }

    // Load window settings
    config.windowTitle = configMgr.getString("window.title", "");
    if (config.windowTitle.empty()) {
        return config; // Invalid config
    }

    config.windowWidth = configMgr.getInt("window.width", 1280);
    config.windowHeight = configMgr.getInt("window.height", 720);
    config.fullscreen = configMgr.getBool("window.fullscreen", false);
    config.vSync = configMgr.getBool("window.vsync", true);
    config.frameRateLimit = configMgr.getInt("window.fps_limit", 60);

    // Load clear color
    auto colorArray = configMgr.getIntArray("window.clear_color");
    if (colorArray.size() >= 3) {
        config.clearColor = Color(
            static_cast<u8>(colorArray[0]),
            static_cast<u8>(colorArray[1]),
            static_cast<u8>(colorArray[2]),
            colorArray.size() >= 4 ? static_cast<u8>(colorArray[3]) : 255
        );
    } else {
        config.clearColor = Color(30, 30, 40);
    }

    return config;
}

NovaEngine::Application::Config GameConfigLoader::getDefaultConfig() {
    using namespace NovaEngine;

    Application::Config config;

    config.windowTitle = "NovaEngine - Game";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.fullscreen = false;
    config.vSync = true;
    config.frameRateLimit = 60;
    config.clearColor = Color(30, 30, 40);

    return config;
}

void GameConfigLoader::logConfigurationLoaded(bool fromFile) {
    using namespace NovaEngine;

    if (fromFile) {
        LOG_INFO("Configuration loaded from config.json");
    } else {
        LOG_WARN("Could not load config.json, using default configuration");
    }
}

} // namespace Game
