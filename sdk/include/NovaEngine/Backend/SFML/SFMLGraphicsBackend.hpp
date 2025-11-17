#pragma once
#include "../Interfaces/IGraphicsBackend.hpp"
#include "SFMLConversions.hpp"
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>

namespace NovaEngine {
class SFMLGraphicsBackend : public IGraphicsBackend {
public:
    SFMLGraphicsBackend();
    ~SFMLGraphicsBackend() override;
    
    bool initialize(void* windowHandle) override;
    void shutdown() override;
    
    TextureHandle loadTexture(const String& path) override;
    TextureHandle createTexture(u32 width, u32 height) override;
    void updateTexture(TextureHandle handle, const u8* pixels, u32 width, u32 height, u32 x, u32 y) override;
    Vec2u getTextureSize(TextureHandle handle) const override;
    void setTextureSmooth(TextureHandle handle, bool smooth) override;
    void setTextureRepeated(TextureHandle handle, bool repeated) override;
    void unloadTexture(TextureHandle handle) override;
    
    void drawSprite(const SpriteData& sprite) override;
    void drawRect(const RectData& rect) override;
    void drawText(const TextData& text) override;
    
    ShaderHandle loadShader(const String& vertexPath, const String& fragmentPath) override;
    void bindShader(ShaderHandle handle) override;
    void unbindShader() override;
    void setShaderParameter(ShaderHandle handle, const String& name, f32 value) override;
    void setShaderParameter(ShaderHandle handle, const String& name, const Vec2f& value) override;
    void unloadShader(ShaderHandle handle) override;

    RenderTextureHandle createRenderTexture(u32 width, u32 height) override;
    void bindRenderTexture(RenderTextureHandle handle) override;
    void unbindRenderTexture() override;
    void clearRenderTexture(RenderTextureHandle handle, const Color& color) override;
    void displayRenderTexture(RenderTextureHandle handle) override;
    void drawRenderTextureToScreen(RenderTextureHandle handle, ShaderHandle shader) override;
    void unloadRenderTexture(RenderTextureHandle handle) override;

    sf::Texture* getSFMLTexture(TextureHandle handle) const;
    sf::Font* getSFMLFont(FontHandle handle) const;
    void registerFont(FontHandle handle, sf::Font* font);
    sf::RenderWindow* getRenderWindow() const { return m_window; }

private:
    sf::RenderWindow* m_window;
    sf::RenderTarget* m_activeRenderTarget;
    std::unordered_map<TextureHandle, std::unique_ptr<sf::Texture>> m_textures;
    std::unordered_map<ShaderHandle, std::unique_ptr<sf::Shader>> m_shaders;
    std::unordered_map<RenderTextureHandle, std::unique_ptr<sf::RenderTexture>> m_renderTextures;
    std::unordered_map<FontHandle, sf::Font*> m_fonts;
    u64 m_nextTextureHandle = 1;
    u64 m_nextShaderHandle = 1;
    u64 m_nextRenderTextureHandle = 1;
    ShaderHandle m_boundShader = INVALID_HANDLE;
};
}
