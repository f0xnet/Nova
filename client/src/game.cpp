#include "NovaEngine/Game.hpp"
#include "NovaEngine/Backend/BackendManager.hpp"
#include "NovaEngine/Core/ConfigManager.hpp"

Game::Game() 
    : Application(createConfig())
    , m_isConnected(false)
{
    LOG_TRACE("Game constructed");
}

Game::~Game() {
    LOG_TRACE("Game destroyed");
}

bool Game::onInitialize() {
    LOG_INFO("Initializing Game");

    // Initialize ECS Scene Manager
    if (!m_sceneManager.initialize("assets/data/definitions/")) {
        LOG_ERROR("Failed to initialize SceneManager");
        return false;
    }

    // Load scenes (if they exist)
    // Note: These files are optional for now, system will work without them
    if (m_sceneManager.loadScene("assets/data/scenes/test_scene.json", "test")) {
        m_sceneManager.setActiveScene("test");
        LOG_INFO("Test scene loaded and activated");
    } else {
        LOG_INFO("No test scene found, continuing without ECS scene");
    }

    // Initialize UI
    m_uiManager.setActionCallback([this](const std::string& action,
                                         const std::string& value,
                                         const NovaEngine::ID& componentID) {
        handleUIAction(action, value, componentID);
    });

    if (!m_uiLoader.loadUI("app_main_menu", m_uiManager)) {
        LOG_ERROR("Failed to load UI: app_main_menu");
        return false;
    }

    m_uiManager.switchToGroup("app_main_menu", "login_menu_not_connected");

    LOG_INFO("Game initialized successfully");
    LOG_INFO("=== Controls ===");
    LOG_INFO("- Click buttons to trigger actions");
    LOG_INFO("- Press SPACE to toggle between connected/not connected states");
    LOG_INFO("- Press ESC to exit");

    return true;
}

void Game::onUpdate(float deltaTime) {
    // Update ECS scene
    m_sceneManager.update(deltaTime);

    // Update UI
    m_uiManager.update(deltaTime);
}

void Game::onRender() {
    // Render ECS scene (background layer)
    m_sceneManager.render();

    // Render UI on top
    m_uiManager.render();
}

void Game::onEvent(const NovaEngine::Event& event) {
    m_uiManager.dispatchEvent(event);

    if (event.type == NovaEngine::EventType::Input && 
        event.inputEvent.type == NovaEngine::InputEventType::KeyPressed) {
        
        if (event.inputEvent.key.code == NovaEngine::KeyCode::Space) {
            toggleConnectionState();
        }
    }
}

void Game::onShutdown() {
    LOG_INFO("Game shutting down");
    m_sceneManager.shutdown();
}

Game::Config Game::createConfig() {
    Config config;
    
    // ✅ CORRECTION: Charger la configuration depuis le fichier
    if (!NovaEngine::ConfigManager::initializeGlobalConfig("config/engine.ini")) {
        LOG_WARN("Failed to load config file, using default values");
        
        // Valeurs par défaut si le fichier n'existe pas
        config.windowTitle = "NovaEngine Game";
        config.windowWidth = 1920;
        config.windowHeight = 1080;
        config.fullscreen = false;
        config.frameRateLimit = 60;
        config.vSync = true;
        config.clearColor = NovaEngine::Color::Black;
    } else {
        // ✅ Utiliser les valeurs du ConfigManager
        const auto& displayConfig = NovaEngine::ConfigManager::getInstance().getDisplayConfig();
        const auto& debugConfig = NovaEngine::ConfigManager::getInstance().getDebugConfig();
        
        config.windowTitle = "NovaEngine Game";
        config.windowWidth = displayConfig.width;
        config.windowHeight = displayConfig.height;
        config.fullscreen = displayConfig.fullscreen;
        config.frameRateLimit = displayConfig.frameRateLimit;
        config.vSync = displayConfig.vsync;
        config.clearColor = NovaEngine::Color::Black;
        
        LOG_INFO("Configuration loaded from file:");
        LOG_INFO("  - Resolution: {}x{}", displayConfig.width, displayConfig.height);
        LOG_INFO("  - Fullscreen: {}", displayConfig.fullscreen ? "YES" : "NO");
        LOG_INFO("  - VSync: {}", displayConfig.vsync ? "ON" : "OFF");
        LOG_INFO("  - FPS Limit: {}", displayConfig.frameRateLimit);
        LOG_INFO("  - Native resolution for UI scaling: {}x{}", 
                displayConfig.nativeWidth, displayConfig.nativeHeight);
    }
    
    return config;
}

void Game::handleUIAction(const std::string& action, 
                         const std::string& value, 
                         const NovaEngine::ID& componentID) 
{
    LOG_INFO("UI Action: '{}' (value: '{}', component: '{}')", action, value, componentID);

    if (action == "connect") {
        m_isConnected = true;
        m_uiManager.switchToGroup("app_main_menu", "login_menu_connected");
        LOG_INFO("Switched to connected state - login and register buttons now visible");
    }
    else if (action == "login") {
        LOG_INFO("Login button clicked - implement your login logic here");
    }
    else if (action == "register") {
        LOG_INFO("Register button clicked - implement your registration logic here");
    }
    else if (action == "quit") {
        LOG_INFO("Quit button clicked");
        quit();
    }
    else {
        LOG_WARN("Unknown action: '{}'", action);
    }
}

void Game::toggleConnectionState() {
    if (m_isConnected) {
        m_uiManager.switchToGroup("app_main_menu", "login_menu_not_connected");
        m_isConnected = false;
        LOG_INFO("Switched to not connected state");
    } else {
        m_uiManager.switchToGroup("app_main_menu", "login_menu_connected");
        m_isConnected = true;
        LOG_INFO("Switched to connected state");
    }
}

bool Game::isConnected() const {
    return m_isConnected;
}