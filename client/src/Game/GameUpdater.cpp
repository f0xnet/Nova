#include "GameUpdater.hpp"
#include "../Dialogue/DialogueSystem.hpp"
#include "../Player/PlayerController.hpp"
#include <NovaEngine/ECS/SceneManager.hpp>
#include <NovaEngine/UI/UIManager.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>

namespace Game {

void GameUpdater::update(
    NovaEngine::SceneManager& sceneManager,
    NovaEngine::UIManager& uiManager,
    const NovaEngine::DialogueSystem& dialogueSystem,
    NovaEngine::PlayerController& playerController,
    float deltaTime)
{
    // Update in specific order to ensure proper game flow
    updateSceneManager(sceneManager, deltaTime);
    updatePlayer(sceneManager, dialogueSystem, playerController, deltaTime);
    updateCamera(sceneManager, playerController);
    updateUI(uiManager, deltaTime);
}

void GameUpdater::updateSceneManager(NovaEngine::SceneManager& sceneManager, float deltaTime) {
    sceneManager.update(deltaTime);
}

void GameUpdater::updatePlayer(
    NovaEngine::SceneManager& sceneManager,
    const NovaEngine::DialogueSystem& dialogueSystem,
    NovaEngine::PlayerController& playerController,
    float deltaTime)
{
    using namespace NovaEngine;

    Scene* scene = sceneManager.getActiveScene();
    if (!scene) return;

    // Update movement (disabled during dialogue)
    bool allowMovement = !dialogueSystem.isActive();
    playerController.updateMovement(scene, deltaTime, allowMovement);

    // Update NPC detection
    playerController.updateNPCDetection(scene);
}

void GameUpdater::updateCamera(
    const NovaEngine::SceneManager& sceneManager,
    const NovaEngine::PlayerController& playerController)
{
    using namespace NovaEngine;

    Scene* scene = sceneManager.getActiveScene();
    if (!scene) return;

    // Camera follows player
    Vec2f playerPos = playerController.getPlayerPosition(scene);
    VIEWPORT().setViewCenter(playerPos);
}

void GameUpdater::updateUI(NovaEngine::UIManager& uiManager, float deltaTime) {
    uiManager.update(deltaTime);
}

} // namespace Game
