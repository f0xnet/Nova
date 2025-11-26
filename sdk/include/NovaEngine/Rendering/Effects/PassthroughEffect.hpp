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

    void apply(RenderTextureHandle renderTexture, f32 deltaTime) override {
        // Dessiner sans shader (passthrough)
        m_graphicsBackend->drawRenderTextureToScreen(renderTexture, INVALID_HANDLE);
    }

    const char* getName() const override { return "Passthrough"; }
};

}
