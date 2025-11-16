#include "PlayerController.hpp"
#include "../Dialogue/DialogueComponent.hpp"
#include <NovaEngine/Backend/BackendManager.hpp>
#include <cmath>

namespace NovaEngine {

void PlayerController::updateMovement(Scene* scene, float deltaTime, bool allowMovement) {
    if (!scene || !allowMovement) return;

    Entity* player = scene->getEntityRegistry().getEntity(m_playerID);
    if (!player) return;

    auto* transform = player->getComponent<TransformComponent>();
    if (!transform) return;

    // Get input
    Vec2f movement(0, 0);

    if (INPUT().isKeyPressed(KeyCode::W) || INPUT().isKeyPressed(KeyCode::Up)) {
        movement.y -= 1;
    }
    if (INPUT().isKeyPressed(KeyCode::S) || INPUT().isKeyPressed(KeyCode::Down)) {
        movement.y += 1;
    }
    if (INPUT().isKeyPressed(KeyCode::A) || INPUT().isKeyPressed(KeyCode::Left)) {
        movement.x -= 1;
    }
    if (INPUT().isKeyPressed(KeyCode::D) || INPUT().isKeyPressed(KeyCode::Right)) {
        movement.x += 1;
    }

    // Normalize diagonal movement
    if (movement.x != 0 || movement.y != 0) {
        float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        movement.x /= length;
        movement.y /= length;

        // Apply movement
        transform->position.x += movement.x * m_moveSpeed * deltaTime;
        transform->position.y += movement.y * m_moveSpeed * deltaTime;
    }
}

void PlayerController::updateNPCDetection(Scene* scene) {
    m_nearestNPC = nullptr;

    if (!scene) return;

    Entity* player = scene->getEntityRegistry().getEntity(m_playerID);
    if (!player) return;

    auto* playerTransform = player->getComponent<TransformComponent>();
    if (!playerTransform) return;

    float nearestDist = m_npcDetectionRadius;

    // Search for NPCs with DialogueComponent
    for (auto* entity : scene->getEntityRegistry().getAllEntities()) {
        // Skip the player
        if (entity->getID() == m_playerID) continue;

        // Check for DialogueComponent
        auto* dialogue = entity->getComponent<DialogueComponent>();
        if (!dialogue) continue;

        auto* npcTransform = entity->getComponent<TransformComponent>();
        if (!npcTransform) continue;

        // Calculate distance
        Vec2f diff = npcTransform->position - playerTransform->position;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

        if (dist < nearestDist) {
            nearestDist = dist;
            m_nearestNPC = entity;
        }
    }
}

Vec2f PlayerController::getPlayerPosition(Scene* scene) const {
    if (!scene) return Vec2f(0, 0);

    Entity* player = scene->getEntityRegistry().getEntity(m_playerID);
    if (!player) return Vec2f(0, 0);

    auto* transform = player->getComponent<TransformComponent>();
    if (!transform) return Vec2f(0, 0);

    return transform->position;
}

} // namespace NovaEngine
