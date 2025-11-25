#include <NovaEngine/Systems/LightingSystem.hpp>
#include <NovaEngine/ECS/EntityRegistry.hpp>
#include <NovaEngine/Core/Logger.hpp>
#include <unordered_set>

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

    // Track which entities currently have lights
    std::unordered_set<u64> currentLightEntities;

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

        u64 entityID = entity->getID();

        // Skip disabled lights (but remove from effect if previously added)
        if (!lightComp->enabled) {
            auto it = m_entityToLightIndex.find(entityID);
            if (it != m_entityToLightIndex.end()) {
                // Light was previously enabled, now disabled → remove it
                m_lightingEffect->removeLight(it->second);
                m_entityToLightIndex.erase(it);
            }
            continue;
        }

        // Mark this entity as having a light
        currentLightEntities.insert(entityID);

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

        // Check if light already exists in effect
        auto it = m_entityToLightIndex.find(entityID);
        if (it != m_entityToLightIndex.end()) {
            // Light exists → update it
            m_lightingEffect->updateLight(it->second, lightData);
        } else {
            // New light → add it
            i32 lightIndex = m_lightingEffect->addLight(lightData);
            m_entityToLightIndex[entityID] = lightIndex;
        }
    }

    // Remove lights from entities that no longer exist or were deleted
    for (auto it = m_entityToLightIndex.begin(); it != m_entityToLightIndex.end(); ) {
        if (currentLightEntities.find(it->first) == currentLightEntities.end()) {
            // Entity no longer has a light → remove from effect
            m_lightingEffect->removeLight(it->second);
            it = m_entityToLightIndex.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace NovaEngine
