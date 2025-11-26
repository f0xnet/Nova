#include "NovaEngine/Rendering/Effects/ColorGradingEffect.hpp"
#include "NovaEngine/Core/Logger.hpp"

namespace NovaEngine {

ColorGradingEffect::ColorGradingEffect()
    : m_shader(INVALID_HANDLE)
    , m_width(0)
    , m_height(0)
    , m_saturation(1.3f)
    , m_contrast(1.0f)
    , m_brightness(0.0f)
{
}

ColorGradingEffect::~ColorGradingEffect() {
    shutdown();
}

bool ColorGradingEffect::initialize(IGraphicsBackend* graphicsBackend, u32 width, u32 height) {
    m_graphicsBackend = graphicsBackend;
    m_width = width;
    m_height = height;

    // Load color grading shader
    m_shader = m_graphicsBackend->loadShader(
        "data/shaders/crt.vert",  // Utilise le vertex shader standard
        "data/shaders/color_grading.frag"
    );

    if(m_shader == INVALID_HANDLE) {
        LOG_ERROR("ColorGradingEffect: Failed to load shader");
        return false;
    }

    updateParameters();

    LOG_INFO("ColorGradingEffect initialized");
    return true;
}

void ColorGradingEffect::shutdown() {
    if(m_shader != INVALID_HANDLE) {
        m_graphicsBackend->unloadShader(m_shader);
        m_shader = INVALID_HANDLE;
    }
}

void ColorGradingEffect::apply(RenderTextureHandle inputTexture, RenderTextureHandle outputTexture, f32 deltaTime) {
    if(m_shader == INVALID_HANDLE) {
        // Fallback: draw without color grading
        if(outputTexture == INVALID_HANDLE) {
            m_graphicsBackend->drawRenderTextureToScreen(inputTexture, INVALID_HANDLE);
        } else {
            m_graphicsBackend->drawRenderTextureToRenderTexture(inputTexture, outputTexture, INVALID_HANDLE);
        }
        return;
    }

    // Set shader parameters (must be set before each draw)
    m_graphicsBackend->setShaderParameter(m_shader, "saturation", m_saturation);
    m_graphicsBackend->setShaderParameter(m_shader, "contrast", m_contrast);
    m_graphicsBackend->setShaderParameter(m_shader, "brightness", m_brightness);

    // Draw with color grading
    if(outputTexture == INVALID_HANDLE) {
        m_graphicsBackend->drawRenderTextureToScreen(inputTexture, m_shader);
    } else {
        m_graphicsBackend->drawRenderTextureToRenderTexture(inputTexture, outputTexture, m_shader);
    }
}

void ColorGradingEffect::updateParameters() {
    if(m_shader == INVALID_HANDLE || !m_graphicsBackend) {
        return;
    }

    m_graphicsBackend->setShaderParameter(m_shader, "saturation", m_saturation);
    m_graphicsBackend->setShaderParameter(m_shader, "contrast", m_contrast);
    m_graphicsBackend->setShaderParameter(m_shader, "brightness", m_brightness);
}

}
