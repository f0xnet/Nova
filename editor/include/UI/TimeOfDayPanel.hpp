#pragma once

#include "UIPanel.hpp"
#include <NovaEngine/Systems/LightingSystem.hpp>
#include <NovaEngine/Rendering/Effects/DynamicLightingEffect.hpp>

namespace NovaEditor {

/**
 * @brief Panel for controlling time of day and ambient lighting
 *
 * Before refactoring: 50+ lines of boilerplate show/hide/update code
 * After refactoring: 30 lines of clean, focused logic
 */
class TimeOfDayPanel : public UIPanel {
public:
    TimeOfDayPanel(
        NovaEngine::UIManager& uiManager,
        NovaEngine::LightingSystem* lightingSystem,
        NovaEngine::DynamicLightingEffect* lightingEffect
    )
        : UIPanel(uiManager, "time_panel")
        , m_lightingSystem(lightingSystem)
        , m_lightingEffect(lightingEffect)
    {}

    void update() override {
        if (!m_lightingEffect) return;

        // Update time display
        float timeOfDay = m_lightingSystem ? m_lightingSystem->getTimeOfDay() : 0.5f;
        setText("time_value", formatTimeOfDay(timeOfDay));

        // Update ambient darkness display
        float ambient = m_lightingEffect->getAmbientDarkness();
        setText("ambient_value", formatFloat(ambient, 2));
    }

    // ===== TIME OF DAY CONTROLS =====

    void setTimeOfDay(float time) {
        if (!m_lightingSystem || !m_lightingEffect) {
            LOG_WARN("Lighting system not initialized");
            return;
        }

        // Clamp to valid range
        time = std::max(0.0f, std::min(1.0f, time));

        m_lightingSystem->setTimeOfDay(time);
        m_lightingEffect->setTimeOfDay(time);

        LOG_INFO("Time of day set to: {}", time);
        update();
    }

    void adjustAmbientDarkness(float delta) {
        if (!m_lightingEffect) {
            LOG_WARN("Lighting effect not initialized");
            return;
        }

        float current = m_lightingEffect->getAmbientDarkness();
        float newValue = std::max(0.01f, std::min(1.0f, current + delta));

        m_lightingEffect->setAmbientDarkness(newValue);

        LOG_INFO("Ambient darkness adjusted: {} -> {}", current, newValue);
        update();
    }

private:
    NovaEngine::LightingSystem* m_lightingSystem;
    NovaEngine::DynamicLightingEffect* m_lightingEffect;

    /**
     * @brief Format time of day value to readable string
     */
    std::string formatTimeOfDay(float time) const {
        // Named periods for better UX
        if (time < 0.05f || time > 0.95f) return "Midnight";
        if (time >= 0.2f && time <= 0.3f) return "Dawn";
        if (time >= 0.45f && time <= 0.55f) return "Noon";
        if (time >= 0.7f && time <= 0.8f) return "Dusk";

        // Otherwise show HH:MM format
        int hours = static_cast<int>(time * 24.0f);
        int minutes = static_cast<int>((time * 24.0f - hours) * 60.0f);

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%02d:%02d", hours, minutes);
        return std::string(buffer);
    }
};

} // namespace NovaEditor
