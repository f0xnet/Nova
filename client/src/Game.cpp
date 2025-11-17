#include "NovaEngine/Game.hpp"
#include "NovaEngine/Backend/BackendManager.hpp"
#include "NovaEngine/Core/ConfigManager.hpp"
#include "Dialogue/DialogueSystem.hpp"
#include "Player/PlayerController.hpp"

Game::Game()
    : Application(createConfig())
    , m_isConnected(false)
    , m_dialogueSystem(std::make_unique<NovaEngine::DialogueSystem>())
    , m_playerController(std::make_unique<NovaEngine::PlayerController>())
{
    LOG_TRACE("Game constructed");
}

Game::~Game() {
    LOG_TRACE("Game destroyed");
}

bool Game::onInitialize() {
    LOG_INFO("Initializing Game");

    // Configure viewport with letterboxing to preserve aspect ratio
    const auto& displayConfig = NovaEngine::ConfigManager::getInstance().getDisplayConfig();
    float logicalWidth = static_cast<float>(displayConfig.nativeWidth);
    float logicalHeight = static_cast<float>(displayConfig.nativeHeight);
    float logicalAspectRatio = logicalWidth / logicalHeight;

    // Get actual window size
    NovaEngine::u32 windowWidth = WINDOW().getWidth();
    NovaEngine::u32 windowHeight = WINDOW().getHeight();
    float windowAspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    // Calculate viewport with letterboxing/pillarboxing
    NovaEngine::Rect viewportRect;
    if (windowAspectRatio > logicalAspectRatio) {
        // Pillarbox (vertical bars on sides)
        float viewportWidth = logicalAspectRatio / windowAspectRatio;
        float offsetX = (1.0f - viewportWidth) / 2.0f;
        viewportRect = NovaEngine::Rect(offsetX, 0.0f, viewportWidth, 1.0f);
    } else {
        // Letterbox (horizontal bars top/bottom)
        float viewportHeight = windowAspectRatio / logicalAspectRatio;
        float offsetY = (1.0f - viewportHeight) / 2.0f;
        viewportRect = NovaEngine::Rect(0.0f, offsetY, 1.0f, viewportHeight);
    }

    NovaEngine::ViewportData viewData;
    viewData.size = NovaEngine::Vec2f(logicalWidth, logicalHeight);
    viewData.center = NovaEngine::Vec2f(logicalWidth / 2.0f, logicalHeight / 2.0f);
    viewData.viewport = viewportRect;
    viewData.rotation = 0.0f;
    VIEWPORT().setView(viewData);

    LOG_INFO("Viewport: {}x{} logical, letterboxed on {}x{} window",
             logicalWidth, logicalHeight, windowWidth, windowHeight);

    // Initialize ECS Scene Manager
    if (!m_sceneManager.initialize("data/definitions/", "data/scenegraph.json")) {
        LOG_ERROR("Failed to initialize SceneManager");
        return false;
    }

    // Load game scene
    if (!m_sceneManager.loadScene("data/scenes/test.json", "test")) {
        LOG_ERROR("Failed to load test scene");
        return false;
    }
    m_sceneManager.setActiveScene("test");
    LOG_INFO("Test scene loaded and activated");

    // Player is defined in the scene JSON (test.json)
    // Find the first sprite entity and use it as the player
    NovaEngine::Scene* scene = m_sceneManager.getActiveScene();
    if (scene) {
        using namespace NovaEngine;
        auto entities = scene->getEntityRegistry().getAllEntities();
        for (auto* entity : entities) {
            if (entity->getComponent<SpriteComponent>()) {
                m_playerController->setPlayerID(entity->getID());
                LOG_INFO("Player set to entity ID: {}", entity->getID());
                break;
            }
        }
    }

    // Initialize UI system
    m_uiManager.setActionCallback([this](const std::string& action,
                                         const std::string& value,
                                         const NovaEngine::ID& componentID) {
        handleUIAction(action, value, componentID);
    });

    // Load dialogue UI
    if (!m_uiLoader.loadFromFile("data/ui/json/dialogue.json", m_uiManager)) {
        LOG_WARN("Failed to load dialogue UI");
    }

    // Initialize dialogue system with UI manager
    m_dialogueSystem->initialize(&m_uiManager);

    LOG_INFO("Game initialized successfully");
    LOG_INFO("=== Controls ===");
    LOG_INFO("  WASD / Arrow Keys - Move");
    LOG_INFO("  E - Talk to NPCs / Advance dialogue");
    LOG_INFO("  ESC - Quit");

    return true;
}

