#pragma once

#include <NovaEngine/Core/Types.hpp>
#include <NovaEngine/Backend/Core/BackendTypes.hpp>
#include <string>

namespace NovaEngine {
    class SceneManager;
    class UIManager;
    class DialogueSystem;
    class PlayerController;
}

namespace Game {

/**
 * @brief Game Initializer - Handles all game initialization logic
 *
 * Centralizes initialization to keep Game.cpp clean and focused on orchestration
 */
class GameInitializer {
public:
    struct InitResult {
        bool success = false;
        std::string errorMessage;
        NovaEngine::FontHandle font = INVALID_HANDLE;
        NovaEngine::u64 playerID = 0;
    };

    /**
     * @brief Initialize all game systems
     * @param sceneManager Scene manager to initialize
     * @param uiManager UI manager to initialize
     * @param dialogueSystem Dialogue system to initialize
     * @param playerController Player controller to initialize
     * @return InitResult with success status and loaded resources
     */
    static InitResult initialize(
        NovaEngine::SceneManager& sceneManager,
        NovaEngine::UIManager& uiManager,
        NovaEngine::DialogueSystem& dialogueSystem,
        NovaEngine::PlayerController& playerController
    );

private:
    static NovaEngine::FontHandle loadFont();
    static bool initializeSceneManager(NovaEngine::SceneManager& sceneManager);
    static bool loadGameScenes(NovaEngine::SceneManager& sceneManager);
    static NovaEngine::u64 createPlayer(NovaEngine::SceneManager& sceneManager);
    static void printControls();
};

} // namespace Game
