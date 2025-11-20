#include "NovaEngine/Rendering/Effects/CRTEffect.hpp"
#include "NovaEngine/Core/Logger.hpp"

namespace NovaEngine {

CRTEffect::CRTEffect()
    : m_shader(INVALID_HANDLE)
    , m_ssaoShader(INVALID_HANDLE)
    , m_blurShader(INVALID_HANDLE)
    , m_aoTexture(INVALID_HANDLE)
    , m_blurredAOTexture(INVALID_HANDLE)
    , m_width(0)
    , m_height(0)
    , m_scanlineIntensity(0.0f)
    , m_pixelGridIntensity(0.0f)
    , m_chromaticAberration(0.0f)
    , m_rgbShiftAmount(0.0f)
    , m_curvature(0.0f)
    , m_aoStrength(0.4f)         // AO strength
    , m_aoRadius(12.0f)          // AO search radius
    , m_glowIntensity(0.4f)      // Bloom
    , m_noiseIntensity(0.0f)
    , m_colorBanding(0.0f)
    , m_saturation(1.3f)         // Saturation
    , m_ambientOcclusion(1.0f)   // Compatibilité
{
    m_startTime = std::chrono::high_resolution_clock::now();
}

CRTEffect::~CRTEffect() {
    shutdown();
}

bool CRTEffect::initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) {
    m_graphicsBackend = graphicsBackend;
    m_width = width;
    m_height = height;

    // Load main CRT shader
    m_shader = m_graphicsBackend->loadShader(
        "data/shaders/crt.vert",
        "data/shaders/crt.frag"
    );

    if(m_shader == INVALID_HANDLE) {
        LOG_ERROR("CRTEffect: Failed to load main shader");
        return false;
    }

    // Load SSAO shader
    m_ssaoShader = m_graphicsBackend->loadShader(
        "data/shaders/ssao.vert",
        "data/shaders/ssao.frag"
    );

    if(m_ssaoShader == INVALID_HANDLE) {
        LOG_WARN("CRTEffect: Failed to load SSAO shader, AO will be disabled");
    }

    // Load blur shader
    m_blurShader = m_graphicsBackend->loadShader(
        "data/shaders/ssao.vert",  // Réutilise le même vertex shader
        "data/shaders/blur.frag"
    );

    if(m_blurShader == INVALID_HANDLE) {
        LOG_WARN("CRTEffect: Failed to load blur shader");
    }

    // Create render textures for multi-pass AO (downsampled for performance)
    u32 aoWidth = width / 2;   // Half resolution for better performance
    u32 aoHeight = height / 2;

    if(m_ssaoShader != INVALID_HANDLE) {
        m_aoTexture = m_graphicsBackend->createRenderTexture(aoWidth, aoHeight);
        if(m_aoTexture == INVALID_HANDLE) {
            LOG_WARN("CRTEffect: Failed to create AO texture");
        }
    }

    if(m_blurShader != INVALID_HANDLE && m_aoTexture != INVALID_HANDLE) {
        m_blurredAOTexture = m_graphicsBackend->createRenderTexture(aoWidth, aoHeight);
        if(m_blurredAOTexture == INVALID_HANDLE) {
            LOG_WARN("CRTEffect: Failed to create blurred AO texture");
        }
    }

    // Initialiser les paramètres
    updateParameters();
    updateAOParameters();

    LOG_INFO("CRTEffect initialized ({}x{}, AO: {}x{})", width, height, aoWidth, aoHeight);
    return true;
}

