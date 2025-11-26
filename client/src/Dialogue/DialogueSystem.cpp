#include "DialogueSystem.hpp"
#include <NovaEngine/UI/UIComponent.hpp>
#include <NovaEngine/UI/Components/Text.hpp>
#include <NovaEngine/Core/Logger.hpp>

namespace NovaEngine {

void DialogueSystem::initialize(UIManager* uiManager) {
    if (!uiManager) {
        LOG_ERROR("DialogueSystem: UIManager is null");
        return;
    }

    m_uiManager = uiManager;

    // Initially hide dialogue UI
    hideDialogueGroup();
    showNPCIndicator(false);

    LOG_INFO("DialogueSystem initialized with UIManager");
}

void DialogueSystem::startDialogue(Entity* npc) {
    if (!npc || !m_uiManager) return;

    auto* dialogue = npc->getComponent<DialogueComponent>();
    if (!dialogue || dialogue->dialogueLines.empty()) {
        LOG_WARN("Cannot start dialogue: NPC has no dialogue data");
        return;
    }

    m_dialogueActive = true;
    m_currentNPCName = dialogue->npcName;
    m_currentDialogue = dialogue->dialogueLines;
    m_currentDialogueLine = 0;

    // Update UI and show dialogue
    updateDialogueUI();
    showDialogueGroup();
    showNPCIndicator(false); // Hide the "[E]" indicator

    LOG_INFO("Started dialogue with {}", m_currentNPCName);
}

bool DialogueSystem::advanceDialogue() {
    if (!m_dialogueActive || !m_uiManager) return false;

    m_currentDialogueLine++;

    if (m_currentDialogueLine >= m_currentDialogue.size()) {
        // Dialogue finished
        reset();
        LOG_INFO("Dialogue ended");
        return false;
    }

    // Update UI with new dialogue line
    updateDialogueUI();
    return true;
}

void DialogueSystem::reset() {
    m_dialogueActive = false;
    m_currentNPCName.clear();
    m_currentDialogue.clear();
    m_currentDialogueLine = 0;

    // Hide dialogue UI
    if (m_uiManager) {
        hideDialogueGroup();
    }
}

void DialogueSystem::showNPCIndicator(bool show) {
    if (!m_uiManager) return;

    // Only update if state has changed to avoid redundant UI updates
    if (m_npcIndicatorVisible != show) {
        m_npcIndicatorVisible = show;
        m_uiManager->setGroupActive("npc_nearby", show);
    }
}

void DialogueSystem::updateDialogueUI() {
    if (!m_uiManager || !m_dialogueActive) return;

    // Update NPC name
    auto npcNameComponent = std::dynamic_pointer_cast<Text>(
        m_uiManager->getComponent("npc_name")
    );
    if (npcNameComponent) {
        npcNameComponent->setString(m_currentNPCName);
    }

    // Update dialogue text
    if (m_currentDialogueLine < m_currentDialogue.size()) {
        auto dialogueTextComponent = std::dynamic_pointer_cast<Text>(
            m_uiManager->getComponent("dialogue_text")
        );
        if (dialogueTextComponent) {
            dialogueTextComponent->setString(m_currentDialogue[m_currentDialogueLine]);
        }
    }
}

void DialogueSystem::showDialogueGroup() {
    if (!m_uiManager) return;
    m_uiManager->setGroupActive("dialogue_active", true);
}

void DialogueSystem::hideDialogueGroup() {
    if (!m_uiManager) return;
    m_uiManager->setGroupActive("dialogue_active", false);
}

} // namespace NovaEngine
