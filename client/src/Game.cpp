#include <NovaEngine/Game.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>
#include <NovaEngine/Core/Logger.hpp>

// Game modules
#include "Game/GameConfigLoader.hpp"
#include "Game/GameInitializer.hpp"
#include "Game/GameUpdater.hpp"
#include "Game/GameRenderer.hpp"
#include "Game/GameEventHandler.hpp"
#include "Dialogue/DialogueSystem.hpp"
#include "Player/PlayerController.hpp"

// ============================================================================
// Constructor / Destructor
// ============================================================================

Game::Game()
    : Application(Game::GameConfigLoader::createConfig())
    , m_isConnected(false)
    , m_dialogueSystem(std::make_unique<NovaEngine::DialogueSystem>())
    , m_playerController(std::make_unique<NovaEngine::PlayerController>())
    , m_font(INVALID_HANDLE)
{
    LOG_INFO("Game instance created");
}

Game::~Game() {
    LOG_INFO("Game instance destroyed");
}

// ============================================================================
// Lifecycle Hooks
// ============================================================================

bool Game::onInitialize() {
    using namespace Game;

    // Delegate all initialization to GameInitializer
    auto result = GameInitializer::initialize(
        m_sceneManager,
        m_uiManager,
        *m_dialogueSystem,
        *m_playerController
    );

    // Store font handle for rendering
    m_font = result.font;

    // Check initialization result
    if (!result.success) {
        LOG_ERROR("Game initialization failed: {}", result.errorMessage);
        return false;
    }

    return true;
}

void Game::onUpdate(float deltaTime) {
    using namespace Game;

    // Delegate all update logic to GameUpdater
    GameUpdater::update(
        m_sceneManager,
        m_uiManager,
        *m_dialogueSystem,
        *m_playerController,
        deltaTime
    );
}

void Game::onRender() {
    using namespace Game;

    // Delegate all rendering to GameRenderer
    GameRenderer::render(
        m_sceneManager,
        m_uiManager,
        *m_dialogueSystem,
        *m_playerController,
        m_font
    );
}

void Game::onEvent(const NovaEngine::Event& event) {
    using namespace Game;

    // Delegate event handling to GameEventHandler
    bool shouldQuit = GameEventHandler::handleEvent(
        event,
        *m_dialogueSystem,
        *m_playerController
    );

    if (shouldQuit) {
        quit();
    }
}

void Game::onShutdown() {
    LOG_INFO("Game shutting down");
}

// ============================================================================
// UI Actions (Original Game Methods)
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
    LOG_INFO("Connection state: {}", m_isConnected ? "Connected" : "Disconnected");
}

bool Game::isConnected() const {
    return m_isConnected;
}
