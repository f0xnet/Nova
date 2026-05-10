#pragma once

#include "System.hpp"
#include "Components.hpp"
#include "EntityRegistry.hpp"
#include "SceneGraph.hpp"
#include "../Backend/BackendManager.hpp"
#include "../Core/Logger.hpp"
#include <algorithm>
#include <limits>

namespace NovaEngine {

// ============================================================================
// RenderSystem - Renders all sprites
// ============================================================================

/**
 * @brief Render system - draws all entities with Sprite + Transform components
 *
 * Entities are sorted by z-order before rendering.
 */
class RenderSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto entities = registry.getEntitiesWith({"TransformComponent", "SpriteComponent"});

        // Sort by z-order (lower values rendered first = behind)
        std::sort(entities.begin(), entities.end(), [](Entity* a, Entity* b) {
            auto* spriteA = a->getComponent<SpriteComponent>();
            auto* spriteB = b->getComponent<SpriteComponent>();
            if (!spriteA || !spriteB) return false;
            return spriteA->zOrder < spriteB->zOrder;
        });

        // Render each sprite
        for (Entity* entity : entities) {
            auto* transform = entity->getComponent<TransformComponent>();
            auto* sprite = entity->getComponent<SpriteComponent>();

            if (!sprite->visible || sprite->textureHandle == INVALID_HANDLE) continue;

            // Prepare sprite data
            SpriteData spriteData;
            spriteData.texture = sprite->textureHandle;
            spriteData.position = transform->position;
            spriteData.size = sprite->size;
            spriteData.rotation = transform->rotation;
            spriteData.scale = transform->scale;
            spriteData.origin = transform->origin;
            spriteData.textureRect = sprite->textureRect;
            spriteData.color = sprite->tint;
            spriteData.blendMode = sprite->blendMode;

            // Check for custom shader
            auto* shaderComp = entity->getComponent<ShaderComponent>();
            if (shaderComp && shaderComp->enabled && shaderComp->shader != INVALID_HANDLE) {
                spriteData.shader = shaderComp->shader;
            }

            // Draw
            GRAPHICS().drawSprite(spriteData);
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "SpriteComponent"};
    }
};

// ============================================================================
// AnimationSystem - Updates frame-based animations
// ============================================================================

/**
 * @brief Animation system - updates sprite animations
 *
 * Advances animation frames based on time and updates the sprite's texture rect.
 */
class AnimationSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto entities = registry.getEntitiesWith({"SpriteComponent", "AnimationComponent"});

        for (Entity* entity : entities) {
            auto* sprite = entity->getComponent<SpriteComponent>();
            auto* anim = entity->getComponent<AnimationComponent>();

            if (!anim->playing || anim->frames.empty()) continue;

            // Advance animation time
            anim->currentTime += deltaTime;

            // Check if we need to advance to next frame
            if (anim->currentTime >= anim->frameDuration) {
                anim->currentTime = 0.0f;
                anim->currentFrame++;

                // Handle loop/end
                if (anim->currentFrame >= anim->frames.size()) {
                    if (anim->loop) {
                        anim->currentFrame = 0;
                    } else {
                        anim->currentFrame = static_cast<u32>(anim->frames.size()) - 1;
                        anim->playing = false;
                    }
                }

                // Update sprite texture rect
                if (anim->currentFrame < anim->frames.size()) {
                    sprite->textureRect = anim->frames[anim->currentFrame];
                }
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"SpriteComponent", "AnimationComponent"};
    }
};

// ============================================================================
// LightSystem - Renders lights (simple visualization)
// ============================================================================

/**
 * @brief Light system - renders lights as colored circles
 *
 * This is a simple visualization. For advanced lighting, you would use shaders.
 */
class LightSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto entities = registry.getEntitiesWith({"TransformComponent", "LightComponent"});

        for (Entity* entity : entities) {
            auto* transform = entity->getComponent<TransformComponent>();
            auto* light = entity->getComponent<LightComponent>();

            if (!light->enabled) continue;

            // Simple point light visualization (colored circle)
            if (light->type == LightComponent::LightType::Point) {
                RectData lightRect;
                lightRect.position = transform->position - Vec2f{light->radius, light->radius};
                lightRect.size = Vec2f{light->radius * 2.0f, light->radius * 2.0f};

                // Semi-transparent colored glow
                Color lightColor = light->color;
                lightColor.a = static_cast<u8>(light->intensity * 50.0f);  // Semi-transparent
                lightRect.fillColor = lightColor;
                lightRect.outlineThickness = 0.0f;

                GRAPHICS().drawRect(lightRect);
            }
            // Directional lights would require shader-based rendering
            // Spot lights would use a cone shape (can be done with triangles or shaders)
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "LightComponent"};
    }
};

// ============================================================================
// AudioSystem - Handles audio playback
// ============================================================================

/**
 * @brief Audio system - plays sounds for entities with AudioComponent
 */
class AudioSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto entities = registry.getEntitiesWith({"AudioComponent"});

        for (Entity* entity : entities) {
            auto* audio = entity->getComponent<AudioComponent>();

            // Play on start
            if (audio->playOnStart && !audio->playing && audio->soundHandle != INVALID_HANDLE) {
                AUDIO().playSound(audio->soundHandle, audio->volume, 1.0f, audio->loop);
                audio->playing = true;
                LOG_DEBUG("Entity {}: Playing sound (handle: {})", entity->getID(), audio->soundHandle);
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"AudioComponent"};
    }
};

// ============================================================================
// PhysicsSystem - Simple AABB collision detection
// ============================================================================

/**
 * @brief Physics system - basic AABB collision detection
 *
 * This is a simple example. For a full physics engine, you'd use Box2D or similar.
 */
class PhysicsSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto entities = registry.getEntitiesWith({"TransformComponent", "ColliderComponent"});

        // Simple AABB collision check between all entities
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                Entity* entityA = entities[i];
                Entity* entityB = entities[j];

                auto* transformA = entityA->getComponent<TransformComponent>();
                auto* colliderA = entityA->getComponent<ColliderComponent>();
                auto* transformB = entityB->getComponent<TransformComponent>();
                auto* colliderB = entityB->getComponent<ColliderComponent>();

                if (!colliderA->enabled || !colliderB->enabled) continue;

                // Only check box-box collisions for simplicity
                if (colliderA->type == ColliderComponent::ColliderType::Box &&
                    colliderB->type == ColliderComponent::ColliderType::Box) {

                    // Calculate AABB bounds — subtract origin so the box aligns with the
                    // sprite's top-left corner (transform->position is the visual center)
                    Vec2f posA = transformA->position - transformA->origin + colliderA->offset;
                    Vec2f posB = transformB->position - transformB->origin + colliderB->offset;

                    Rect boundsA{posA.x, posA.y, colliderA->size.x, colliderA->size.y};
                    Rect boundsB{posB.x, posB.y, colliderB->size.x, colliderB->size.y};

                    // Check intersection
                    if (boundsA.left < boundsB.left + boundsB.width &&
                        boundsA.left + boundsA.width > boundsB.left &&
                        boundsA.top < boundsB.top + boundsB.height &&
                        boundsA.top + boundsA.height > boundsB.top) {

                        // Collision detected
                        LOG_DEBUG("Collision between entities {} and {}",
                                entityA->getID(), entityB->getID());

                        // Here you would fire collision events or resolve physics
                    }
                }
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "ColliderComponent"};
    }
};

// ============================================================================
// ActivatorSystem - Handles trigger zone activation
// ============================================================================

/**
 * @brief Activator system - detects entities in trigger zones and activates them
 *
 * This system checks all activators and detects when tagged entities
 * enter their activation zones. It handles cooldowns, reactivation,
 * and fires events when activation occurs.
 */
class ActivatorSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto activators = registry.getEntitiesWith({"TransformComponent", "ActivatorComponent"});
        auto potentialTriggers = registry.getEntitiesWith({"TransformComponent", "TagComponent"});

        // Update cooldowns
        for (Entity* activatorEntity : activators) {
            auto* activator = activatorEntity->getComponent<ActivatorComponent>();

            if (activator->currentCooldown > 0.0f) {
                activator->currentCooldown -= deltaTime;
                if (activator->currentCooldown <= 0.0f) {
                    activator->currentCooldown = 0.0f;
                }
            }
        }

        // Check for activations
        for (Entity* activatorEntity : activators) {
            auto* activatorTransform = activatorEntity->getComponent<TransformComponent>();
            auto* activator = activatorEntity->getComponent<ActivatorComponent>();

            // Skip if on cooldown
            if (activator->currentCooldown > 0.0f) continue;

            // Calculate activation zone — subtract origin to anchor from visual top-left
            Vec2f activatorPos = activatorTransform->position - activatorTransform->origin + activator->offset;
            bool wasActive = activator->isActive;
            bool entityInZone = false;

            // Check all tagged entities
            for (Entity* triggerEntity : potentialTriggers) {
                auto* tag = triggerEntity->getComponent<TagComponent>();

                // Skip if not the target tag
                if (tag->tag != activator->targetTag) continue;

                auto* triggerTransform = triggerEntity->getComponent<TransformComponent>();
                Vec2f triggerPos = triggerTransform->position - triggerTransform->origin;

                // Check if entity is in activation zone
                bool inZone = false;

                if (activator->shape == ActivatorComponent::ActivatorShape::Box) {
                    // Box collision check
                    Rect activatorRect{
                        activatorPos.x - activator->size.x * 0.5f,
                        activatorPos.y - activator->size.y * 0.5f,
                        activator->size.x,
                        activator->size.y
                    };
                    inZone = activatorRect.contains(triggerPos);
                }
                else if (activator->shape == ActivatorComponent::ActivatorShape::Circle) {
                    // Circle collision check
                    f32 dx = triggerPos.x - activatorPos.x;
                    f32 dy = triggerPos.y - activatorPos.y;
                    f32 distSquared = dx * dx + dy * dy;
                    inZone = distSquared <= (activator->radius * activator->radius);
                }

                if (inZone) {
                    entityInZone = true;
                    break;
                }
            }

            // Handle activation logic
            if (activator->type == ActivatorComponent::ActivatorType::Proximity) {
                // Proximity: activate once on entry
                if (entityInZone && !wasActive) {
                    activateActivator(activatorEntity, activator);
                }
                else if (!entityInZone && wasActive) {
                    deactivateActivator(activatorEntity, activator);
                }
            }
            else if (activator->type == ActivatorComponent::ActivatorType::Automatic) {
                // Automatic: activate continuously while in zone
                if (entityInZone) {
                    if (!wasActive) {
                        activateActivator(activatorEntity, activator);
                    }
                    // Still active, you can add continuous logic here
                }
                else if (wasActive) {
                    deactivateActivator(activatorEntity, activator);
                }
            }
            else if (activator->type == ActivatorComponent::ActivatorType::Manual) {
                // Manual: requires explicit activation (e.g., key press)
                // This would be handled by input system or custom logic
                // For now, just update state based on zone
                if (entityInZone && !wasActive) {
                    // You would check for input here in a real implementation
                    // For demo purposes, we'll just log that manual activation is possible
                    LOG_DEBUG("Entity {} can be manually activated (press E)",
                             activatorEntity->getID());
                }
            }

            // Draw debug visualization
            if (activator->showDebugZone) {
                if (activator->shape == ActivatorComponent::ActivatorShape::Box) {
                    RectData debugRect;
                    debugRect.position = Vec2f{
                        activatorPos.x - activator->size.x * 0.5f,
                        activatorPos.y - activator->size.y * 0.5f
                    };
                    debugRect.size = activator->size;
                    debugRect.fillColor = Color{0, 0, 0, 0};  // Transparent fill
                    debugRect.outlineColor = activator->isActive ?
                        Color{255, 0, 0, 200} : activator->debugColor;
                    debugRect.outlineThickness = 2.0f;
                    GRAPHICS().drawRect(debugRect);
                }
                else if (activator->shape == ActivatorComponent::ActivatorShape::Circle) {
                    // Draw circle as rect for simplicity (you could make a circle draw function)
                    RectData debugCircle;
                    debugCircle.position = Vec2f{
                        activatorPos.x - activator->radius,
                        activatorPos.y - activator->radius
                    };
                    debugCircle.size = Vec2f{activator->radius * 2.0f, activator->radius * 2.0f};
                    debugCircle.fillColor = Color{0, 0, 0, 0};
                    debugCircle.outlineColor = activator->isActive ?
                        Color{255, 0, 0, 200} : activator->debugColor;
                    debugCircle.outlineThickness = 2.0f;
                    GRAPHICS().drawRect(debugCircle);
                }
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "ActivatorComponent"};
    }

