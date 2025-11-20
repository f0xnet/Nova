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

void BloomEffect::apply(RenderTextureHandle inputTexture, RenderTextureHandle outputTexture, f32 deltaTime) {
    if(m_shader == INVALID_HANDLE) {
        // Fallback: draw without bloom
        if(outputTexture == INVALID_HANDLE) {
            m_graphicsBackend->drawRenderTextureToScreen(inputTexture, INVALID_HANDLE);
        } else {
            m_graphicsBackend->drawRenderTextureToRenderTexture(inputTexture, outputTexture, INVALID_HANDLE);
        }
        return;
    }

    // Set shader parameters (must be set before each draw)
    m_graphicsBackend->setShaderParameter(m_shader, "resolution",
        Vec2f(static_cast<f32>(m_width), static_cast<f32>(m_height)));
    m_graphicsBackend->setShaderParameter(m_shader, "intensity", m_intensity);

    // Draw with bloom
    if(outputTexture == INVALID_HANDLE) {
        m_graphicsBackend->drawRenderTextureToScreen(inputTexture, m_shader);
    } else {
        m_graphicsBackend->drawRenderTextureToRenderTexture(inputTexture, outputTexture, m_shader);
    }
}

void BloomEffect::updateParameters() {
    if(m_shader == INVALID_HANDLE || !m_graphicsBackend) {
        return;
    }

    m_graphicsBackend->setShaderParameter(m_shader, "intensity", m_intensity);
}

}
