#include "GameEventHandler.hpp"
#include "../Dialogue/DialogueSystem.hpp"
#include "../Player/PlayerController.hpp"
#include <NovaEngine/Core/Logger.hpp>

namespace Game {

bool GameEventHandler::handleEvent(
    const NovaEngine::Event& event,
    NovaEngine::DialogueSystem& dialogueSystem,
    const NovaEngine::PlayerController& playerController)
{
    using namespace NovaEngine;

    // Only handle input events
    if (event.type != EventType::Input) {
        return false;
    }

    const InputEvent& inputEvent = event.inputEvent;

    // Handle key press events
    if (inputEvent.type == InputEventType::KeyPressed) {
        return handleKeyPress(inputEvent, dialogueSystem, playerController);
    }

    return false;
}

bool GameEventHandler::handleKeyPress(
    const NovaEngine::InputEvent& inputEvent,
    NovaEngine::DialogueSystem& dialogueSystem,
    const NovaEngine::PlayerController& playerController)
{
    using namespace NovaEngine;

    // Handle ESC key - quit game
    if (inputEvent.key.code == KeyCode::Escape) {
        return handleEscapeKey();
    }

    // Handle E key - NPC interaction
    if (inputEvent.key.code == KeyCode::E) {
        handleInteractionKey(dialogueSystem, playerController);
        return false;
    }

    return false;
}

bool GameEventHandler::handleEscapeKey() {
    using namespace NovaEngine;

    LOG_INFO("Escape pressed - quitting game");
    return true; // Signal to quit
}

void GameEventHandler::handleInteractionKey(
    NovaEngine::DialogueSystem& dialogueSystem,
    const NovaEngine::PlayerController& playerController)
{
    using namespace NovaEngine;

    if (dialogueSystem.isActive()) {
        // Advance current dialogue
        dialogueSystem.advanceDialogue();
    } else {
        // Start dialogue with nearest NPC
        Entity* nearestNPC = playerController.getNearestNPC();
        if (nearestNPC) {
            dialogueSystem.startDialogue(nearestNPC);
        }
    }
}

} // namespace Game
