#include "NovaEngine/Rendering/PostProcessPipeline.hpp"
#include "NovaEngine/Core/Logger.hpp"

namespace NovaEngine {

PostProcessPipeline::PostProcessPipeline(IGraphicsBackend* graphicsBackend)
    : m_graphicsBackend(graphicsBackend)
    , m_renderTexture(INVALID_HANDLE)
    , m_width(0)
    , m_height(0)
    , m_initialized(false)
    , m_enabled(true)
{
}

PostProcessPipeline::~PostProcessPipeline() {
    shutdown();
}

bool PostProcessPipeline::initialize(u32 width, u32 height) {
    m_width = width;
    m_height = height;

    // Créer la render texture pour la scène
    m_renderTexture = m_graphicsBackend->createRenderTexture(width, height);
    if(m_renderTexture == INVALID_HANDLE) {
        LOG_ERROR("Failed to create render texture for post-processing pipeline");
        return false;
    }

    // Initialiser tous les effets déjà ajoutés
    for(auto& effect : m_effects) {
        if(!effect->initialize(m_graphicsBackend, width, height)) {
            LOG_WARN("Failed to initialize post-process effect: {}", effect->getName());
        }
    }

    m_initialized = true;
    LOG_INFO("Post-processing pipeline initialized ({}x{})", width, height);
    return true;
}

void PostProcessPipeline::shutdown() {
    // Nettoyer tous les effets
    for(auto& effect : m_effects) {
        effect->shutdown();
    }
    m_effects.clear();

    // Nettoyer la render texture
    if(m_renderTexture != INVALID_HANDLE) {
        m_graphicsBackend->unloadRenderTexture(m_renderTexture);
        m_renderTexture = INVALID_HANDLE;
    }

    m_initialized = false;
}

void PostProcessPipeline::clearEffects() {
    for(auto& effect : m_effects) {
        effect->shutdown();
    }
    m_effects.clear();
}

void PostProcessPipeline::beginSceneRender() {
    if(!m_enabled || m_renderTexture == INVALID_HANDLE) {
        return;
    }

    // Clear et bind la render texture
    m_graphicsBackend->clearRenderTexture(m_renderTexture, Color::Black);
    m_graphicsBackend->bindRenderTexture(m_renderTexture);
}

void PostProcessPipeline::endSceneRender(f32 deltaTime) {
    if(!m_enabled || m_renderTexture == INVALID_HANDLE) {
        return;
    }

    // Finaliser la render texture
    m_graphicsBackend->displayRenderTexture(m_renderTexture);

    // Unbind pour revenir à la fenêtre
    m_graphicsBackend->unbindRenderTexture();

    // Appliquer chaque effet activé séquentiellement
    for(auto& effect : m_effects) {
        if(effect->isEnabled()) {
            effect->apply(m_renderTexture, deltaTime);
        }
    }

    // Si aucun effet n'est activé, on dessine quand même la scène
    if(m_effects.empty() || !hasAnyEffectEnabled()) {
        m_graphicsBackend->drawRenderTextureToScreen(m_renderTexture, INVALID_HANDLE);
    }
}

bool PostProcessPipeline::hasAnyEffectEnabled() const {
    for(const auto& effect : m_effects) {
        if(effect->isEnabled()) {
            return true;
        }
    }
    return false;
}

}
