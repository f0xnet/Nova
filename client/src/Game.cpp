#include <NovaEngine/Game.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>
#include <NovaEngine/Core/ConfigManager.hpp>
#include "Dialogue/DialogueSystem.hpp"
#include "Dialogue/DialogueComponent.hpp"
#include "Player/PlayerController.hpp"
#include <cmath>

// ============================================================================
// Game Constructor / Destructor
// ============================================================================

Game::Game()
    : Application(createConfig())
    , m_isConnected(false)
    , m_dialogueSystem(std::make_unique<NovaEngine::DialogueSystem>())
    , m_playerController(std::make_unique<NovaEngine::PlayerController>())
{
    LOG_INFO("Game instance created");
}

Game::~Game() {
    LOG_INFO("Game instance destroyed");
}

// ============================================================================
// Configuration
// ============================================================================

NovaEngine::Application::Config Game::createConfig() {
    using namespace NovaEngine;

    Config config;
    ConfigManager& configMgr = ConfigManager::getInstance();

    // Load configuration from file
    if (configMgr.loadFromFile("config.json")) {
        config.windowTitle = configMgr.getString("window.title", "NovaEngine - Simple RPG");
        config.windowWidth = configMgr.getInt("window.width", 1280);
        config.windowHeight = configMgr.getInt("window.height", 720);
        config.fullscreen = configMgr.getBool("window.fullscreen", false);
        config.vSync = configMgr.getBool("window.vsync", true);
        config.frameRateLimit = configMgr.getInt("window.fps_limit", 60);

        // Get clear color from config
        auto colorArray = configMgr.getIntArray("window.clear_color");
        if (colorArray.size() >= 3) {
            config.clearColor = Color(
                static_cast<u8>(colorArray[0]),
                static_cast<u8>(colorArray[1]),
                static_cast<u8>(colorArray[2]),
                colorArray.size() >= 4 ? static_cast<u8>(colorArray[3]) : 255
            );
        } else {
            config.clearColor = Color(30, 30, 40); // Dark blue-gray default
        }

        LOG_INFO("Configuration loaded from config.json");
    } else {
        // Default configuration
        config.windowTitle = "NovaEngine - Simple RPG";
        config.windowWidth = 1280;
        config.windowHeight = 720;
        config.fullscreen = false;
        config.vSync = true;
        config.frameRateLimit = 60;
        config.clearColor = Color(30, 30, 40);

        LOG_WARN("Could not load config.json, using defaults");
    }

    return config;
}

// ============================================================================
// Game Lifecycle
// ============================================================================

bool Game::onInitialize() {
    using namespace NovaEngine;

    LOG_INFO("=== Initializing Simple RPG Game ===");

    // Load font
    FontHandle font = RESOURCES().loadFont("data/fonts/arial.ttf");
    if (font == INVALID_HANDLE) {
        LOG_ERROR("Failed to load font - trying alternative path");
        font = RESOURCES().loadFont("data/arial.ttf");
        if (font == INVALID_HANDLE) {
            LOG_ERROR("Failed to load font from both paths!");
            return false;
        }
    }

    // Initialize game modules
    m_dialogueSystem->initialize(font);
    LOG_INFO("Dialogue System initialized");

    // Initialize UI Manager
    m_uiManager.initialize(font);
    LOG_INFO("UI Manager initialized");

    // Initialize Scene Manager
    if (!m_sceneManager.initialize("data/definitions/", "data/scenegraph.json")) {
        LOG_ERROR("Failed to initialize Scene Manager");
        return false;
    }
    LOG_INFO("Scene Manager initialized");

    // Load initial scene (ville/village)
    if (!m_sceneManager.loadScene("data/scenes/ville.json", "ville")) {
        LOG_ERROR("Failed to load initial scene 'ville'");
        return false;
    }

    // Set active scene
    m_sceneManager.setActiveScene("ville");
    Scene* ville = m_sceneManager.getActiveScene();
    if (!ville) {
        LOG_ERROR("Failed to activate scene 'ville'");
        return false;
    }

    // Create player entity
    Entity* player = ville->getEntityRegistry().createEntity();

    // Add Transform component
    auto* transform = player->addComponent(std::make_unique<TransformComponent>());
    transform->position = Vec2f(640, 360);

    // Add Sprite component
    auto* sprite = player->addComponent(std::make_unique<SpriteComponent>());
    sprite->textureHandle = RESOURCES().loadTexture("data/sprites/player.png");
    sprite->size = Vec2f(64, 64);
    sprite->tint = Color(100, 200, 255); // Light blue for player

    // Register player with PlayerController
    m_playerController->setPlayerID(player->getID());

    LOG_INFO("Player entity created at ({}, {})", transform->position.x, transform->position.y);

    LOG_INFO("=== Game Initialized Successfully ===");
    LOG_INFO("Controls:");
    LOG_INFO("  WASD / Arrow Keys - Move");
    LOG_INFO("  E - Talk to NPCs / Advance dialogue");
    LOG_INFO("  ESC - Quit");

    return true;
}

