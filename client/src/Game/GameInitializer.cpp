#include "GameInitializer.hpp"
#include "../Dialogue/DialogueSystem.hpp"
#include "../Player/PlayerController.hpp"
#include <NovaEngine/ECS/SceneManager.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <NovaEngine/UI/UIManager.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>
#include <NovaEngine/Core/Logger.hpp>

namespace Game {

GameInitializer::InitResult GameInitializer::initialize(
    NovaEngine::SceneManager& sceneManager,
    NovaEngine::UIManager& uiManager,
    NovaEngine::DialogueSystem& dialogueSystem,
    NovaEngine::PlayerController& playerController)
{
    using namespace NovaEngine;

    InitResult result;
    LOG_INFO("=== Initializing Game ===");

    // Step 1: Load font
    result.font = loadFont();
    if (result.font == INVALID_HANDLE) {
        result.errorMessage = "Failed to load font";
        return result;
    }

    // Step 2: Initialize dialogue system
    dialogueSystem.initialize(result.font);
    LOG_INFO("Dialogue System initialized");

    // Step 3: Initialize UI manager
    uiManager.initialize(result.font);
    LOG_INFO("UI Manager initialized");

    // Step 4: Initialize scene manager
    if (!initializeSceneManager(sceneManager)) {
        result.errorMessage = "Failed to initialize Scene Manager";
        return result;
    }

    // Step 5: Load game scenes
    if (!loadGameScenes(sceneManager)) {
        result.errorMessage = "Failed to load game scenes";
        return result;
    }

    // Step 6: Create player entity
    result.playerID = createPlayer(sceneManager);
    if (result.playerID == 0) {
        result.errorMessage = "Failed to create player";
        return result;
    }

    // Step 7: Register player with controller
    playerController.setPlayerID(result.playerID);
    LOG_INFO("Player controller configured");

    // Print controls
    printControls();

    result.success = true;
    LOG_INFO("=== Game Initialized Successfully ===");
    return result;
}

NovaEngine::FontHandle GameInitializer::loadFont() {
    using namespace NovaEngine;

    // Try primary path
    FontHandle font = RESOURCES().loadFont("data/fonts/arial.ttf");
    if (font != INVALID_HANDLE) {
        LOG_INFO("Font loaded: data/fonts/arial.ttf");
        return font;
    }

    // Try fallback path
    LOG_WARN("Primary font path failed, trying fallback");
    font = RESOURCES().loadFont("data/arial.ttf");
    if (font != INVALID_HANDLE) {
        LOG_INFO("Font loaded: data/arial.ttf");
        return font;
    }

    LOG_ERROR("Failed to load font from all paths");
    return INVALID_HANDLE;
}

bool GameInitializer::initializeSceneManager(NovaEngine::SceneManager& sceneManager) {
    using namespace NovaEngine;

    if (!sceneManager.initialize("data/definitions/", "data/scenegraph.json")) {
        LOG_ERROR("Scene Manager initialization failed");
        return false;
    }

    LOG_INFO("Scene Manager initialized");
    return true;
}

bool GameInitializer::loadGameScenes(NovaEngine::SceneManager& sceneManager) {
    using namespace NovaEngine;

    // Load main scene (ville/village)
    if (!sceneManager.loadScene("data/scenes/ville.json", "ville")) {
        LOG_ERROR("Failed to load scene: ville");
        return false;
    }
    LOG_INFO("Scene loaded: ville");

    // Set as active scene
    sceneManager.setActiveScene("ville");
    LOG_INFO("Active scene set: ville");

    return true;
}

NovaEngine::u64 GameInitializer::createPlayer(NovaEngine::SceneManager& sceneManager) {
    using namespace NovaEngine;

    Scene* activeScene = sceneManager.getActiveScene();
    if (!activeScene) {
        LOG_ERROR("No active scene for player creation");
        return 0;
    }

    // Create player entity
    Entity* player = activeScene->getEntityRegistry().createEntity();
    if (!player) {
        LOG_ERROR("Failed to create player entity");
        return 0;
    }

    // Add Transform component
    auto* transform = player->addComponent(std::make_unique<TransformComponent>());
    transform->position = Vec2f(640, 360);

    // Add Sprite component
    auto* sprite = player->addComponent(std::make_unique<SpriteComponent>());
    sprite->textureHandle = RESOURCES().loadTexture("data/sprites/player.png");
    sprite->size = Vec2f(64, 64);
    sprite->tint = Color(100, 200, 255); // Light blue

    LOG_INFO("Player created at ({}, {})", transform->position.x, transform->position.y);

    return player->getID();
}

void GameInitializer::printControls() {
    using namespace NovaEngine;

    LOG_INFO("=== Controls ===");
    LOG_INFO("  WASD / Arrow Keys - Move");
    LOG_INFO("  E - Talk to NPCs / Advance dialogue");
    LOG_INFO("  ESC - Quit");
}

} // namespace Game
