#include "NovaEngine/Rendering/Effects/BloomEffect.hpp"
#include "NovaEngine/Core/Logger.hpp"

namespace NovaEngine {

BloomEffect::BloomEffect()
    : m_shader(INVALID_HANDLE)
    , m_width(0)
    , m_height(0)
    , m_intensity(0.4f)
{
}

BloomEffect::~BloomEffect() {
    shutdown();
}

bool BloomEffect::initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) {
    m_graphicsBackend = graphicsBackend;
    m_width = width;
    m_height = height;

    // Load bloom shader
    m_shader = m_graphicsBackend->loadShader(
        "data/shaders/crt.vert",  // Utilise le vertex shader standard
        "data/shaders/bloom.frag"
    );

    if(m_shader == INVALID_HANDLE) {
        LOG_ERROR("BloomEffect: Failed to load shader");
        return false;
    }

    updateParameters();

    LOG_INFO("BloomEffect initialized");
    return true;
}

void BloomEffect::shutdown() {
    if(m_shader != INVALID_HANDLE) {
        m_graphicsBackend->unloadShader(m_shader);
        m_shader = INVALID_HANDLE;
    }
}

void BloomEffect::apply(RenderTextureHandle renderTexture, f32 deltaTime) {
    if(m_shader == INVALID_HANDLE) {
        // Fallback: draw without bloom
        m_graphicsBackend->drawRenderTextureToScreen(renderTexture, INVALID_HANDLE);
        return;
    }

    // Set shader parameters
    m_graphicsBackend->setShaderParameter(m_shader, "resolution",
        Vec2f(static_cast<f32>(m_width), static_cast<f32>(m_height)));

    // Draw with bloom
    m_graphicsBackend->drawRenderTextureToScreen(renderTexture, m_shader);
}

void BloomEffect::updateParameters() {
    if(m_shader == INVALID_HANDLE || !m_graphicsBackend) {
        return;
    }

    m_graphicsBackend->setShaderParameter(m_shader, "intensity", m_intensity);
}

}
