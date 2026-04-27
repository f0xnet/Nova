#pragma once
#include "NovaEngine/Backend/Interfaces/IGraphicsBackend.hpp"

// Minimal mock backend for unit tests — only RenderTexture methods track calls.
namespace NovaTest {

class MockGraphicsBackend : public NovaEngine::IGraphicsBackend {
public:
    int createRTCount  = 0;
    int unloadRTCount  = 0;
    NovaEngine::u64 nextHandle = 1;

    // -- RenderTexture --
    NovaEngine::RenderTextureHandle createRenderTexture(NovaEngine::u32, NovaEngine::u32) override {
        ++createRTCount;
        return nextHandle++;
    }
    void unloadRenderTexture(NovaEngine::RenderTextureHandle) override { ++unloadRTCount; }
    void bindRenderTexture(NovaEngine::RenderTextureHandle) override {}
    void unbindRenderTexture() override {}
    void clearRenderTexture(NovaEngine::RenderTextureHandle, const NovaEngine::Color&) override {}
    void displayRenderTexture(NovaEngine::RenderTextureHandle) override {}
    void drawRenderTextureToScreen(NovaEngine::RenderTextureHandle, NovaEngine::ShaderHandle) override {}
    void drawRenderTextureToRenderTexture(NovaEngine::RenderTextureHandle, NovaEngine::RenderTextureHandle, NovaEngine::ShaderHandle) override {}

    // -- Stubs --
    bool initialize(void*) override { return true; }
    void shutdown() override {}
    NovaEngine::TextureHandle loadTexture(const NovaEngine::String&) override { return nextHandle++; }
    NovaEngine::TextureHandle createTexture(NovaEngine::u32, NovaEngine::u32) override { return nextHandle++; }
    void updateTexture(NovaEngine::TextureHandle, const NovaEngine::u8*, NovaEngine::u32, NovaEngine::u32, NovaEngine::u32, NovaEngine::u32) override {}
    NovaEngine::Vec2u getTextureSize(NovaEngine::TextureHandle) const override { return {0, 0}; }
    void setTextureSmooth(NovaEngine::TextureHandle, bool) override {}
    void setTextureRepeated(NovaEngine::TextureHandle, bool) override {}
    void unloadTexture(NovaEngine::TextureHandle) override {}
    void drawSprite(const NovaEngine::SpriteData&) override {}
    void drawRect(const NovaEngine::RectData&) override {}
    void drawText(const NovaEngine::TextData&) override {}
    NovaEngine::ShaderHandle loadShader(const NovaEngine::String&, const NovaEngine::String&) override { return nextHandle++; }
    void bindShader(NovaEngine::ShaderHandle) override {}
    void unbindShader() override {}
    void setShaderParameter(NovaEngine::ShaderHandle, const NovaEngine::String&, NovaEngine::f32) override {}
    void setShaderParameter(NovaEngine::ShaderHandle, const NovaEngine::String&, NovaEngine::i32) override {}
    void setShaderParameter(NovaEngine::ShaderHandle, const NovaEngine::String&, const NovaEngine::Vec2f&) override {}
    void setShaderParameter(NovaEngine::ShaderHandle, const NovaEngine::String&, const NovaEngine::Vec3f&) override {}
    void setShaderParameterArray(NovaEngine::ShaderHandle, const NovaEngine::String&, const NovaEngine::f32*, size_t) override {}
    void setShaderParameterArray(NovaEngine::ShaderHandle, const NovaEngine::String&, const NovaEngine::Vec2f*, size_t) override {}
    void setShaderParameterArray(NovaEngine::ShaderHandle, const NovaEngine::String&, const NovaEngine::Vec3f*, size_t) override {}
    void unloadShader(NovaEngine::ShaderHandle) override {}
};

} // namespace NovaTest