private:
    void activateActivator(Entity* entity, ActivatorComponent* activator) {
        activator->isActive = true;

        LOG_INFO("Activator {} activated! Action: '{}'",
                entity->getID(), activator->actionID);

        // Fire activation event
        if (!activator->onActivateEvent.empty()) {
            LOG_DEBUG("Firing event: {}", activator->onActivateEvent);
            // Here you would fire the event through an event system
            // For now we just log it
        }

        // Start cooldown if can't reactivate
        if (!activator->canReactivate) {
            activator->currentCooldown = std::numeric_limits<f32>::infinity();  // Permanently disable
        }
        else if (activator->cooldownTime > 0.0f) {
            activator->currentCooldown = activator->cooldownTime;
        }
    }

    void deactivateActivator(Entity* entity, ActivatorComponent* activator) {
        activator->isActive = false;

        LOG_DEBUG("Activator {} deactivated", entity->getID());

        // Fire deactivation event
        if (!activator->onDeactivateEvent.empty()) {
            LOG_DEBUG("Firing event: {}", activator->onDeactivateEvent);
        }
    }
};

// ============================================================================
// JourneySystem - Multi-scene NPC travel management
// ============================================================================

/**
 * @brief Journey system - manages multi-scene NPC travel
 *
 * This system ensures NPCs physically traverse all intermediate scenes
 * instead of teleporting. If the player is in an intermediate scene,
 * they will SEE the NPC passing through.
 *
 * Requires a SceneGraph to be provided for pathfinding.
 */
class JourneySystem : public System {
private:
    class SceneGraph* m_sceneGraph;

    struct PendingTransfer {
        u64 entityID;
        std::string fromScene;
        std::string toScene;
        Vec2f targetPosition;
        int nextSceneIndex;
        std::vector<std::string> remainingPath;
        Vec2f finalDestination;
        std::string finalScene;
    };

    std::vector<PendingTransfer> m_pendingTransfers;

public:
    explicit JourneySystem(class SceneGraph* sceneGraph)
        : m_sceneGraph(sceneGraph) {}

