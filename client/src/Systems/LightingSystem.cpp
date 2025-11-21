#include <NovaEngine/Systems/LightingSystem.hpp>
#include <NovaEngine/ECS/EntityRegistry.hpp>
#include <NovaEngine/Core/Logger.hpp>

namespace NovaEngine {

void LightingSystem::update(float deltaTime, EntityRegistry& registry) {
    if (!m_lightingEffect) {
        return; // No lighting effect to update
    }

    // Clear previous lights
    m_lightingEffect->clearLights();

    // Get all entities with LightComponent and TransformComponent
    auto entities = registry.getAllEntities();

    for (auto* entity : entities) {
        // Check if entity has both required components
        if (!entity->hasComponent<LightComponent>() ||
            !entity->hasComponent<TransformComponent>()) {
            continue;
        }

        auto* lightComp = entity->getComponent<LightComponent>();
        auto* transformComp = entity->getComponent<TransformComponent>();

        // Skip disabled lights
        if (!lightComp->enabled) {
            continue;
        }

        // Only support Point lights for now (can extend later)
        if (lightComp->type != LightComponent::LightType::Point) {
            continue;
        }

        // Convert LightComponent + TransformComponent → LightData
        LightData lightData;
        lightData.position = transformComp->position;
        lightData.color = colorToVec3f(lightComp->color);
        lightData.radius = lightComp->radius;
        lightData.intensity = lightComp->intensity;
        lightData.enabled = lightComp->enabled;

        // Add light to effect
        m_lightingEffect->addLight(lightData);
    }
}

} // namespace NovaEngine
