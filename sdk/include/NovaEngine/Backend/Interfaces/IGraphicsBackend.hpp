#pragma once
#include "../Core/BackendTypes.hpp"

namespace NovaEngine {
class IGraphicsBackend {
public:
    virtual ~IGraphicsBackend() = default;
    
    virtual bool initialize(void* windowHandle) = 0;
    virtual void shutdown() = 0;
    
    virtual TextureHandle loadTexture(const String& path) = 0;
    virtual TextureHandle createTexture(u32 width, u32 height) = 0;
    virtual void updateTexture(TextureHandle handle, const u8* pixels, u32 width, u32 height, u32 x = 0, u32 y = 0) = 0;
    virtual Vec2u getTextureSize(TextureHandle handle) const = 0;
    virtual void setTextureSmooth(TextureHandle handle, bool smooth) = 0;
    virtual void setTextureRepeated(TextureHandle handle, bool repeated) = 0;
    virtual void unloadTexture(TextureHandle handle) = 0;
    
    virtual void drawSprite(const SpriteData& sprite) = 0;
    virtual void drawRect(const RectData& rect) = 0;
    virtual void drawText(const TextData& text) = 0;
    
    virtual ShaderHandle loadShader(const String& vertexPath, const String& fragmentPath) = 0;
    virtual void bindShader(ShaderHandle handle) = 0;
    virtual void unbindShader() = 0;
    virtual void setShaderParameter(ShaderHandle handle, const String& name, f32 value) = 0;
    virtual void setShaderParameter(ShaderHandle handle, const String& name, i32 value) = 0;
    virtual void setShaderParameter(ShaderHandle handle, const String& name, const Vec2f& value) = 0;
    virtual void setShaderParameter(ShaderHandle handle, const String& name, const Vec3f& value) = 0;
    virtual void setShaderParameterArray(ShaderHandle handle, const String& name, const f32* values, size_t count) = 0;
    virtual void setShaderParameterArray(ShaderHandle handle, const String& name, const Vec2f* values, size_t count) = 0;
    virtual void setShaderParameterArray(ShaderHandle handle, const String& name, const Vec3f* values, size_t count) = 0;
    virtual void unloadShader(ShaderHandle handle) = 0;

    virtual RenderTextureHandle createRenderTexture(u32 width, u32 height) = 0;
    virtual void bindRenderTexture(RenderTextureHandle handle) = 0;
    virtual void unbindRenderTexture() = 0;
    virtual void clearRenderTexture(RenderTextureHandle handle, const Color& color) = 0;
    virtual void displayRenderTexture(RenderTextureHandle handle) = 0;
    virtual void drawRenderTextureToScreen(RenderTextureHandle handle, ShaderHandle shader) = 0;
    virtual void drawRenderTextureToRenderTexture(RenderTextureHandle source, RenderTextureHandle dest, ShaderHandle shader) = 0;
    virtual void unloadRenderTexture(RenderTextureHandle handle) = 0;

    // Rect batching — drawRect() calls between begin/end are accumulated and
    // flushed as a single draw call in endRectBatch(). Rects that cannot be
    // batched (outline, rotation) are drawn immediately as usual.
    // Default implementation is a no-op so alternative backends need not override.
    virtual void beginRectBatch() {}
    virtual void endRectBatch() {}

    // Sprite batching — drawSprite() calls between begin/end are grouped by
    // texture and flushed with one draw call per texture group. Sprites with
    // rotation, a per-sprite shader, or a non-Alpha blend mode are drawn
    // immediately. Cross-type ordering (rect vs sprite) is preserved by
    // automatic flushing when the draw type changes.
    virtual void beginSpriteBatch() {}
    virtual void endSpriteBatch() {}
};
}
