#pragma once
#include "../PostProcessEffect.hpp"

namespace NovaEngine {

/**
 * @brief Bloom/Glow post-processing effect
 *
 * Applique un effet de glow sur les zones lumineuses de l'image
 */
class BloomEffect : public PostProcessEffect {
public:
    BloomEffect();
    ~BloomEffect() override;

    bool initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) override;
    void shutdown() override;
    void apply(RenderTextureHandle inputTexture, RenderTextureHandle outputTexture, f32 deltaTime) override;
    const char* getName() const override { return "Bloom"; }

    // Paramètres ajustables
    void setIntensity(f32 intensity) { m_intensity = intensity; updateParameters(); }
    f32 getIntensity() const { return m_intensity; }

private:
    void updateParameters();

    ShaderHandle m_shader;
    u32 m_width;
    u32 m_height;
    f32 m_intensity;
};

}
