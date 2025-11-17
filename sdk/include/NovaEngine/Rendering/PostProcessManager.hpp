#pragma once
#include "../Backend/Core/BackendTypes.hpp"
#include "../Backend/Interfaces/IGraphicsBackend.hpp"
#include <chrono>

namespace NovaEngine {
class PostProcessManager {
public:
    PostProcessManager(IGraphicsBackend* graphicsBackend);
    ~PostProcessManager();

    bool initialize(u32 width, u32 height);
    void shutdown();

    void beginFrame();
    void endFrame();

    void setCRTEnabled(bool enabled) { m_crtEnabled = enabled; }
    bool isCRTEnabled() const { return m_crtEnabled; }

    // Paramètres shader CRT
    void setScanlineIntensity(f32 intensity) { m_scanlineIntensity = intensity; }
    void setPixelGridIntensity(f32 intensity) { m_pixelGridIntensity = intensity; }
    void setChromaticAberration(f32 amount) { m_chromaticAberration = amount; }
    void setRGBShiftAmount(f32 amount) { m_rgbShiftAmount = amount; }
    void setCurvature(f32 curvature) { m_curvature = curvature; }
    void setVignetteStrength(f32 strength) { m_vignetteStrength = strength; }
    void setGlowIntensity(f32 intensity) { m_glowIntensity = intensity; }
    void setNoiseIntensity(f32 intensity) { m_noiseIntensity = intensity; }
    void setColorBanding(f32 banding) { m_colorBanding = banding; }

private:
    void updateShaderParameters();

    IGraphicsBackend* m_graphicsBackend;
    RenderTextureHandle m_renderTexture;
    ShaderHandle m_crtShader;
    bool m_crtEnabled;
    u32 m_width;
    u32 m_height;

    // Paramètres CRT
    f32 m_scanlineIntensity;
    f32 m_pixelGridIntensity;
    f32 m_chromaticAberration;
    f32 m_rgbShiftAmount;
    f32 m_curvature;
    f32 m_vignetteStrength;
    f32 m_glowIntensity;
    f32 m_noiseIntensity;
    f32 m_colorBanding;

    std::chrono::high_resolution_clock::time_point m_startTime;
};
}
