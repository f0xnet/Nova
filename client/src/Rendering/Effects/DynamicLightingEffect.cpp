#include "NovaEngine/Rendering/Effects/DynamicLightingEffect.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include <algorithm>

namespace NovaEngine {

DynamicLightingEffect::DynamicLightingEffect()
    : m_lightingShader(INVALID_HANDLE)
    , m_width(0)
    , m_height(0)
    , m_ambientDarkness(0.01f)  // Très sombre par défaut
    , m_timeOfDay(0.5f)         // Midi par défaut (0.5 = 12h00)
    , m_cameraPosition(640.0f, 360.0f)  // Default camera center
    , m_viewportSize(1280.0f, 720.0f)   // Default viewport size
{
}

DynamicLightingEffect::~DynamicLightingEffect() {
    shutdown();
}

bool DynamicLightingEffect::initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) {
    m_graphicsBackend = graphicsBackend;
    m_width = width;
    m_height = height;

    // Charger le shader
    m_lightingShader = m_graphicsBackend->loadShader(
        "data/shaders/crt.vert",  // Vertex shader standard (comme BloomEffect et ColorGradingEffect)
        "data/shaders/dynamic_lighting.frag");

    if (m_lightingShader == INVALID_HANDLE) {
        LOG_ERROR("DynamicLightingEffect: Failed to load dynamic lighting shader");
        return false;
    }

    LOG_INFO("DynamicLightingEffect initialized ({}x{})", width, height);
    return true;
}

void DynamicLightingEffect::shutdown() {
    if (m_lightingShader != INVALID_HANDLE) {
        m_graphicsBackend->unloadShader(m_lightingShader);
        m_lightingShader = INVALID_HANDLE;
    }
}

void DynamicLightingEffect::apply(RenderTextureHandle inputTexture,
                                   RenderTextureHandle outputTexture,
                                   f32 deltaTime)
{
    if (m_lightingShader == INVALID_HANDLE) {
        LOG_WARN("DynamicLightingEffect: Invalid shader handle");
        return;
    }

    // Mettre à jour les uniforms du shader
    updateShaderUniforms();

    // Appliquer le shader
    if (outputTexture == INVALID_HANDLE) {
        // Dernier effet : dessiner à l'écran
        m_graphicsBackend->drawRenderTextureToScreen(inputTexture, m_lightingShader);
    } else {
        // Effet intermédiaire : dessiner dans une texture
        m_graphicsBackend->drawRenderTextureToRenderTexture(inputTexture, outputTexture, m_lightingShader);
    }
}

void DynamicLightingEffect::updateShaderUniforms() {
    // Nombre de lumières actives
    i32 activeLightCount = 0;
    for (const auto& light : m_lights) {
        if (light.enabled) activeLightCount++;
    }

    m_graphicsBackend->setShaderParameter(m_lightingShader, "numLights", activeLightCount);
    m_graphicsBackend->setShaderParameter(m_lightingShader, "ambientDarkness", m_ambientDarkness);
    m_graphicsBackend->setShaderParameter(m_lightingShader, "timeOfDay", m_timeOfDay);

    // Camera parameters for world → screen conversion
    m_graphicsBackend->setShaderParameter(m_lightingShader, "cameraPosition", m_cameraPosition);
    m_graphicsBackend->setShaderParameter(m_lightingShader, "viewportSize", m_viewportSize);

    // Préparer les tableaux pour les uniforms
    std::vector<Vec2f> positions;
    std::vector<Vec3f> colors;
    std::vector<f32> radii;
    std::vector<f32> intensities;
    std::vector<f32> types;           // f32 for shader compatibility (int arrays not supported)
    std::vector<Vec2f> directions;
    std::vector<f32> angles;

    positions.reserve(MAX_LIGHTS);
    colors.reserve(MAX_LIGHTS);
    radii.reserve(MAX_LIGHTS);
    intensities.reserve(MAX_LIGHTS);
    types.reserve(MAX_LIGHTS);
    directions.reserve(MAX_LIGHTS);
    angles.reserve(MAX_LIGHTS);

    // Remplir les tableaux avec les lumières actives
    for (const auto& light : m_lights) {
        if (light.enabled) {
            positions.push_back(light.position);
            colors.push_back(light.color);
            radii.push_back(light.radius);
            intensities.push_back(light.intensity);
            types.push_back(static_cast<f32>(light.type));  // Convert enum to f32
            directions.push_back(light.direction);
            angles.push_back(light.angle);
        }
    }

    // Remplir le reste avec des valeurs par défaut (si moins de MAX_LIGHTS)
    while (positions.size() < MAX_LIGHTS) {
        positions.push_back(Vec2f(0.0f, 0.0f));
        colors.push_back(Vec3f(1.0f, 1.0f, 1.0f));
        radii.push_back(0.0f);
        intensities.push_back(0.0f);
        types.push_back(0.0f); // Point by default
        directions.push_back(Vec2f(0.0f, 1.0f));
        angles.push_back(45.0f);
    }

    // Envoyer les tableaux au shader
    m_graphicsBackend->setShaderParameterArray(m_lightingShader, "lightPositions", positions.data(), MAX_LIGHTS);
    m_graphicsBackend->setShaderParameterArray(m_lightingShader, "lightColors", colors.data(), MAX_LIGHTS);
    m_graphicsBackend->setShaderParameterArray(m_lightingShader, "lightRadius", radii.data(), MAX_LIGHTS);
    m_graphicsBackend->setShaderParameterArray(m_lightingShader, "lightIntensity", intensities.data(), MAX_LIGHTS);
    m_graphicsBackend->setShaderParameterArray(m_lightingShader, "lightTypes", types.data(), MAX_LIGHTS);
    m_graphicsBackend->setShaderParameterArray(m_lightingShader, "lightDirections", directions.data(), MAX_LIGHTS);
    m_graphicsBackend->setShaderParameterArray(m_lightingShader, "lightAngles", angles.data(), MAX_LIGHTS);
}

i32 DynamicLightingEffect::addLight(const LightData& light) {
    if (m_lights.size() >= MAX_LIGHTS) {
        LOG_WARN("Cannot add light: maximum of {} lights reached", MAX_LIGHTS);
        return -1;
    }

    m_lights.push_back(light);
    LOG_INFO("Light added (total: {})", m_lights.size());
    return static_cast<i32>(m_lights.size() - 1);
}

void DynamicLightingEffect::removeLight(i32 index) {
    if (index < 0 || index >= static_cast<i32>(m_lights.size())) {
        LOG_WARN("Cannot remove light: invalid index {}", index);
        return;
    }

    m_lights.erase(m_lights.begin() + index);
    LOG_INFO("Light removed (remaining: {})", m_lights.size());
}

void DynamicLightingEffect::updateLight(i32 index, const LightData& light) {
    if (index < 0 || index >= static_cast<i32>(m_lights.size())) {
        LOG_WARN("Cannot update light: invalid index {}", index);
        return;
    }

    m_lights[index] = light;
}

void DynamicLightingEffect::clearLights() {
    i32 count = static_cast<i32>(m_lights.size());
    m_lights.clear();
    LOG_INFO("All lights cleared (removed {} lights)", count);
}

LightData* DynamicLightingEffect::getLight(i32 index) {
    if (index < 0 || index >= static_cast<i32>(m_lights.size())) {
        return nullptr;
    }
    return &m_lights[index];
}

void DynamicLightingEffect::setCamera(const Vec2f& position, const Vec2f& viewportSize) {
    m_cameraPosition = position;
    m_viewportSize = viewportSize;
}

}  // namespace NovaEngine
