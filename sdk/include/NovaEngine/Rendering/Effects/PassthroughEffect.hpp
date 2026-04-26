#pragma once
#include "../PostProcessEffect.hpp"

namespace NovaEngine {

/**
 * @brief Effet passthrough simple (aucun traitement)
 *
 * Utile pour désactiver temporairement les effets ou pour tester
 */
class PassthroughEffect : public PostProcessEffect {
public:
    PassthroughEffect() = default;
    ~PassthroughEffect() override = default;

    bool initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) override {
        m_graphicsBackend = graphicsBackend;
        return true;
    }

    void shutdown() override {}

    void apply(RenderTextureHandle inputTexture, RenderTextureHandle outputTexture, f32 deltaTime) override {
        if(outputTexture == INVALID_HANDLE)
            m_graphicsBackend->drawRenderTextureToScreen(inputTexture, INVALID_HANDLE);
        else
            m_graphicsBackend->drawRenderTextureToRenderTexture(inputTexture, outputTexture, INVALID_HANDLE);
    }

    const char* getName() const override { return "Passthrough"; }
};

}
