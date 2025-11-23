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
    f32 m_elapsedTime;      // Temps écoulé depuis le début du cycle (en secondes)
    f32 m_dayDuration;      // Durée d'un cycle jour/nuit complet (en secondes)
    bool m_enableDayNightCycle;  // Activer/désactiver le cycle automatique

public:
    LightingSystem()
        : m_lightingEffect(nullptr)
        , m_elapsedTime(0.0f)
        , m_dayDuration(120.0f)      // 2 minutes par défaut pour un cycle complet
        , m_enableDayNightCycle(true)
    {}

    /**
     * @brief Set the lighting effect to update
     * @param effect Pointer to the DynamicLightingEffect (must remain valid)
     */
    void setLightingEffect(DynamicLightingEffect* effect) {
        m_lightingEffect = effect;
    }

    /**
     * @brief Configure le cycle jour/nuit
     * @param enabled Activer ou désactiver le cycle automatique
     * @param duration Durée d'un cycle complet en secondes (défaut: 120s = 2min)
     */
    void setDayNightCycle(bool enabled, f32 duration = 120.0f) {
        m_enableDayNightCycle = enabled;
        m_dayDuration = duration;
    }

    /**
     * @brief Définir manuellement l'heure de la journée
     * @param timeOfDay Heure (0.0 = minuit, 0.5 = midi, 1.0 = minuit)
     */
    void setTimeOfDay(f32 timeOfDay) {
        if (m_lightingEffect) {
            m_lightingEffect->setTimeOfDay(timeOfDay);
        }
        // Synchroniser l'elapsed time avec le timeOfDay manuel
        m_elapsedTime = timeOfDay * m_dayDuration;
    }

    /**
     * @brief Obtenir l'heure actuelle de la journée
     * @return Heure (0.0 = minuit, 0.5 = midi, 1.0 = minuit)
     */
    f32 getTimeOfDay() const {
        return m_lightingEffect ? m_lightingEffect->getTimeOfDay() : 0.5f;
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
