#pragma once

#include <NovaEngine/Backend/Core/BackendTypes.hpp>

namespace NovaEngine {
    class SceneManager;
    class UIManager;
    class DialogueSystem;
    class PlayerController;
}

namespace Game {

/**
 * @brief Game Renderer - Handles all rendering logic
 *
 * Separates rendering concerns from main game loop
 */
class GameRenderer {
public:
    /**
     * @brief Render all game elements
     * @param sceneManager Scene manager for entity rendering
     * @param uiManager UI manager for UI rendering
     * @param dialogueSystem Dialogue system for dialogue box
     * @param playerController Player controller for NPC indicators
     */
    static void render(
        const NovaEngine::SceneManager& sceneManager,
        const NovaEngine::UIManager& uiManager,
        const NovaEngine::DialogueSystem& dialogueSystem,
        const NovaEngine::PlayerController& playerController,
        NovaEngine::FontHandle font
    );

private:
    static void renderScene(const NovaEngine::SceneManager& sceneManager);
    static void renderNPCIndicator(
        const NovaEngine::DialogueSystem& dialogueSystem,
        const NovaEngine::PlayerController& playerController,
        NovaEngine::FontHandle font
    );
    static void renderDialogue(
        const NovaEngine::DialogueSystem& dialogueSystem
    );
    static void renderUI(const NovaEngine::UIManager& uiManager);
};

} // namespace Game
