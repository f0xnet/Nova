#pragma once
#include "../PostProcessEffect.hpp"

namespace NovaEngine {

/**
 * @brief Color grading post-processing effect
 *
 * Ajustements de couleur: saturation, contraste, luminosité
 */
class ColorGradingEffect : public PostProcessEffect {
public:
    ColorGradingEffect();
    ~ColorGradingEffect() override;

    bool initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) override;
    void shutdown() override;
    void apply(RenderTextureHandle renderTexture, f32 deltaTime) override;
    const char* getName() const override { return "ColorGrading"; }

    // Paramètres ajustables
    void setSaturation(f32 saturation) { m_saturation = saturation; updateParameters(); }
    void setContrast(f32 contrast) { m_contrast = contrast; updateParameters(); }
    void setBrightness(f32 brightness) { m_brightness = brightness; updateParameters(); }

    f32 getSaturation() const { return m_saturation; }
    f32 getContrast() const { return m_contrast; }
    f32 getBrightness() const { return m_brightness; }

private:
    void updateParameters();

    ShaderHandle m_shader;
    u32 m_width;
    u32 m_height;

    f32 m_saturation;
    f32 m_contrast;
    f32 m_brightness;
};

}
