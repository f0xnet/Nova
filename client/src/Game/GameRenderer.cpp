#include "GameRenderer.hpp"
#include "../Dialogue/DialogueSystem.hpp"
#include "../Player/PlayerController.hpp"
#include <NovaEngine/ECS/SceneManager.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <NovaEngine/UI/UIManager.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>

namespace Game {

void GameRenderer::render(
    const NovaEngine::SceneManager& sceneManager,
    const NovaEngine::UIManager& uiManager,
    const NovaEngine::DialogueSystem& dialogueSystem,
    const NovaEngine::PlayerController& playerController,
    NovaEngine::FontHandle font)
{
    // Render in order: scene -> indicators -> dialogue -> UI
    renderScene(sceneManager);
    renderNPCIndicator(dialogueSystem, playerController, font);
    renderDialogue(dialogueSystem);
    renderUI(uiManager);
}

void GameRenderer::renderScene(const NovaEngine::SceneManager& sceneManager) {
    using namespace NovaEngine;

    Scene* scene = sceneManager.getActiveScene();
    if (!scene) return;

    // Render all visible entities with sprites
    for (auto* entity : scene->getEntityRegistry().getAllEntities()) {
        auto* transform = entity->getComponent<TransformComponent>();
        auto* sprite = entity->getComponent<SpriteComponent>();

        if (transform && sprite && sprite->visible && sprite->textureHandle != INVALID_HANDLE) {
            SpriteData spriteData;
            spriteData.texture = sprite->textureHandle;
            spriteData.position = transform->position;
            spriteData.size = sprite->size;
            spriteData.rotation = transform->rotation;
            spriteData.origin = transform->origin;
            spriteData.color = sprite->tint;
            spriteData.textureRect = sprite->textureRect;

            GRAPHICS().drawSprite(spriteData);
        }
    }
}

void GameRenderer::renderNPCIndicator(
    const NovaEngine::DialogueSystem& dialogueSystem,
    const NovaEngine::PlayerController& playerController,
    NovaEngine::FontHandle font)
{
    using namespace NovaEngine;

    // Don't show indicator during dialogue
    if (dialogueSystem.isActive()) return;

    Entity* nearestNPC = playerController.getNearestNPC();
    if (!nearestNPC) return;

    auto* npcTransform = nearestNPC->getComponent<TransformComponent>();
    if (!npcTransform) return;

    // Draw "[E]" indicator above NPC
    TextData indicator;
    indicator.text = "[ E ]";
    indicator.font = font;
    indicator.characterSize = 20;
    indicator.fillColor = Color::Yellow;
    indicator.position = Vec2f(
        npcTransform->position.x - 20,
        npcTransform->position.y - 50
    );
    GRAPHICS().drawText(indicator);
}

void GameRenderer::renderDialogue(const NovaEngine::DialogueSystem& dialogueSystem) {
    using namespace NovaEngine;

    if (!dialogueSystem.isActive()) return;

    Vec2f viewCenter = VIEWPORT().getViewCenter();
    Vec2f viewSize = VIEWPORT().getViewSize();

    // Delegate to dialogue system's render method
    const_cast<DialogueSystem&>(dialogueSystem).render(viewCenter, viewSize);
}

void GameRenderer::renderUI(const NovaEngine::UIManager& uiManager) {
    // Delegate to UI manager's render method
    const_cast<NovaEngine::UIManager&>(uiManager).render();
}

} // namespace Game
