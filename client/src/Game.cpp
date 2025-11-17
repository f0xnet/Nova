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

    // Configure viewport with 16:9 logical resolution
    // This is the standard approach for indie games - we only support 16:9
    // Sprites will scale to window size but may stretch on non-16:9 screens
    // (which is acceptable since 90% of screens are 16:9)
    const auto& displayConfig = NovaEngine::ConfigManager::getInstance().getDisplayConfig();
    float logicalWidth = static_cast<float>(displayConfig.nativeWidth);
    float logicalHeight = static_cast<float>(displayConfig.nativeHeight);

    VIEWPORT().setViewSize(NovaEngine::Vec2f(logicalWidth, logicalHeight));
    VIEWPORT().setViewCenter(NovaEngine::Vec2f(logicalWidth / 2.0f, logicalHeight / 2.0f));

    LOG_INFO("Viewport configured: 16:9 logical resolution {}x{}", logicalWidth, logicalHeight);

    // Initialize ECS Scene Manager
    if (!m_sceneManager.initialize("data/definitions/", "data/scenegraph.json")) {
        LOG_ERROR("Failed to initialize SceneManager");
        return false;
    }

    // Load game scene
    if (!m_sceneManager.loadScene("data/scenes/ville.json", "ville")) {
        LOG_WARN("Failed to load ville scene, using test scene");
        m_sceneManager.loadScene("assets/data/scenes/test_scene.json", "test");
        m_sceneManager.setActiveScene("test");
    } else {
        m_sceneManager.setActiveScene("ville");
        LOG_INFO("Ville scene loaded and activated");
    }

    // Create player in active scene
    createPlayer();

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

void Game::createPlayer() {
    using namespace NovaEngine;

    Scene* scene = m_sceneManager.getActiveScene();
    if (!scene) return;

    // Create player entity
    Entity* player = scene->getEntityRegistry().createEntity();

    // Add Transform
    auto* transform = player->addComponent(std::make_unique<TransformComponent>());
    transform->position = Vec2f(640, 360);

    // Add Sprite
    auto* sprite = player->addComponent(std::make_unique<SpriteComponent>());
    sprite->textureHandle = RESOURCES().loadTexture("data/sprites/player.png");
    if (sprite->textureHandle == INVALID_HANDLE) {
        LOG_WARN("Player sprite texture not found: data/sprites/player.png");
    } else {
        LOG_INFO("Player sprite loaded successfully (handle: {})", sprite->textureHandle);
    }

    // Set sprite size in logical coordinates (matches viewport resolution)
    // The viewport scaling system will automatically scale this to screen resolution
    sprite->size = Vec2f(114, 225);  // Size in logical pixels
    sprite->visible = true;
    sprite->zOrder = 100;

    // Register with player controller
    m_playerController->setPlayerID(player->getID());

    LOG_INFO("Player created at ({}, {})", transform->position.x, transform->position.y);
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