void Game::onUpdate(float deltaTime) {
    using namespace NovaEngine;

    Scene* scene = m_sceneManager.getActiveScene();
    if (scene) {
        // Update player movement (disabled during dialogue)
        m_playerController->updateMovement(scene, deltaTime, !m_dialogueSystem->isActive());

        // Update NPC detection
        m_playerController->updateNPCDetection(scene);

        // Update camera to follow player
        Vec2f playerPos = m_playerController->getPlayerPosition(scene);
        VIEWPORT().setViewCenter(playerPos);

        // Show/hide NPC indicator
        Entity* nearestNPC = m_playerController->getNearestNPC();
        m_dialogueSystem->showNPCIndicator(nearestNPC != nullptr && !m_dialogueSystem->isActive());
    }

    // Update ECS scene
    m_sceneManager.update(deltaTime);

    // Update UI
    m_uiManager.update(deltaTime);
}

void Game::onRender() {
    // Render ECS scene
    m_sceneManager.render();

    // Render UI (includes dialogue)
    m_uiManager.render();
}

void Game::onEvent(const NovaEngine::Event& event) {
    using namespace NovaEngine;

    // Dispatch to UI first
    m_uiManager.dispatchEvent(event);

    // Handle game-specific input
    if (event.type == EventType::Input &&
        event.inputEvent.type == InputEventType::KeyPressed) {

        if (event.inputEvent.key.code == KeyCode::Escape) {
            quit();
        }
        else if (event.inputEvent.key.code == KeyCode::E) {
            // Handle dialogue interaction
            if (m_dialogueSystem->isActive()) {
                m_dialogueSystem->advanceDialogue();
            } else {
                Entity* nearestNPC = m_playerController->getNearestNPC();
                if (nearestNPC) {
                    m_dialogueSystem->startDialogue(nearestNPC);
                }
            }
        }
    }
}

void Game::onShutdown() {
    LOG_INFO("Game shutting down");
    m_sceneManager.shutdown();
}

Game::Config Game::createConfig() {
    Config config;

    if (!NovaEngine::ConfigManager::initializeGlobalConfig("config/engine.ini")) {
        LOG_WARN("Failed to load config file, using default values");

        config.windowTitle = "NovaEngine - RPG Game";
        config.windowWidth = 1280;
        config.windowHeight = 720;
        config.fullscreen = false;
        config.frameRateLimit = 60;
        config.vSync = true;
        config.clearColor = NovaEngine::Color(30, 30, 40);
    } else {
        const auto& displayConfig = NovaEngine::ConfigManager::getInstance().getDisplayConfig();

        config.windowTitle = "NovaEngine - RPG Game";
        config.windowWidth = displayConfig.width;
        config.windowHeight = displayConfig.height;
        config.fullscreen = displayConfig.fullscreen;
        config.frameRateLimit = displayConfig.frameRateLimit;
        config.vSync = displayConfig.vsync;
        config.clearColor = NovaEngine::Color(30, 30, 40);

        LOG_INFO("Configuration loaded: {}x{}", displayConfig.width, displayConfig.height);
    }

    return config;
}

void Game::handleUIAction(const std::string& action,
                         const std::string& value,
                         const NovaEngine::ID& componentID)
{
    LOG_INFO("UI Action: '{}' (value: '{}', component: '{}')", action, value, componentID);

    if (action == "connect") {
        toggleConnectionState();
    }
}

void Game::toggleConnectionState() {
    m_isConnected = !m_isConnected;
    LOG_INFO("Connection state: {}", m_isConnected ? "Connected" : "Disconnected");
}

bool Game::isConnected() const {
    return m_isConnected;
}
