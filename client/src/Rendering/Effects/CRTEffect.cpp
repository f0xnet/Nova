#include "NovaEngine/Rendering/Effects/CRTEffect.hpp"
#include "NovaEngine/Core/Logger.hpp"

namespace NovaEngine {

CRTEffect::CRTEffect()
    : m_shader(INVALID_HANDLE)
    , m_width(0)
    , m_height(0)
    , m_scanlineIntensity(0.25f)
    , m_pixelGridIntensity(0.3f)
    , m_chromaticAberration(0.002f)
    , m_rgbShiftAmount(0.001f)
    , m_curvature(0.08f)
    , m_vignetteStrength(0.4f)
    , m_glowIntensity(0.5f)
    , m_noiseIntensity(0.04f)
    , m_colorBanding(0.08f)
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

    // Load CRT shader
    m_shader = m_graphicsBackend->loadShader(
        "data/shaders/crt.vert",
        "data/shaders/crt.frag"
    );

    if(m_shader == INVALID_HANDLE) {
        LOG_ERROR("CRTEffect: Failed to load shader");
        return false;
    }

    // Initialiser les paramètres
    updateParameters();

    LOG_INFO("CRTEffect initialized");
    return true;
}

void CRTEffect::shutdown() {
    if(m_shader != INVALID_HANDLE) {
        m_graphicsBackend->unloadShader(m_shader);
        m_shader = INVALID_HANDLE;
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

    // Envoyer les paramètres dynamiques
    m_graphicsBackend->setShaderParameter(m_shader, "time", time);
    m_graphicsBackend->setShaderParameter(m_shader, "resolution",
        Vec2f(static_cast<f32>(m_width), static_cast<f32>(m_height)));

    // Dessiner avec le shader
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
    m_graphicsBackend->setShaderParameter(m_shader, "vignetteStrength", m_vignetteStrength);
    m_graphicsBackend->setShaderParameter(m_shader, "glowIntensity", m_glowIntensity);
    m_graphicsBackend->setShaderParameter(m_shader, "noiseIntensity", m_noiseIntensity);
    m_graphicsBackend->setShaderParameter(m_shader, "colorBanding", m_colorBanding);
}

}