void CRTEffect::shutdown() {
    if(m_shader != INVALID_HANDLE) {
        m_graphicsBackend->unloadShader(m_shader);
        m_shader = INVALID_HANDLE;
    }

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

void CRTEffect::apply(RenderTextureHandle renderTexture, f32 deltaTime) {
    if(m_shader == INVALID_HANDLE) {
        return;
    }

    // Mettre à jour le temps pour les effets animés
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - m_startTime;
    float time = elapsed.count();

    // ========================================================================
    // PASS 1: Generate SSAO map (if available)
    // ========================================================================
    if(m_ssaoShader != INVALID_HANDLE && m_aoTexture != INVALID_HANDLE && m_aoStrength > 0.0f) {
        // Clear and bind AO texture
        m_graphicsBackend->clearRenderTexture(m_aoTexture, Color::White);  // White = no occlusion
        m_graphicsBackend->bindRenderTexture(m_aoTexture);

        // Set SSAO shader parameters
        m_graphicsBackend->setShaderParameter(m_ssaoShader, "resolution",
            Vec2f(static_cast<f32>(m_width / 2), static_cast<f32>(m_height / 2)));

        // Draw scene with SSAO shader to generate AO map
        m_graphicsBackend->drawRenderTextureToScreen(renderTexture, m_ssaoShader);

        // Display and unbind
        m_graphicsBackend->displayRenderTexture(m_aoTexture);
        m_graphicsBackend->unbindRenderTexture();

        // ====================================================================
        // PASS 2: Blur the AO map (if available)
        // ====================================================================
        if(m_blurShader != INVALID_HANDLE && m_blurredAOTexture != INVALID_HANDLE) {
            // Clear and bind blurred AO texture
            m_graphicsBackend->clearRenderTexture(m_blurredAOTexture, Color::White);
            m_graphicsBackend->bindRenderTexture(m_blurredAOTexture);

            // Set blur shader parameters
            m_graphicsBackend->setShaderParameter(m_blurShader, "resolution",
                Vec2f(static_cast<f32>(m_width / 2), static_cast<f32>(m_height / 2)));
            m_graphicsBackend->setShaderParameter(m_blurShader, "blurRadius", 1.5f);

            // Blur the AO map
            m_graphicsBackend->drawRenderTextureToScreen(m_aoTexture, m_blurShader);

            // Display and unbind
            m_graphicsBackend->displayRenderTexture(m_blurredAOTexture);
            m_graphicsBackend->unbindRenderTexture();
        }
    }

    // ========================================================================
    // PASS 3: Final combine (Scene + Bloom + Saturation + AO)
    // ========================================================================

    // Send dynamic parameters
    m_graphicsBackend->setShaderParameter(m_shader, "time", time);
    m_graphicsBackend->setShaderParameter(m_shader, "resolution",
        Vec2f(static_cast<f32>(m_width), static_cast<f32>(m_height)));

    // TODO: Bind AO texture as secondary texture (need to extend IGraphicsBackend interface)
    // For now, draw without AO texture (will work but no AO effect)

    // Draw final result to screen
    m_graphicsBackend->drawRenderTextureToScreen(renderTexture, m_shader);
}

void CRTEffect::updateParameters() {
    if(m_shader == INVALID_HANDLE || !m_graphicsBackend) {
        return;
    }

    m_graphicsBackend->setShaderParameter(m_shader, "scanlineIntensity", m_scanlineIntensity);
    m_graphicsBackend->setShaderParameter(m_shader, "pixelGridIntensity", m_pixelGridIntensity);
    m_graphicsBackend->setShaderParameter(m_shader, "chromaticAberration", m_chromaticAberration);
    m_graphicsBackend->setShaderParameter(m_shader, "rgbShiftAmount", m_rgbShiftAmount);
    m_graphicsBackend->setShaderParameter(m_shader, "curvature", m_curvature);
    m_graphicsBackend->setShaderParameter(m_shader, "glowIntensity", m_glowIntensity);
    m_graphicsBackend->setShaderParameter(m_shader, "noiseIntensity", m_noiseIntensity);
    m_graphicsBackend->setShaderParameter(m_shader, "colorBanding", m_colorBanding);
    m_graphicsBackend->setShaderParameter(m_shader, "saturation", m_saturation);

    // Note: vignetteStrength removed, now using aoStrength separately
}

void CRTEffect::updateAOParameters() {
    if(m_ssaoShader == INVALID_HANDLE || !m_graphicsBackend) {
        return;
    }

    m_graphicsBackend->setShaderParameter(m_ssaoShader, "aoStrength", m_aoStrength);
    m_graphicsBackend->setShaderParameter(m_ssaoShader, "aoRadius", m_aoRadius);
}

}
