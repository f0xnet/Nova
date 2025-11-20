#pragma once

#include "../PostProcessEffect.hpp"
#include "LightData.hpp"
#include <vector>

namespace NovaEngine {

class DynamicLightingEffect : public PostProcessEffect {
public:
    static constexpr i32 MAX_LIGHTS = 8;

    DynamicLightingEffect();
    ~DynamicLightingEffect() override;

    bool initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) override;
    void shutdown() override;
    void apply(RenderTextureHandle inputTexture, RenderTextureHandle outputTexture, f32 deltaTime) override;
    const char* getName() const override { return "DynamicLighting"; }

    // Gestion des lumières
    i32 addLight(const LightData& light);
    void removeLight(i32 index);
    void updateLight(i32 index, const LightData& light);
    void clearLights();

    LightData* getLight(i32 index);
    i32 getLightCount() const { return static_cast<i32>(m_lights.size()); }

    // Paramètres globaux
    void setAmbientDarkness(f32 darkness) { m_ambientDarkness = darkness; }
    f32 getAmbientDarkness() const { return m_ambientDarkness; }

private:
    void updateShaderUniforms();

    ShaderHandle m_lightingShader;
    u32 m_width;
    u32 m_height;

    std::vector<LightData> m_lights;
    f32 m_ambientDarkness;  // 0.01 = très sombre, 1.0 = pas d'obscurité
};

}
