#include "NovaEngine/Core/NovaEngine.hpp"
#include "NovaEngine/Core/ConfigManager.hpp"
#include "NovaEngine/Resources/ResourceManager.hpp"
#include "NovaEngine/Events/EventDispatcher.hpp"

namespace NovaEngine {

    NovaEngine& NovaEngine::get() {
        static NovaEngine instance;
        return instance;
    }

    NovaEngine::NovaEngine()
        : m_running(false),
          m_width(1920),
          m_height(1080),
          m_isFullscreen(false),
          m_window(nullptr),
          m_resourceManager(nullptr),
          m_eventDispatcher(nullptr)
    {
        LOG_TRACE("NovaEngine instance created");
    }

    NovaEngine::~NovaEngine() {
        LOG_TRACE("NovaEngine destroyed");
    }

    bool NovaEngine::initialize(const std::string& title, u32 width, u32 height, bool fullscreen) {
        LOG_INFO("Initializing NovaEngine");

        if (!ConfigManager::initializeGlobalConfig()) {
            LOG_WARN("Failed to load configuration, using provided parameters");
        } else {
            const auto& displayConfig = DISPLAY_CONFIG;
            width = displayConfig.width;
            height = displayConfig.height;
            fullscreen = displayConfig.fullscreen;
            LOG_INFO("Using configuration: {}x{} (fullscreen: {})", width, height, fullscreen);
        }

        m_width = width;
        m_height = height;
        m_isFullscreen = fullscreen;

        sf::VideoMode videoMode(width, height);
        sf::Uint32 style;
        
        if (fullscreen) {
            // Borderless fullscreen window
            style = sf::Style::None;
            // Utiliser la résolution native pour éviter les problèmes de scaling
            auto desktopMode = sf::VideoMode::getDesktopMode();
            videoMode = sf::VideoMode(desktopMode.width, desktopMode.height);
            LOG_INFO("Creating borderless fullscreen: {}x{}", videoMode.width, videoMode.height);
        } else {
            style = sf::Style::Close | sf::Style::Titlebar;
            LOG_INFO("Creating windowed mode: {}x{}", width, height);
        }

        m_window = std::make_unique<sf::RenderWindow>(videoMode, title, style);
        
        if (fullscreen) {
            m_window->setPosition(sf::Vector2i(0, 0));
        }
        if (!m_window || !m_window->isOpen()) {
            LOG_FATAL("Failed to create SFML window");
            return false;
        }

        const auto& displayConfig = DISPLAY_CONFIG;
        m_window->setVerticalSyncEnabled(displayConfig.vsync);
        
        if (!displayConfig.vsync && displayConfig.frameRateLimit > 0) {
            m_window->setFramerateLimit(displayConfig.frameRateLimit);
        }

        LOG_DEBUG("SFML Window created: {}x{} ({})", width, height, 
                 fullscreen ? "fullscreen" : "windowed");

        m_resourceManager = std::make_unique<ResourceManager>();
        m_eventDispatcher = std::make_unique<EventDispatcher>();

        const auto& debugConfig = DEBUG_CONFIG;
        if (debugConfig.enableLogging && !debugConfig.logFile.empty()) {
            Logger::getInstance().setLogFile(debugConfig.logFile);
            
            LogLevel logLevel = LogLevel::Info;
            if (debugConfig.logLevel == "TRACE") logLevel = LogLevel::Trace;
            else if (debugConfig.logLevel == "DEBUG") logLevel = LogLevel::Debug;
            else if (debugConfig.logLevel == "INFO") logLevel = LogLevel::Info;
            else if (debugConfig.logLevel == "WARN") logLevel = LogLevel::Warning;
            else if (debugConfig.logLevel == "ERROR") logLevel = LogLevel::Error;
            else if (debugConfig.logLevel == "FATAL") logLevel = LogLevel::Fatal;
            
            Logger::getInstance().setLogLevel(logLevel);
            LOG_DEBUG("Logging configured: Level={}, File={}", debugConfig.logLevel, debugConfig.logFile);
        }

        m_running = true;
        LOG_INFO("NovaEngine initialized successfully");
        return true;
    }

