#pragma once

#include <NovaEngine/Core/Application.hpp>

namespace Game {

/**
 * @brief Game Config Loader - Loads game configuration
 *
 * Handles loading configuration from file and providing defaults
 */
class GameConfigLoader {
public:
    /**
     * @brief Create application configuration
     * @return Application config structure
     */
    static NovaEngine::Application::Config createConfig();

private:
    static NovaEngine::Application::Config loadFromFile();
    static NovaEngine::Application::Config getDefaultConfig();
    static void logConfigurationLoaded(bool fromFile);
};

} // namespace Game
