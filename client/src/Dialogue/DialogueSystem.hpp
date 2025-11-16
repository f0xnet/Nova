#pragma once

#include "DialogueComponent.hpp"
#include <NovaEngine/ECS/Entity.hpp>
#include <NovaEngine/Backend/Core/BackendTypes.hpp>
#include <string>
#include <vector>

namespace NovaEngine {

/**
 * @brief Dialogue System - Manages NPC dialogues and UI rendering
 *
 * Handles dialogue state, progression, and visual presentation
 */
class DialogueSystem {
private:
    bool m_dialogueActive = false;
    std::string m_currentNPCName;
    std::vector<std::string> m_currentDialogue;
    size_t m_currentDialogueLine = 0;
    FontHandle m_font = INVALID_HANDLE;

public:
    DialogueSystem() = default;

    /**
     * @brief Initialize the dialogue system
     * @param fontHandle Handle to the font to use for text rendering
     */
    void initialize(FontHandle fontHandle) {
        m_font = fontHandle;
    }

    /**
     * @brief Start a dialogue with an NPC
     * @param npc Entity with DialogueComponent
     */
    void startDialogue(Entity* npc);

    /**
     * @brief Advance to the next line of dialogue
     * @return true if dialogue continues, false if ended
     */
    bool advanceDialogue();

    /**
     * @brief Check if dialogue is currently active
     */
    bool isActive() const { return m_dialogueActive; }

    /**
     * @brief Render the dialogue box
     * @param viewCenter Current camera center position
     * @param viewSize Current viewport size
     */
    void render(const Vec2f& viewCenter, const Vec2f& viewSize);

    /**
     * @brief Reset dialogue state
     */
    void reset();
};

} // namespace NovaEngine
