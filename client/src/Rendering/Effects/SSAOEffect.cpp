#include "NovaEngine/Rendering/Effects/SSAOEffect.hpp"
#include "NovaEngine/Core/Logger.hpp"

namespace NovaEngine {

SSAOEffect::SSAOEffect()
    : m_ssaoShader(INVALID_HANDLE)
    , m_blurShader(INVALID_HANDLE)
    , m_aoTexture(INVALID_HANDLE)
    , m_blurredAOTexture(INVALID_HANDLE)
    , m_width(0)
    , m_height(0)
    , m_downsampleFactor(2)
    , m_strength(0.4f)
    , m_radius(12.0f)
{
}

SSAOEffect::~SSAOEffect() {
    shutdown();
}

bool SSAOEffect::initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) {
    m_graphicsBackend = graphicsBackend;
    m_width = width;
    m_height = height;

    // Load SSAO shader
    m_ssaoShader = m_graphicsBackend->loadShader(
        "data/shaders/ssao.vert",
        "data/shaders/ssao.frag"
    );

    if(m_ssaoShader == INVALID_HANDLE) {
        LOG_ERROR("SSAOEffect: Failed to load SSAO shader");
        return false;
    }

    // Load blur shader
    m_blurShader = m_graphicsBackend->loadShader(
        "data/shaders/ssao.vert",
        "data/shaders/blur.frag"
    );

    if(m_blurShader == INVALID_HANDLE) {
        LOG_WARN("SSAOEffect: Failed to load blur shader");
    }

    // Create downsampled render textures for performance
    u32 aoWidth = width / m_downsampleFactor;
    u32 aoHeight = height / m_downsampleFactor;

    m_aoTexture = m_graphicsBackend->createRenderTexture(aoWidth, aoHeight);
    if(m_aoTexture == INVALID_HANDLE) {
        LOG_ERROR("SSAOEffect: Failed to create AO texture");
        return false;
    }

    if(m_blurShader != INVALID_HANDLE) {
        m_blurredAOTexture = m_graphicsBackend->createRenderTexture(aoWidth, aoHeight);
        if(m_blurredAOTexture == INVALID_HANDLE) {
            LOG_WARN("SSAOEffect: Failed to create blurred AO texture");
        }
    }

    updateParameters();

    LOG_INFO("SSAOEffect initialized ({}x{}, AO: {}x{})", width, height, aoWidth, aoHeight);
    return true;
}

void SSAOEffect::shutdown() {
    if(m_ssaoShader != INVALID_HANDLE) {
        m_graphicsBackend->unloadShader(m_ssaoShader);
        m_ssaoShader = INVALID_HANDLE;
    }

    if(m_blurShader != INVALID_HANDLE) {
        m_graphicsBackend->unloadShader(m_blurShader);
        m_blurShader = INVALID_HANDLE;
    }

    if(m_aoTexture != INVALID_HANDLE) {
        m_graphicsBackend->unloadRenderTexture(m_aoTexture);
        m_aoTexture = INVALID_HANDLE;
    }

    if(m_blurredAOTexture != INVALID_HANDLE) {
        m_graphicsBackend->unloadRenderTexture(m_blurredAOTexture);
        m_blurredAOTexture = INVALID_HANDLE;
    }
}

void SSAOEffect::apply(RenderTextureHandle renderTexture, f32 deltaTime) {
    if(m_ssaoShader == INVALID_HANDLE || m_aoTexture == INVALID_HANDLE) {
        // Fallback: draw without AO
        m_graphicsBackend->drawRenderTextureToScreen(renderTexture, INVALID_HANDLE);
        return;
    }

    u32 aoWidth = m_width / m_downsampleFactor;
    u32 aoHeight = m_height / m_downsampleFactor;

    // ========================================================================
    // PASS 1: Generate SSAO map
    // ========================================================================
    m_graphicsBackend->clearRenderTexture(m_aoTexture, Color::White);
    m_graphicsBackend->bindRenderTexture(m_aoTexture);

    // Set SSAO parameters
    m_graphicsBackend->setShaderParameter(m_ssaoShader, "resolution",
        Vec2f(static_cast<f32>(aoWidth), static_cast<f32>(aoHeight)));

    // Render scene with SSAO shader
    m_graphicsBackend->drawRenderTextureToScreen(renderTexture, m_ssaoShader);

    m_graphicsBackend->displayRenderTexture(m_aoTexture);
    m_graphicsBackend->unbindRenderTexture();

    // ========================================================================
    // PASS 2: Blur AO map (optional)
    // ========================================================================
    RenderTextureHandle finalAO = m_aoTexture;

    if(m_blurShader != INVALID_HANDLE && m_blurredAOTexture != INVALID_HANDLE) {
        m_graphicsBackend->clearRenderTexture(m_blurredAOTexture, Color::White);
        m_graphicsBackend->bindRenderTexture(m_blurredAOTexture);

        m_graphicsBackend->setShaderParameter(m_blurShader, "resolution",
            Vec2f(static_cast<f32>(aoWidth), static_cast<f32>(aoHeight)));
        m_graphicsBackend->setShaderParameter(m_blurShader, "blurRadius", 1.5f);

        m_graphicsBackend->drawRenderTextureToScreen(m_aoTexture, m_blurShader);

        m_graphicsBackend->displayRenderTexture(m_blurredAOTexture);
        m_graphicsBackend->unbindRenderTexture();

        finalAO = m_blurredAOTexture;
    }

    // ========================================================================
    // PASS 3: Draw AO result to screen (for now, just the AO map)
    // TODO: Need to combine with original scene - requires multi-texture support
    // ========================================================================
    m_graphicsBackend->drawRenderTextureToScreen(finalAO, INVALID_HANDLE);
}

void SSAOEffect::updateParameters() {
    if(m_ssaoShader == INVALID_HANDLE || !m_graphicsBackend) {
        return;
    }

    m_graphicsBackend->setShaderParameter(m_ssaoShader, "aoStrength", m_strength);
    m_graphicsBackend->setShaderParameter(m_ssaoShader, "aoRadius", m_radius);
}

}