    void update(float deltaTime, EntityRegistry& registry) override {
        auto travelers = registry.getEntitiesWith({
            "TransformComponent",
            "SceneTransitionComponent",
            "JourneyComponent"
        });

        for (Entity* entity : travelers) {
            auto* transform = entity->getComponent<TransformComponent>();
            auto* transition = entity->getComponent<SceneTransitionComponent>();
            auto* journey = entity->getComponent<JourneyComponent>();

            if (!journey->isOnJourney) continue;

            // Suivre le chemin de waypoints locaux si disponible
            if (!journey->localWaypointPath.empty() &&
                journey->currentLocalWaypointIndex < static_cast<int>(journey->localWaypointPath.size())) {

                // Vérifier si on a atteint le waypoint actuel
                Vec2f currentWaypoint = journey->localWaypointPath[journey->currentLocalWaypointIndex];
                Vec2f toWaypoint = currentWaypoint - transform->position;
                f32 distanceSquared = toWaypoint.x * toWaypoint.x + toWaypoint.y * toWaypoint.y;

                if (distanceSquared < 25.0f) {  // Moins de 5 pixels
                    // Waypoint atteint, passer au suivant
                    journey->currentLocalWaypointIndex++;

                    if (journey->currentLocalWaypointIndex >= static_cast<int>(journey->localWaypointPath.size())) {
                        // Tous les waypoints atteints = destination finale atteinte
                        journey->reachedCurrentDestination = true;
                        LOG_DEBUG("NPC {} reached final destination via waypoints", entity->getID());
                    } else {
                        LOG_DEBUG("NPC {} reached waypoint {}/{}", entity->getID(),
                                 journey->currentLocalWaypointIndex,
                                 journey->localWaypointPath.size());
                    }
                }
            } else {
                // Pas de waypoints, aller directement à currentDestination (ancien comportement)
                Vec2f toDestination = journey->currentDestination - transform->position;
                f32 distanceSquared = toDestination.x * toDestination.x + toDestination.y * toDestination.y;

                if (distanceSquared < 25.0f) {  // Moins de 5 pixels
                    // Atteint le portail/destination!
                    journey->reachedCurrentDestination = true;
                }
            }

            // Si destination atteinte, gérer la transition de scène
            if (journey->reachedCurrentDestination) {

                // Y a-t-il une prochaine scène?
                if (journey->currentSceneIndex + 1 < static_cast<int>(journey->scenePath.size())) {
                    // OUI - Préparer la transition vers la scène suivante
                    std::string currentScene = journey->scenePath[journey->currentSceneIndex];
                    std::string nextScene = journey->scenePath[journey->currentSceneIndex + 1];

                    if (m_sceneGraph) {
                        const auto* connection = m_sceneGraph->getConnection(currentScene, nextScene);

                        if (connection) {
                            LOG_INFO("NPC {} transitioning from '{}' to '{}' (journey step {}/{})",
                                    entity->getID(), currentScene, nextScene,
                                    journey->currentSceneIndex + 1, journey->scenePath.size() - 1);

                            // Enregistrer la transition pendante
                            PendingTransfer transfer;
                            transfer.entityID = entity->getID();
                            transfer.fromScene = currentScene;
                            transfer.toScene = nextScene;
                            transfer.targetPosition = connection->entryPortalPos;
                            transfer.nextSceneIndex = journey->currentSceneIndex + 1;
                            transfer.remainingPath = journey->scenePath;
                            transfer.finalDestination = journey->finalDestinationPos;
                            transfer.finalScene = journey->finalDestinationScene;
                            m_pendingTransfers.push_back(transfer);

                            // Marquer pour transition
                            transition->targetScene = nextScene;
                            transition->targetPosition = connection->entryPortalPos;
                            transition->isTransitioning = true;
                        }
                    }
                } else {
                    // FIN du voyage!
                    LOG_INFO("NPC {} completed journey to '{}'",
                            entity->getID(), journey->finalDestinationScene);
                    journey->isOnJourney = false;
                    journey->scenePath.clear();
                }
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "SceneTransitionComponent", "JourneyComponent"};
    }

    /**
     * @brief Process pending transfers after scene transitions
     *
     * This should be called by SceneManager after entities have been transferred
     */
    void updateTransferredEntities(EntityRegistry& registry) {
        for (const auto& transfer : m_pendingTransfers) {
            Entity* entity = registry.getEntity(transfer.entityID);
            if (!entity) continue;

            auto* journey = entity->getComponent<JourneyComponent>();
            if (!journey) continue;

            // Mettre à jour le voyage
            journey->currentSceneIndex = transfer.nextSceneIndex;
            journey->reachedCurrentDestination = false;

            // Définir la prochaine destination
            if (journey->currentSceneIndex == static_cast<int>(journey->scenePath.size()) - 1) {
                // Dernière scène - aller à la destination finale
                journey->currentDestination = transfer.finalDestination;
            } else {
                // Scène intermédiaire - aller au prochain portail
                std::string currentScene = journey->scenePath[journey->currentSceneIndex];
                std::string nextScene = journey->scenePath[journey->currentSceneIndex + 1];

                if (m_sceneGraph) {
                    const auto* connection = m_sceneGraph->getConnection(currentScene, nextScene);
                    if (connection) {
                        journey->currentDestination = connection->exitPortalPos;
                    }
                }
            }
        }

        m_pendingTransfers.clear();
    }

    /**
     * @brief Start a journey for an NPC
     * @param entity The entity to send on a journey
     * @param currentScene Current scene name
     * @param targetScene Destination scene name
     * @param targetPosition Final position in destination scene
     * @return true if journey started successfully
     */
    bool startJourney(Entity* entity,
                     const std::string& currentScene,
                     const std::string& targetScene,
                     const Vec2f& targetPosition) {
        auto* journey = entity->getComponent<JourneyComponent>();
        auto* transition = entity->getComponent<SceneTransitionComponent>();

        if (!journey || !transition) {
            LOG_ERROR("Entity {} missing Journey or Transition component", entity->getID());
            return false;
        }

        if (!m_sceneGraph) {
            LOG_ERROR("No SceneGraph available for pathfinding");
            return false;
        }

        // Trouver le chemin
        journey->scenePath = m_sceneGraph->findPath(currentScene, targetScene);

        if (journey->scenePath.empty()) {
            LOG_ERROR("No path found from '{}' to '{}'", currentScene, targetScene);
            return false;
        }

        LOG_INFO("NPC {} starting journey: {} -> {} ({} scenes to traverse)",
                entity->getID(), currentScene, targetScene, journey->scenePath.size());

        journey->currentSceneIndex = 0;
        journey->isOnJourney = true;
        journey->finalDestinationScene = targetScene;
        journey->finalDestinationPos = targetPosition;
        journey->reachedCurrentDestination = false;

        // Définir la première destination
        if (journey->scenePath.size() > 1) {
            // Aller au portail de sortie vers la prochaine scène
            const auto* connection = m_sceneGraph->getConnection(
                journey->scenePath[0],
                journey->scenePath[1]
            );
            if (connection) {
                journey->currentDestination = connection->exitPortalPos;
                LOG_DEBUG("  First destination: portal to '{}' at ({}, {})",
                         journey->scenePath[1],
                         connection->exitPortalPos.x,
                         connection->exitPortalPos.y);
            }
        } else {
            // Déjà dans la bonne scène
            journey->currentDestination = targetPosition;
        }

        return true;
    }

    /**
     * @brief Calculate local waypoint path for current scene
     * @param entity Entity to calculate path for
     * @param currentScene Current scene (for waypoint graph access)
     *
     * This should be called when:
     * - Journey starts
     * - Entity enters a new scene
     * - Destination changes
     *
     * Uses the entity's preferredPathTags for personality-based pathfinding.
     *
     * Implementation is in Scene.hpp after Scene class is defined to avoid circular dependency.
     */
    void calculateLocalWaypointPath(Entity* entity, class Scene* currentScene);

    /**
     * @brief Cancel an ongoing journey
     */
    void cancelJourney(Entity* entity) {
        auto* journey = entity->getComponent<JourneyComponent>();
        if (journey) {
            journey->isOnJourney = false;
            journey->scenePath.clear();
            journey->localWaypointPath.clear();
            LOG_INFO("NPC {} journey cancelled", entity->getID());
        }
    }
};

} // namespace NovaEngine
