#pragma once
#include "../PostProcessEffect.hpp"
#include <chrono>

namespace NovaEngine {

/**
 * @brief Effet CRT (écran cathodique) vintage
 *
 * Simule l'apparence d'un vieux moniteur CRT avec :
 * - Courbure de l'écran
 * - Scanlines animées
 * - Vignettage
 * - Aberration chromatique
 * - Et bien d'autres effets
 */
class CRTEffect : public PostProcessEffect {
public:
    CRTEffect();
    ~CRTEffect() override;

    bool initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) override;
    void shutdown() override;
    void apply(RenderTextureHandle inputTexture, RenderTextureHandle outputTexture, f32 deltaTime) override;
    const char* getName() const override { return "CRT"; }

    // Paramètres ajustables
    void setScanlineIntensity(f32 intensity) { m_scanlineIntensity = intensity; updateParameters(); }
    void setPixelGridIntensity(f32 intensity) { m_pixelGridIntensity = intensity; updateParameters(); }
    void setChromaticAberration(f32 amount) { m_chromaticAberration = amount; updateParameters(); }
    void setRGBShiftAmount(f32 amount) { m_rgbShiftAmount = amount; updateParameters(); }
    void setCurvature(f32 curvature) { m_curvature = curvature; updateParameters(); }
    void setAOStrength(f32 strength) { m_aoStrength = strength; updateAOParameters(); }
    void setAORadius(f32 radius) { m_aoRadius = radius; updateAOParameters(); }
    void setGlowIntensity(f32 intensity) { m_glowIntensity = intensity; updateParameters(); }
    void setNoiseIntensity(f32 intensity) { m_noiseIntensity = intensity; updateParameters(); }
    void setColorBanding(f32 banding) { m_colorBanding = banding; updateParameters(); }
    void setSaturation(f32 saturation) { m_saturation = saturation; updateParameters(); }

private:
    void updateParameters();
    void updateAOParameters();

    // Main shader
    ShaderHandle m_shader;

    // Multi-pass SSAO
    ShaderHandle m_ssaoShader;
    ShaderHandle m_blurShader;
    RenderTextureHandle m_aoTexture;
    RenderTextureHandle m_blurredAOTexture;

    u32 m_width;
    u32 m_height;

    // Paramètres CRT
    f32 m_scanlineIntensity;
    f32 m_pixelGridIntensity;
    f32 m_chromaticAberration;
    f32 m_rgbShiftAmount;
    f32 m_curvature;
    f32 m_aoStrength;
    f32 m_aoRadius;
    f32 m_glowIntensity;
    f32 m_noiseIntensity;
    f32 m_colorBanding;
    f32 m_saturation;
    f32 m_ambientOcclusion;  // Gardé pour compatibilité

    std::chrono::high_resolution_clock::time_point m_startTime;
};

}