    bool NovaEngine::initializeWithConfig(const std::string& title, const std::string& configPath) {
        LOG_INFO("Initializing NovaEngine with config file: {}", configPath);

        if (!ConfigManager::initializeGlobalConfig(configPath)) {
            LOG_ERROR("Failed to load configuration from: {}", configPath);
            return false;
        }

        const auto& displayConfig = DISPLAY_CONFIG;
        return initialize(title, displayConfig.width, displayConfig.height, displayConfig.fullscreen);
    }

    void NovaEngine::shutdown() {
        if (!m_running) return;
        LOG_INFO("Shutting down NovaEngine");

        m_running = false;

        if (m_window && m_window->isOpen()) {
            m_window->close();
        }

        const auto& config = ConfigManager::getInstance();
        if (!config.saveToFile()) {
            LOG_WARN("Failed to save configuration on shutdown");
        }

        m_resourceManager.reset();
        m_eventDispatcher.reset();
        m_window.reset();

        LOG_INFO("NovaEngine shutdown complete");
    }

    void NovaEngine::applyDisplaySettings(const DisplayConfig& config) {
        if (!m_window) {
            LOG_ERROR("Cannot apply display settings: window not created");
            return;
        }

        sf::VideoMode currentMode = sf::VideoMode(m_width, m_height);
        sf::VideoMode newMode(config.width, config.height);
        sf::Uint32 style;
        
        if (config.fullscreen) {
            style = sf::Style::None;
            auto desktopMode = sf::VideoMode::getDesktopMode();
            newMode = sf::VideoMode(desktopMode.width, desktopMode.height);
        } else {
            style = sf::Style::Close | sf::Style::Titlebar;
        }
        
        if (currentMode.width != newMode.width || 
            currentMode.height != newMode.height ||
            m_isFullscreen != config.fullscreen) {
            
            LOG_INFO("Recreating window with new settings: {}x{} (fullscreen: {})",
                    config.width, config.height, config.fullscreen);
            
            m_window->create(newMode, "NovaEngine", style);
            
            if (config.fullscreen) {
                m_window->setPosition(sf::Vector2i(0, 0));
            }
            
            m_width = config.width;
            m_height = config.height;
            m_isFullscreen = config.fullscreen;
        }

        m_window->setVerticalSyncEnabled(config.vsync);
        
        if (!config.vsync && config.frameRateLimit > 0) {
            m_window->setFramerateLimit(config.frameRateLimit);
        } else if (!config.vsync) {
            m_window->setFramerateLimit(0);
        }

        LOG_DEBUG("Display settings applied successfully");
    }

    const DisplayConfig& NovaEngine::getDisplayConfig() const {
        return DISPLAY_CONFIG;
    }

    void NovaEngine::setDisplayConfig(const DisplayConfig& config) {
        ConfigManager::getInstance().getDisplayConfig() = config;
        applyDisplaySettings(config);
    }

    sf::RenderWindow& NovaEngine::getWindow() {
        return *m_window;
    }

    ResourceManager& NovaEngine::getResourceManager() {
        return *m_resourceManager;
    }

    EventDispatcher& NovaEngine::getEventDispatcher() {
        return *m_eventDispatcher;
    }

    bool NovaEngine::isRunning() const {
        return m_running;
    }

    u32 NovaEngine::getWidth() const {
        return m_width;
    }

    u32 NovaEngine::getHeight() const {
        return m_height;
    }

    bool NovaEngine::isFullscreen() const {
        return m_isFullscreen;
    }

    void NovaEngine::toggleFullscreen() {
        auto& displayConfig = ConfigManager::getInstance().getDisplayConfig();
        displayConfig.fullscreen = !displayConfig.fullscreen;
        applyDisplaySettings(displayConfig);
        LOG_INFO("Toggled fullscreen: {}", displayConfig.fullscreen ? "ON" : "OFF");
    }

    void NovaEngine::setTitle(const std::string& title) {
        if (m_window) {
            m_window->setTitle(title);
            LOG_DEBUG("Window title changed to: {}", title);
        }
    }

} // namespace NovaEngine