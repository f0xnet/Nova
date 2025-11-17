#include "NovaEngine/Rendering/PostProcessManager.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include <chrono>

namespace NovaEngine {

PostProcessManager::PostProcessManager(IGraphicsBackend* graphicsBackend)
    : m_graphicsBackend(graphicsBackend)
    , m_renderTexture(INVALID_HANDLE)
    , m_crtShader(INVALID_HANDLE)
    , m_crtEnabled(true)
    , m_width(0)
    , m_height(0)
    , m_scanlineIntensity(0.15f)
    , m_pixelGridIntensity(0.2f)
    , m_chromaticAberration(0.001f)
    , m_rgbShiftAmount(0.0005f)
    , m_curvature(0.15f)
    , m_vignetteStrength(0.3f)
    , m_glowIntensity(0.4f)
    , m_noiseIntensity(0.03f)
    , m_colorBanding(0.05f)
{
    m_startTime = std::chrono::high_resolution_clock::now();
}

PostProcessManager::~PostProcessManager() {
    shutdown();
}

bool PostProcessManager::initialize(u32 width, u32 height) {
    m_width = width;
    m_height = height;

    // Créer la render texture
    m_renderTexture = m_graphicsBackend->createRenderTexture(width, height);
    if(m_renderTexture == INVALID_HANDLE) {
        return false;
    }

    // Charger le shader CRT
    m_crtShader = m_graphicsBackend->loadShader(
        "data/shaders/crt.vert",
        "data/shaders/crt.frag"
    );

    if(m_crtShader == INVALID_HANDLE) {
        LOG_ERROR("Failed to load CRT shader! Check that data/shaders/crt.vert and crt.frag exist");
        return false;
    }

    LOG_INFO("CRT shader loaded successfully");

    // Initialiser les paramètres du shader
    updateShaderParameters();

    return true;
}

void PostProcessManager::shutdown() {
    if(m_renderTexture != INVALID_HANDLE) {
        m_graphicsBackend->unloadRenderTexture(m_renderTexture);
        m_renderTexture = INVALID_HANDLE;
    }
    if(m_crtShader != INVALID_HANDLE) {
        m_graphicsBackend->unloadShader(m_crtShader);
        m_crtShader = INVALID_HANDLE;
    }
}

void PostProcessManager::beginFrame() {
    if(!m_crtEnabled || m_renderTexture == INVALID_HANDLE) {
        return;
    }

    // Clear et bind la render texture
    m_graphicsBackend->clearRenderTexture(m_renderTexture, Color::Black);
    m_graphicsBackend->bindRenderTexture(m_renderTexture);
}

void PostProcessManager::endFrame() {
    if(!m_crtEnabled || m_renderTexture == INVALID_HANDLE || m_crtShader == INVALID_HANDLE) {
        return;
    }

    // Finaliser la render texture
    m_graphicsBackend->displayRenderTexture(m_renderTexture);

    // Unbind pour revenir à la fenêtre
    m_graphicsBackend->unbindRenderTexture();

    // Mettre à jour le temps pour les effets animés
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - m_startTime;
    float time = elapsed.count();

    // Mettre à jour les paramètres du shader avant le rendu
    m_graphicsBackend->setShaderParameter(m_crtShader, "time", time);
    m_graphicsBackend->setShaderParameter(m_crtShader, "resolution", Vec2f(static_cast<f32>(m_width), static_cast<f32>(m_height)));

    // Dessiner la render texture sur la fenêtre avec le shader CRT
    m_graphicsBackend->drawRenderTextureToScreen(m_renderTexture, m_crtShader);
}

void PostProcessManager::updateShaderParameters() {
    if(m_crtShader == INVALID_HANDLE) return;

    m_graphicsBackend->setShaderParameter(m_crtShader, "scanlineIntensity", m_scanlineIntensity);
    m_graphicsBackend->setShaderParameter(m_crtShader, "pixelGridIntensity", m_pixelGridIntensity);
    m_graphicsBackend->setShaderParameter(m_crtShader, "chromaticAberration", m_chromaticAberration);
    m_graphicsBackend->setShaderParameter(m_crtShader, "rgbShiftAmount", m_rgbShiftAmount);
    m_graphicsBackend->setShaderParameter(m_crtShader, "curvature", m_curvature);
    m_graphicsBackend->setShaderParameter(m_crtShader, "vignetteStrength", m_vignetteStrength);
    m_graphicsBackend->setShaderParameter(m_crtShader, "glowIntensity", m_glowIntensity);
    m_graphicsBackend->setShaderParameter(m_crtShader, "noiseIntensity", m_noiseIntensity);
    m_graphicsBackend->setShaderParameter(m_crtShader, "colorBanding", m_colorBanding);
}

}
