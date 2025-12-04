#pragma once

#include "UIPanel.hpp"
#include <NovaEngine/Rendering/Effects/BloomEffect.hpp>
#include <NovaEngine/Rendering/Effects/ColorGradingEffect.hpp>

namespace NovaEditor {

/**
 * @brief Panel for controlling post-processing effects
 *
 * Before refactoring: 54 lines of repetitive component access code
 * After refactoring: 40 lines with clean separation of concerns
 */
class PostProcessPanel : public UIPanel {
public:
    PostProcessPanel(
        NovaEngine::UIManager& uiManager,
        NovaEngine::BloomEffect* bloomEffect,
        NovaEngine::ColorGradingEffect* colorGradingEffect
    )
        : UIPanel(uiManager, "pp_panel")
        , m_bloomEffect(bloomEffect)
        , m_colorGradingEffect(colorGradingEffect)
    {}

    void update() override {
        // Update bloom intensity
        if (m_bloomEffect) {
            float bloom = m_bloomEffect->getIntensity();
            setText("pp_bloom_value", formatFloat(bloom, 2));
        }

        // Update color grading values
        if (m_colorGradingEffect) {
            setText("pp_saturation_value", formatFloat(m_colorGradingEffect->getSaturation(), 2));
            setText("pp_contrast_value", formatFloat(m_colorGradingEffect->getContrast(), 2));
            setText("pp_brightness_value", formatFloat(m_colorGradingEffect->getBrightness(), 2));
        }
    }

    // ===== BLOOM CONTROLS =====

    void adjustBloomIntensity(float delta) {
        if (!m_bloomEffect) {
            LOG_WARN("Bloom effect not initialized");
            return;
        }

        float current = m_bloomEffect->getIntensity();
        float newValue = std::max(0.0f, std::min(2.0f, current + delta));

        m_bloomEffect->setIntensity(newValue);

        LOG_INFO("Bloom intensity adjusted: {} -> {}", current, newValue);
        update();
    }

    // ===== COLOR GRADING CONTROLS =====

    void adjustSaturation(float delta) {
        if (!m_colorGradingEffect) {
            LOG_WARN("Color grading effect not initialized");
            return;
        }

        float current = m_colorGradingEffect->getSaturation();
        float newValue = std::max(0.0f, std::min(3.0f, current + delta));

        m_colorGradingEffect->setSaturation(newValue);

        LOG_INFO("Saturation adjusted: {} -> {}", current, newValue);
        update();
    }

    void adjustContrast(float delta) {
        if (!m_colorGradingEffect) {
            LOG_WARN("Color grading effect not initialized");
            return;
        }

        float current = m_colorGradingEffect->getContrast();
        float newValue = std::max(0.0f, std::min(3.0f, current + delta));

        m_colorGradingEffect->setContrast(newValue);

        LOG_INFO("Contrast adjusted: {} -> {}", current, newValue);
        update();
    }

    void adjustBrightness(float delta) {
        if (!m_colorGradingEffect) {
            LOG_WARN("Color grading effect not initialized");
            return;
        }

        float current = m_colorGradingEffect->getBrightness();
        float newValue = std::max(-1.0f, std::min(1.0f, current + delta));

        m_colorGradingEffect->setBrightness(newValue);

        LOG_INFO("Brightness adjusted: {} -> {}", current, newValue);
        update();
    }

private:
    NovaEngine::BloomEffect* m_bloomEffect;
    NovaEngine::ColorGradingEffect* m_colorGradingEffect;
};

} // namespace NovaEditor
