#pragma once

#include "../ECS/System.hpp"
#include "../ECS/Entity.hpp"
#include "../ECS/Components.hpp"
#include "../Rendering/Effects/DynamicLightingEffect.hpp"
#include "../Rendering/Effects/LightData.hpp"
#include <vector>

namespace NovaEngine {

/**
 * @brief Lighting System - Collects light components and updates rendering
 *
 * This system:
 * - Iterates all entities with LightComponent and TransformComponent
 * - Converts them to LightData for the DynamicLightingEffect
 * - Updates the lighting effect with all active lights
 */
class LightingSystem : public System {
private:
    DynamicLightingEffect* m_lightingEffect;

public:
    LightingSystem() : m_lightingEffect(nullptr) {}

    /**
     * @brief Set the lighting effect to update
     * @param effect Pointer to the DynamicLightingEffect (must remain valid)
     */
    void setLightingEffect(DynamicLightingEffect* effect) {
        m_lightingEffect = effect;
    }

    /**
     * @brief Update all lights from ECS entities
     * @param deltaTime Time since last frame in seconds
     * @param registry The entity registry to query
     */
    void update(float deltaTime, EntityRegistry& registry) override;

    /**
     * @brief Get required components: LightComponent and TransformComponent
     * @return Vector of component type IDs
     */
    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"LightComponent", "TransformComponent"};
    }

    /**
     * @brief Called when system is added to scene
     */
    void onInit() override {}

    /**
     * @brief Called when system is removed from scene
     */
    void onShutdown() override {}

private:
    /**
     * @brief Convert Color to Vec3f (0-1 range)
     */
    static Vec3f colorToVec3f(const Color& color) {
        return Vec3f(
            color.r / 255.0f,
            color.g / 255.0f,
            color.b / 255.0f
        );
    }
};

} // namespace NovaEngine
