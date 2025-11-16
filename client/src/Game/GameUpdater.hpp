#pragma once

namespace NovaEngine {
    class SceneManager;
    class UIManager;
    class DialogueSystem;
    class PlayerController;
}

namespace Game {

/**
 * @brief Game Updater - Handles all game update logic
 *
 * Separates update concerns from main game loop
 */
class GameUpdater {
public:
    /**
     * @brief Update all game systems
     * @param sceneManager Scene manager to update
     * @param uiManager UI manager to update
     * @param dialogueSystem Dialogue system for state checking
     * @param playerController Player controller for movement/detection
     * @param deltaTime Time since last frame
     */
    static void update(
        NovaEngine::SceneManager& sceneManager,
        NovaEngine::UIManager& uiManager,
        const NovaEngine::DialogueSystem& dialogueSystem,
        NovaEngine::PlayerController& playerController,
        float deltaTime
    );

private:
    static void updateSceneManager(NovaEngine::SceneManager& sceneManager, float deltaTime);
    static void updatePlayer(
        NovaEngine::SceneManager& sceneManager,
        const NovaEngine::DialogueSystem& dialogueSystem,
        NovaEngine::PlayerController& playerController,
        float deltaTime
    );
    static void updateCamera(
        const NovaEngine::SceneManager& sceneManager,
        const NovaEngine::PlayerController& playerController
    );
    static void updateUI(NovaEngine::UIManager& uiManager, float deltaTime);
};

} // namespace Game
