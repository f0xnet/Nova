#include "NovaEngine/Rendering/Effects/CRTEffect.hpp"
#include "NovaEngine/Core/Logger.hpp"

namespace NovaEngine {

CRTEffect::CRTEffect()
    : m_shader(INVALID_HANDLE)
    , m_ssaoShader(INVALID_HANDLE)
    , m_blurShader(INVALID_HANDLE)
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

    // Les textures scratch AO sont empruntées au pool partagé pendant apply(),
    // pas allouées ici, afin d'éviter la fragmentation de la mémoire GPU.

    // Initialiser les paramètres
    updateParameters();
    updateAOParameters();

    LOG_INFO("CRTEffect initialized ({}x{}, AO: {}x{})", width, height, width / 2, height / 2);
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
    // Les textures scratch AO sont gérées par le pool (pipeline) — pas à libérer ici.
}

void CRTEffect::apply(RenderTextureHandle inputTexture, RenderTextureHandle outputTexture, f32 deltaTime) {
    if(m_shader == INVALID_HANDLE) {
        return;
    }

    // Mettre à jour le temps pour les effets animés
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - m_startTime;
    float time = elapsed.count();

    // ========================================================================
    // PASS 1 & 2: SSAO + blur (textures empruntées au pool partagé)
    // ========================================================================
    u32 aoWidth  = m_width  / 2;
    u32 aoHeight = m_height / 2;

    RenderTextureHandle aoTex     = INVALID_HANDLE;
    RenderTextureHandle blurredTex = INVALID_HANDLE;

    if(m_ssaoShader != INVALID_HANDLE && m_pool && m_aoStrength > 0.0f) {
        aoTex = m_pool->acquire(aoWidth, aoHeight);
        if(aoTex != INVALID_HANDLE) {
            m_graphicsBackend->clearRenderTexture(aoTex, Color::White);
            m_graphicsBackend->bindRenderTexture(aoTex);
            m_graphicsBackend->setShaderParameter(m_ssaoShader, "resolution",
                Vec2f(static_cast<f32>(aoWidth), static_cast<f32>(aoHeight)));
            m_graphicsBackend->drawRenderTextureToScreen(inputTexture, m_ssaoShader);
            m_graphicsBackend->displayRenderTexture(aoTex);
            m_graphicsBackend->unbindRenderTexture();

            // PASS 2: blur the AO map
            if(m_blurShader != INVALID_HANDLE) {
                blurredTex = m_pool->acquire(aoWidth, aoHeight);
                if(blurredTex != INVALID_HANDLE) {
                    m_graphicsBackend->clearRenderTexture(blurredTex, Color::White);
                    m_graphicsBackend->bindRenderTexture(blurredTex);
                    m_graphicsBackend->setShaderParameter(m_blurShader, "resolution",
                        Vec2f(static_cast<f32>(aoWidth), static_cast<f32>(aoHeight)));
                    m_graphicsBackend->setShaderParameter(m_blurShader, "blurRadius", 1.5f);
                    m_graphicsBackend->drawRenderTextureToScreen(aoTex, m_blurShader);
                    m_graphicsBackend->displayRenderTexture(blurredTex);
                    m_graphicsBackend->unbindRenderTexture();
                }
            }
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

    // Draw final result
    if(outputTexture == INVALID_HANDLE) {
        m_graphicsBackend->drawRenderTextureToScreen(inputTexture, m_shader);
    } else {
        m_graphicsBackend->drawRenderTextureToRenderTexture(inputTexture, outputTexture, m_shader);
    }

    // Rendre les textures scratch au pool
    if(m_pool) {
        if(blurredTex != INVALID_HANDLE) m_pool->release(blurredTex);
        if(aoTex      != INVALID_HANDLE) m_pool->release(aoTex);
    }
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
