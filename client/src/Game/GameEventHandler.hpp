#pragma once

#include <NovaEngine/Events/Event.hpp>

namespace NovaEngine {
    class DialogueSystem;
    class PlayerController;
}

namespace Game {

/**
 * @brief Game Event Handler - Handles all input events
 *
 * Separates event handling concerns from main game loop
 */
class GameEventHandler {
public:
    /**
     * @brief Handle a game event
     * @param event The event to handle
     * @param dialogueSystem Dialogue system for dialogue interactions
     * @param playerController Player controller for NPC detection
     * @return true if the event was handled and app should quit, false otherwise
     */
    static bool handleEvent(
        const NovaEngine::Event& event,
        NovaEngine::DialogueSystem& dialogueSystem,
        const NovaEngine::PlayerController& playerController
    );

private:
    static bool handleKeyPress(
        const NovaEngine::InputEvent& inputEvent,
        NovaEngine::DialogueSystem& dialogueSystem,
        const NovaEngine::PlayerController& playerController
    );

    static bool handleEscapeKey();
    static void handleInteractionKey(
        NovaEngine::DialogueSystem& dialogueSystem,
        const NovaEngine::PlayerController& playerController
    );
};

} // namespace Game
