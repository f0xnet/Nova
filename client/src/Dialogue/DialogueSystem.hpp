#pragma once

#include "DialogueComponent.hpp"
#include <NovaEngine/ECS/Entity.hpp>
#include <NovaEngine/UI/UIManager.hpp>
#include <string>
#include <vector>

namespace NovaEngine {

/**
 * @brief Dialogue System - Manages NPC dialogues using UIManager
 *
 * This system controls dialogue state and updates the UI accordingly
 * All rendering is delegated to UIManager
 */
class DialogueSystem {
private:
    bool m_dialogueActive = false;
    std::string m_currentNPCName;
    std::vector<std::string> m_currentDialogue;
    size_t m_currentDialogueLine = 0;
    bool m_npcIndicatorVisible = false;  // Track indicator state to avoid redundant UI updates

    UIManager* m_uiManager = nullptr;

public:
    DialogueSystem() = default;

    /**
     * @brief Initialize the dialogue system with UI manager
     * @param uiManager Pointer to UI manager (must remain valid)
     */
    void initialize(UIManager* uiManager);

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
     * @brief Reset dialogue state
     */
    void reset();

    /**
     * @brief Show/hide NPC interaction indicator
     */
    void showNPCIndicator(bool show);

private:
    void updateDialogueUI();
    void showDialogueGroup();
    void hideDialogueGroup();
};

} // namespace NovaEngine
