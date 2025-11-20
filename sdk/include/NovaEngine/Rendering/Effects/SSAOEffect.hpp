#pragma once
#include "../PostProcessEffect.hpp"

namespace NovaEngine {

/**
 * @brief Screen-Space Ambient Occlusion effect
 *
 * Détecte les bords des objets et applique des ombres qui s'estompent
 * pour créer de la profondeur autour des objets.
 */
class SSAOEffect : public PostProcessEffect {
public:
    SSAOEffect();
    ~SSAOEffect() override;

    bool initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) override;
    void shutdown() override;
    void apply(RenderTextureHandle renderTexture, f32 deltaTime) override;
    const char* getName() const override { return "SSAO"; }

    // Paramètres ajustables
    void setStrength(f32 strength) { m_strength = strength; updateParameters(); }
    void setRadius(f32 radius) { m_radius = radius; updateParameters(); }
    void setDownsampleFactor(u32 factor) { m_downsampleFactor = factor; }

    f32 getStrength() const { return m_strength; }
    f32 getRadius() const { return m_radius; }

private:
    void updateParameters();

    ShaderHandle m_ssaoShader;
    ShaderHandle m_blurShader;
    RenderTextureHandle m_aoTexture;
    RenderTextureHandle m_blurredAOTexture;

    u32 m_width;
    u32 m_height;
    u32 m_downsampleFactor;  // 2 = half res, 4 = quarter res

    f32 m_strength;
    f32 m_radius;
};

}
