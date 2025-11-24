#include <NovaEngine/Systems/LightingSystem.hpp>
#include <NovaEngine/ECS/EntityRegistry.hpp>
#include <NovaEngine/Core/Logger.hpp>

namespace NovaEngine {

void LightingSystem::update(float deltaTime, EntityRegistry& registry) {
    if (!m_lightingEffect) {
        return; // No lighting effect to update
    }

    // Mettre à jour le cycle jour/nuit
    if (m_enableDayNightCycle) {
        m_elapsedTime += deltaTime;

        // Boucler le temps (0 -> dayDuration -> 0)
        if (m_elapsedTime >= m_dayDuration) {
            m_elapsedTime -= m_dayDuration;
        }

        // Calculer timeOfDay (0.0 = minuit, 0.5 = midi, 1.0 = minuit)
        f32 timeOfDay = m_elapsedTime / m_dayDuration;
        m_lightingEffect->setTimeOfDay(timeOfDay);
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

        // Convert LightComponent + TransformComponent → LightData
        LightData lightData;
        lightData.position = transformComp->position;
        lightData.color = colorToVec3f(lightComp->color);
        lightData.radius = lightComp->radius;
        lightData.intensity = lightComp->intensity;
        lightData.direction = lightComp->direction;
        lightData.angle = lightComp->angle;
        lightData.enabled = lightComp->enabled;

        // Convert LightComponent::LightType to LightData::LightType
        switch (lightComp->type) {
            case LightComponent::LightType::Point:
                lightData.type = LightData::LightType::Point;
                break;
            case LightComponent::LightType::Directional:
                lightData.type = LightData::LightType::Directional;
                break;
            case LightComponent::LightType::Spot:
                lightData.type = LightData::LightType::Spot;
                break;
        }

        // Add light to effect
        m_lightingEffect->addLight(lightData);
    }
}

} // namespace NovaEngine
