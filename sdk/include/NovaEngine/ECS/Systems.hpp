#pragma once

#include "System.hpp"
#include "Components.hpp"
#include "EntityRegistry.hpp"
#include "../Backend/BackendManager.hpp"
#include "../Core/Logger.hpp"
#include <algorithm>

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

                    // Calculate AABB bounds
                    Vec2f posA = transformA->position + colliderA->offset;
                    Vec2f posB = transformB->position + colliderB->offset;

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

} // namespace NovaEngine