void Game::onUpdate(float deltaTime) {
    using namespace NovaEngine;

    Scene* scene = m_sceneManager.getActiveScene();
    if (!scene) return;

    // Update Scene Manager
    m_sceneManager.update(deltaTime);

    // Update Player Movement (only when dialogue is not active)
    m_playerController->updateMovement(scene, deltaTime, !m_dialogueSystem->isActive());

    // Update NPC Detection
    m_playerController->updateNPCDetection(scene);

    // Update camera to follow player
    Vec2f playerPos = m_playerController->getPlayerPosition(scene);
    VIEWPORT().setViewCenter(playerPos);

    // Update UI Manager
    m_uiManager.update(deltaTime);
}

void Game::onRender() {
    using namespace NovaEngine;

    // Render scene entities
    renderScene();

    // Render NPC interaction indicator
    renderNPCIndicator();

    // Render dialogue if active
    if (m_dialogueSystem->isActive()) {
        Vec2f viewCenter = VIEWPORT().getViewCenter();
        Vec2f viewSize = VIEWPORT().getViewSize();
        m_dialogueSystem->render(viewCenter, viewSize);
    }

    // Render UI
    m_uiManager.render();
}

void Game::onEvent(const NovaEngine::Event& event) {
    using namespace NovaEngine;

    if (event.type != EventType::Input) return;

    const InputEvent& inputEvent = event.inputEvent;

    // Handle ESC to quit
    if (inputEvent.type == InputEventType::KeyPressed &&
        inputEvent.key.code == KeyCode::Escape) {
        quit();
        return;
    }

    // Handle E key for dialogue
    if (inputEvent.type == InputEventType::KeyPressed &&
        inputEvent.key.code == KeyCode::E) {

        if (m_dialogueSystem->isActive()) {
            // Advance dialogue
            m_dialogueSystem->advanceDialogue();
        } else {
            // Start dialogue with nearest NPC
            Entity* nearestNPC = m_playerController->getNearestNPC();
            if (nearestNPC) {
                m_dialogueSystem->startDialogue(nearestNPC);
            }
        }
    }
}

void Game::onShutdown() {
    LOG_INFO("Game shutting down");
}

// ============================================================================
// Game Methods
// ============================================================================

void Game::handleUIAction(const std::string& action,
                          const std::string& value,
                          const NovaEngine::ID& componentID)
{
    using namespace NovaEngine;

    LOG_INFO("UI Action: {} - Value: {}", action, value);

    if (action == "connect") {
        toggleConnectionState();
    }
}

void Game::toggleConnectionState() {
    m_isConnected = !m_isConnected;
    LOG_INFO("Connection state toggled: {}", m_isConnected ? "Connected" : "Disconnected");
}

bool Game::isConnected() const {
    return m_isConnected;
}

// ============================================================================
// Rendering Helper Methods
// ============================================================================

void Game::renderScene() {
    using namespace NovaEngine;

    Scene* scene = m_sceneManager.getActiveScene();
    if (!scene) return;

    // Render all visible sprites in the scene
    for (auto* entity : scene->getEntityRegistry().getAllEntities()) {
        auto* transform = entity->getComponent<TransformComponent>();
        auto* sprite = entity->getComponent<SpriteComponent>();

        if (transform && sprite && sprite->visible && sprite->textureHandle != INVALID_HANDLE) {
            SpriteData spriteData;
            spriteData.texture = sprite->textureHandle;
            spriteData.position = transform->position;
            spriteData.size = sprite->size;
            spriteData.rotation = transform->rotation;
            spriteData.origin = transform->origin;
            spriteData.color = sprite->tint;
            spriteData.textureRect = sprite->textureRect;

            GRAPHICS().drawSprite(spriteData);
        }
    }
}

void Game::renderNPCIndicator() {
    using namespace NovaEngine;

    // Don't show indicator during dialogue
    if (m_dialogueSystem->isActive()) return;

    Entity* nearestNPC = m_playerController->getNearestNPC();
    if (!nearestNPC) return;

    auto* npcTransform = nearestNPC->getComponent<TransformComponent>();
    if (!npcTransform) return;

    // Draw "[E]" indicator above NPC
    TextData indicator;
    indicator.text = "[ E ]";
    indicator.font = RESOURCES().loadFont("data/fonts/arial.ttf");
    indicator.characterSize = 20;
    indicator.fillColor = Color::Yellow;
    indicator.position = Vec2f(
        npcTransform->position.x - 20,
        npcTransform->position.y - 50
    );
    GRAPHICS().drawText(indicator);
}
