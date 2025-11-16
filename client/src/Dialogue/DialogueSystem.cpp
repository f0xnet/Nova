#include "DialogueSystem.hpp"
#include <NovaEngine/Backend/BackendManager.hpp>
#include <NovaEngine/Core/Logger.hpp>

namespace NovaEngine {

void DialogueSystem::startDialogue(Entity* npc) {
    if (!npc) return;

    auto* dialogue = npc->getComponent<DialogueComponent>();
    if (!dialogue || dialogue->dialogueLines.empty()) {
        LOG_WARN("Cannot start dialogue: NPC has no dialogue data");
        return;
    }

    m_dialogueActive = true;
    m_currentNPCName = dialogue->npcName;
    m_currentDialogue = dialogue->dialogueLines;
    m_currentDialogueLine = 0;

    LOG_INFO("Started dialogue with {}", m_currentNPCName);
}

bool DialogueSystem::advanceDialogue() {
    if (!m_dialogueActive) return false;

    m_currentDialogueLine++;

    if (m_currentDialogueLine >= m_currentDialogue.size()) {
        // Dialogue finished
        reset();
        LOG_INFO("Dialogue ended");
        return false;
    }

    return true;
}

void DialogueSystem::render(const Vec2f& viewCenter, const Vec2f& viewSize) {
    if (!m_dialogueActive || m_currentDialogueLine >= m_currentDialogue.size()) {
        return;
    }

    if (m_font == INVALID_HANDLE) {
        LOG_ERROR("DialogueSystem: Invalid font handle");
        return;
    }

    // Dialogue box dimensions and position (in world coordinates)
    float boxWidth = 900.0f;
    float boxHeight = 150.0f;
    float boxX = viewCenter.x - boxWidth / 2;
    float boxY = viewCenter.y + viewSize.y / 2 - boxHeight - 50;

    // Background box
    RectData box;
    box.position = Vec2f(boxX, boxY);
    box.size = Vec2f(boxWidth, boxHeight);
    box.fillColor = Color(0, 0, 0, 220);
    box.outlineColor = Color(200, 200, 200);
    box.outlineThickness = 3.0f;
    GRAPHICS().drawRect(box);

    // NPC name
    TextData nameText;
    nameText.text = m_currentNPCName;
    nameText.font = m_font;
    nameText.characterSize = 24;
    nameText.fillColor = Color::Yellow;
    nameText.position = Vec2f(boxX + 20, boxY + 10);
    nameText.style = TextStyle::Bold;
    GRAPHICS().drawText(nameText);

    // Dialogue text
    TextData dialogueText;
    dialogueText.text = m_currentDialogue[m_currentDialogueLine];
    dialogueText.font = m_font;
    dialogueText.characterSize = 18;
    dialogueText.fillColor = Color::White;
    dialogueText.position = Vec2f(boxX + 20, boxY + 50);
    GRAPHICS().drawText(dialogueText);

    // Continue indicator
    TextData continueText;
    continueText.text = "[ Press E to continue ]";
    continueText.font = m_font;
    continueText.characterSize = 14;
    continueText.fillColor = Color(150, 150, 150);
    continueText.position = Vec2f(boxX + boxWidth - 200, boxY + boxHeight - 30);
    GRAPHICS().drawText(continueText);
}

void DialogueSystem::reset() {
    m_dialogueActive = false;
    m_currentNPCName.clear();
    m_currentDialogue.clear();
    m_currentDialogueLine = 0;
}

} // namespace NovaEngine
