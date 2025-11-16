#pragma once

#include <NovaEngine/ECS/Entity.hpp>
#include <NovaEngine/ECS/Scene.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <NovaEngine/Backend/Core/BackendTypes.hpp>

namespace NovaEngine {

/**
 * @brief Player Controller - Manages player movement and NPC detection
 *
 * Handles player input, movement, and proximity-based NPC detection
 */
class PlayerController {
private:
    u64 m_playerID = 0;
    Entity* m_nearestNPC = nullptr;
    float m_moveSpeed = 200.0f;
    float m_npcDetectionRadius = 80.0f;

public:
    PlayerController() = default;

    /**
     * @brief Set the player entity ID
     */
    void setPlayerID(u64 playerID) { m_playerID = playerID; }

    /**
     * @brief Get the player entity ID
     */
    u64 getPlayerID() const { return m_playerID; }

    /**
     * @brief Update player movement based on input
     * @param scene Current scene
     * @param deltaTime Time since last frame
     * @param allowMovement Whether movement is allowed (false during dialogues)
     */
    void updateMovement(Scene* scene, float deltaTime, bool allowMovement);

    /**
     * @brief Update NPC detection
     * @param scene Current scene
     */
    void updateNPCDetection(Scene* scene);

    /**
     * @brief Get the nearest NPC entity
     */
    Entity* getNearestNPC() const { return m_nearestNPC; }

    /**
     * @brief Get player position
     */
    Vec2f getPlayerPosition(Scene* scene) const;

    /**
     * @brief Set movement speed
     */
    void setMoveSpeed(float speed) { m_moveSpeed = speed; }

    /**
     * @brief Set NPC detection radius
     */
    void setDetectionRadius(float radius) { m_npcDetectionRadius = radius; }
};

} // namespace NovaEngine
