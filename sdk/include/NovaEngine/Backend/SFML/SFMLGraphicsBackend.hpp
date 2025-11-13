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
    void unloadShader(ShaderHandle handle) override;
    
    sf::Texture* getSFMLTexture(TextureHandle handle) const;
    sf::Font* getSFMLFont(FontHandle handle) const;
    void registerFont(FontHandle handle, sf::Font* font);
    sf::RenderWindow* getRenderWindow() const { return m_window; }
    
private:
    sf::RenderWindow* m_window;
    std::unordered_map<TextureHandle, std::unique_ptr<sf::Texture>> m_textures;
    std::unordered_map<ShaderHandle, std::unique_ptr<sf::Shader>> m_shaders;
    std::unordered_map<FontHandle, sf::Font*> m_fonts;
    u64 m_nextTextureHandle = 1;
    u64 m_nextShaderHandle = 1;
    ShaderHandle m_boundShader = INVALID_HANDLE;
};
}
