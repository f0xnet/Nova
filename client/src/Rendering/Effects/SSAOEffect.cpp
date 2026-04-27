#include "NovaEngine/Rendering/Effects/SSAOEffect.hpp"
#include "NovaEngine/Core/Logger.hpp"

namespace NovaEngine {

SSAOEffect::SSAOEffect()
    : m_ssaoShader(INVALID_HANDLE)
    , m_width(0)
    , m_height(0)
    , m_strength(1.0f)
    , m_radius(50.0f)
{
}

SSAOEffect::~SSAOEffect() {
    shutdown();
}

bool SSAOEffect::initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) {
    m_graphicsBackend = graphicsBackend;
    m_width = width;
    m_height = height;

    // Load SSAO shader (single-pass, no intermediate textures needed)
    m_ssaoShader = m_graphicsBackend->loadShader(
        "data/shaders/ssao.vert",
        "data/shaders/ssao.frag"
    );

    if(m_ssaoShader == INVALID_HANDLE) {
        LOG_ERROR("SSAOEffect: Failed to load SSAO shader");
        return false;
    }

    updateParameters();

    LOG_INFO("SSAOEffect initialized ({}x{}, single-pass)", width, height);
    return true;
}

void SSAOEffect::shutdown() {
    if(m_ssaoShader != INVALID_HANDLE) {
        m_graphicsBackend->unloadShader(m_ssaoShader);
        m_ssaoShader = INVALID_HANDLE;
    }
}

void SSAOEffect::apply(RenderTextureHandle inputTexture, RenderTextureHandle outputTexture, f32 deltaTime) {
    if(m_ssaoShader == INVALID_HANDLE) {
        // Fallback: draw without AO
        if(outputTexture == INVALID_HANDLE) {
            m_graphicsBackend->drawRenderTextureToScreen(inputTexture, INVALID_HANDLE);
        } else {
            m_graphicsBackend->drawRenderTextureToRenderTexture(inputTexture, outputTexture, INVALID_HANDLE);
        }
        return;
    }

    // Set shader parameters (must be set before each draw)
    m_graphicsBackend->setShaderParameter(m_ssaoShader, "resolution",
        Vec2f(static_cast<f32>(m_width), static_cast<f32>(m_height)));
    m_graphicsBackend->setShaderParameter(m_ssaoShader, "aoStrength", m_strength);
    m_graphicsBackend->setShaderParameter(m_ssaoShader, "aoRadius", m_radius);

    // Apply SSAO shader (computes + applies AO in single pass)
    if(outputTexture == INVALID_HANDLE) {
        m_graphicsBackend->drawRenderTextureToScreen(inputTexture, m_ssaoShader);
    } else {
        m_graphicsBackend->drawRenderTextureToRenderTexture(inputTexture, outputTexture, m_ssaoShader);
    }
}

void SSAOEffect::updateParameters() {
    if(m_ssaoShader == INVALID_HANDLE || !m_graphicsBackend) {
        return;
    }

    m_graphicsBackend->setShaderParameter(m_ssaoShader, "aoStrength", m_strength);
    m_graphicsBackend->setShaderParameter(m_ssaoShader, "aoRadius", m_radius);
}

}
