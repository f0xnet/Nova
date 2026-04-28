#pragma once
#include "../Interfaces/IGraphicsBackend.hpp"
#include <memory>
#include <unordered_map>

// Forward declarations SFML — évite de propager <SFML/Graphics.hpp>
// au reste du projet. Seul SFMLGraphicsBackend.cpp inclut les headers complets.
namespace sf {
    class Texture;
    class Font;
    class RenderWindow;
}

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
    void setShaderParameter(ShaderHandle handle, const String& name, i32 value) override;
    void setShaderParameter(ShaderHandle handle, const String& name, const Vec2f& value) override;
    void setShaderParameter(ShaderHandle handle, const String& name, const Vec3f& value) override;
    void setShaderParameterArray(ShaderHandle handle, const String& name, const f32* values, size_t count) override;
    void setShaderParameterArray(ShaderHandle handle, const String& name, const Vec2f* values, size_t count) override;
    void setShaderParameterArray(ShaderHandle handle, const String& name, const Vec3f* values, size_t count) override;
    void unloadShader(ShaderHandle handle) override;

    RenderTextureHandle createRenderTexture(u32 width, u32 height) override;
    void bindRenderTexture(RenderTextureHandle handle) override;
    void unbindRenderTexture() override;
    void clearRenderTexture(RenderTextureHandle handle, const Color& color) override;
    void displayRenderTexture(RenderTextureHandle handle) override;
    void drawRenderTextureToScreen(RenderTextureHandle handle, ShaderHandle shader) override;
    void drawRenderTextureToRenderTexture(RenderTextureHandle source, RenderTextureHandle dest, ShaderHandle shader) override;
    void unloadRenderTexture(RenderTextureHandle handle) override;

    void beginRectBatch() override;
    void endRectBatch() override;
    void beginSpriteBatch() override;
    void endSpriteBatch() override;

    // Méthodes SFML internes — utilisées uniquement par les autres backends SFML
    sf::Texture*      getSFMLTexture(TextureHandle handle) const;
    sf::Font*         getSFMLFont(FontHandle handle) const;
    void              registerFont(FontHandle handle, sf::Font* font);
    sf::RenderWindow* getRenderWindow() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}
