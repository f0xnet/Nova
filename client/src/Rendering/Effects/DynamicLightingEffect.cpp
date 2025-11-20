#include <NovaEngine/Rendering/Effects/DynamicLightingEffect.hpp>
#include <NovaEngine/Backend/Interfaces/IGraphicsBackend.hpp>
#include <NovaEngine/Core/Logger.hpp>
#include <algorithm>

DynamicLightingEffect::DynamicLightingEffect(NovaEngine::IGraphicsBackend* graphicsBackend)
    : m_graphicsBackend(graphicsBackend)
    , m_lightingShader(NovaEngine::INVALID_HANDLE)
    , m_ambientDarkness(0.01f)  // Très sombre par défaut
{
    // Charger le shader
    m_lightingShader = m_graphicsBackend->loadShader("", "assets/shaders/dynamic_lighting.frag");

    if (m_lightingShader == NovaEngine::INVALID_HANDLE) {
        LOG_ERROR("Failed to load dynamic lighting shader");
    } else {
        LOG_INFO("DynamicLightingEffect initialized successfully");
    }
}

void DynamicLightingEffect::apply(NovaEngine::RenderTextureHandle inputTexture,
                                   NovaEngine::RenderTextureHandle outputTexture,
                                   NovaEngine::f32 deltaTime)
{
    if (m_lightingShader == NovaEngine::INVALID_HANDLE) {
        LOG_WARN("DynamicLightingEffect: Invalid shader handle");
        return;
    }

    // Mettre à jour les uniforms du shader
    updateShaderUniforms();

    // Appliquer le shader
    if (outputTexture == NovaEngine::INVALID_HANDLE) {
        // Dernier effet : dessiner à l'écran
        m_graphicsBackend->drawRenderTextureToScreen(inputTexture, m_lightingShader);
    } else {
        // Effet intermédiaire : dessiner dans une texture
        m_graphicsBackend->drawRenderTextureToRenderTexture(inputTexture, outputTexture, m_lightingShader);
    }
}

void DynamicLightingEffect::updateShaderUniforms() {
    // Nombre de lumières actives
    NovaEngine::i32 activeLightCount = 0;
    for (const auto& light : m_lights) {
        if (light.enabled) activeLightCount++;
    }

    m_graphicsBackend->setShaderParameter(m_lightingShader, "numLights", activeLightCount);
    m_graphicsBackend->setShaderParameter(m_lightingShader, "ambientDarkness", m_ambientDarkness);

    // Préparer les tableaux pour les uniforms
    std::vector<NovaEngine::Vec2f> positions;
    std::vector<NovaEngine::Vec3f> colors;
    std::vector<NovaEngine::f32> radii;
    std::vector<NovaEngine::f32> intensities;

    positions.reserve(MAX_LIGHTS);
    colors.reserve(MAX_LIGHTS);
    radii.reserve(MAX_LIGHTS);
    intensities.reserve(MAX_LIGHTS);

    // Remplir les tableaux avec les lumières actives
    for (const auto& light : m_lights) {
        if (light.enabled) {
            positions.push_back(light.position);
            colors.push_back(light.color);
            radii.push_back(light.radius);
            intensities.push_back(light.intensity);
        }
    }

    // Remplir le reste avec des valeurs par défaut (si moins de MAX_LIGHTS)
    while (positions.size() < MAX_LIGHTS) {
        positions.push_back(NovaEngine::Vec2f(0.0f, 0.0f));
        colors.push_back(NovaEngine::Vec3f(1.0f, 1.0f, 1.0f));
        radii.push_back(0.0f);
        intensities.push_back(0.0f);
    }

    // Envoyer les tableaux au shader
    m_graphicsBackend->setShaderParameterArray(m_lightingShader, "lightPositions", positions.data(), MAX_LIGHTS);
    m_graphicsBackend->setShaderParameterArray(m_lightingShader, "lightColors", colors.data(), MAX_LIGHTS);
    m_graphicsBackend->setShaderParameterArray(m_lightingShader, "lightRadius", radii.data(), MAX_LIGHTS);
    m_graphicsBackend->setShaderParameterArray(m_lightingShader, "lightIntensity", intensities.data(), MAX_LIGHTS);
}

NovaEngine::i32 DynamicLightingEffect::addLight(const LightData& light) {
    if (m_lights.size() >= MAX_LIGHTS) {
        LOG_WARN("Cannot add light: maximum of {} lights reached", MAX_LIGHTS);
        return -1;
    }

    m_lights.push_back(light);
    LOG_INFO("Light added (total: {})", m_lights.size());
    return static_cast<NovaEngine::i32>(m_lights.size() - 1);
}

void DynamicLightingEffect::removeLight(NovaEngine::i32 index) {
    if (index < 0 || index >= static_cast<NovaEngine::i32>(m_lights.size())) {
        LOG_WARN("Cannot remove light: invalid index {}", index);
        return;
    }

    m_lights.erase(m_lights.begin() + index);
    LOG_INFO("Light removed (remaining: {})", m_lights.size());
}

void DynamicLightingEffect::updateLight(NovaEngine::i32 index, const LightData& light) {
    if (index < 0 || index >= static_cast<NovaEngine::i32>(m_lights.size())) {
        LOG_WARN("Cannot update light: invalid index {}", index);
        return;
    }

    m_lights[index] = light;
}

void DynamicLightingEffect::clearLights() {
    NovaEngine::i32 count = static_cast<NovaEngine::i32>(m_lights.size());
    m_lights.clear();
    LOG_INFO("All lights cleared (removed {} lights)", count);
}

LightData* DynamicLightingEffect::getLight(NovaEngine::i32 index) {
    if (index < 0 || index >= static_cast<NovaEngine::i32>(m_lights.size())) {
        return nullptr;
    }
    return &m_lights[index];
}
