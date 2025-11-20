#pragma once

#include <NovaEngine/Rendering/PostProcessEffect.hpp>
#include <NovaEngine/Core/Types.hpp>
#include "LightData.hpp"
#include <vector>

namespace NovaEngine {
    class IGraphicsBackend;
}

class DynamicLightingEffect : public NovaEngine::PostProcessEffect {
public:
    static constexpr NovaEngine::i32 MAX_LIGHTS = 8;

    DynamicLightingEffect(NovaEngine::IGraphicsBackend* graphicsBackend);
    ~DynamicLightingEffect() override = default;

    void apply(NovaEngine::RenderTextureHandle inputTexture,
               NovaEngine::RenderTextureHandle outputTexture,
               NovaEngine::f32 deltaTime) override;

    // Gestion des lumières
    NovaEngine::i32 addLight(const LightData& light);
    void removeLight(NovaEngine::i32 index);
    void updateLight(NovaEngine::i32 index, const LightData& light);
    void clearLights();

    LightData* getLight(NovaEngine::i32 index);
    NovaEngine::i32 getLightCount() const { return static_cast<NovaEngine::i32>(m_lights.size()); }

    // Paramètres globaux
    void setAmbientDarkness(NovaEngine::f32 darkness) { m_ambientDarkness = darkness; }
    NovaEngine::f32 getAmbientDarkness() const { return m_ambientDarkness; }

private:
    void updateShaderUniforms();

    NovaEngine::IGraphicsBackend* m_graphicsBackend;
    NovaEngine::ShaderHandle m_lightingShader;

    std::vector<LightData> m_lights;
    NovaEngine::f32 m_ambientDarkness;  // 0.01 = très sombre, 1.0 = pas d'obscurité
};
