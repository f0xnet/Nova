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
    virtual void unloadShader(ShaderHandle handle) = 0;
};
}
