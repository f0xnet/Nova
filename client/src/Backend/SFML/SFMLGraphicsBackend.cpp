#include "NovaEngine/Backend/SFML/SFMLGraphicsBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLWindowBackend.hpp"

namespace NovaEngine {
SFMLGraphicsBackend::SFMLGraphicsBackend() : m_window(nullptr) {}
SFMLGraphicsBackend::~SFMLGraphicsBackend() { shutdown(); }

bool SFMLGraphicsBackend::initialize(void* windowHandle) {
    auto* windowBackend = static_cast<SFMLWindowBackend*>(windowHandle);
    if(!windowBackend) return false;
    m_window = windowBackend->getSFMLWindow();
    return m_window != nullptr;
}

void SFMLGraphicsBackend::shutdown() {
    m_textures.clear();
    m_shaders.clear();
    m_fonts.clear();
}

TextureHandle SFMLGraphicsBackend::loadTexture(const String& path) {
    if(path.empty()) return INVALID_HANDLE;
    auto tex = std::make_unique<sf::Texture>();
    if(!tex->loadFromFile(path)) return INVALID_HANDLE;
    TextureHandle handle = m_nextTextureHandle++;
    m_textures[handle] = std::move(tex);
    return handle;
}

TextureHandle SFMLGraphicsBackend::createTexture(u32 width, u32 height) {
    if(width == 0 || height == 0) return INVALID_HANDLE;
    auto tex = std::make_unique<sf::Texture>();
    if(!tex->create(width, height)) return INVALID_HANDLE;
    TextureHandle handle = m_nextTextureHandle++;
    m_textures[handle] = std::move(tex);
    return handle;
}

void SFMLGraphicsBackend::updateTexture(TextureHandle handle, const u8* pixels, u32 width, u32 height, u32 x, u32 y) {
    if(!pixels || width == 0 || height == 0 || handle == INVALID_HANDLE) return;
    auto it = m_textures.find(handle);
    if(it != m_textures.end() && it->second) {
        it->second->update(pixels, width, height, x, y);
    }
}

Vec2u SFMLGraphicsBackend::getTextureSize(TextureHandle handle) const {
    if(handle == INVALID_HANDLE) return Vec2u(0, 0);
    auto it = m_textures.find(handle);
    if(it != m_textures.end() && it->second) {
        return SFMLConv::fromSFML(it->second->getSize());
    }
    return Vec2u(0, 0);
}

void SFMLGraphicsBackend::setTextureSmooth(TextureHandle handle, bool smooth) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_textures.find(handle);
    if(it != m_textures.end() && it->second) {
        it->second->setSmooth(smooth);
    }
}

void SFMLGraphicsBackend::setTextureRepeated(TextureHandle handle, bool repeated) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_textures.find(handle);
    if(it != m_textures.end() && it->second) {
        it->second->setRepeated(repeated);
    }
}

void SFMLGraphicsBackend::unloadTexture(TextureHandle handle) { 
    if(handle == INVALID_HANDLE) return;
    m_textures.erase(handle); 
}

void SFMLGraphicsBackend::drawSprite(const SpriteData& sprite) {
    if(sprite.texture == INVALID_HANDLE || !m_window) return;
    auto it = m_textures.find(sprite.texture);
    if(it == m_textures.end() || !it->second) return;
    
    sf::Sprite sfSprite(*it->second);
    sfSprite.setPosition(SFMLConv::toSFML(sprite.position));
    sfSprite.setRotation(sprite.rotation);
    
    // ✅ AJOUT: Appliquer sprite.size si spécifié
    if(sprite.size.x > 0 && sprite.size.y > 0) {
        sf::Vector2u texSize = it->second->getSize();
        if(texSize.x > 0 && texSize.y > 0) {
            float scaleX = sprite.size.x / texSize.x;
            float scaleY = sprite.size.y / texSize.y;
            sfSprite.setScale(scaleX * sprite.scale.x, scaleY * sprite.scale.y);
        }
    } else {
        sfSprite.setScale(SFMLConv::toSFML(sprite.scale));
    }
    
    sfSprite.setOrigin(SFMLConv::toSFML(sprite.origin));
    sfSprite.setColor(SFMLConv::toSFML(sprite.color));
    
    if(sprite.textureRect.width > 0 && sprite.textureRect.height > 0) {
        sfSprite.setTextureRect(SFMLConv::toSFML(sprite.textureRect));
    }
    
    sf::RenderStates states;
    states.blendMode = SFMLConv::toSFML(sprite.blendMode);
    if(m_boundShader != INVALID_HANDLE) {
        auto sit = m_shaders.find(m_boundShader);
        if(sit != m_shaders.end() && sit->second) {
            states.shader = sit->second.get();
        }
    }
    m_window->draw(sfSprite, states);
}

void SFMLGraphicsBackend::drawRect(const RectData& rect) {
    if(!m_window) return;
    
    sf::RectangleShape shape(SFMLConv::toSFML(rect.size));
    shape.setPosition(SFMLConv::toSFML(rect.position));
    shape.setRotation(rect.rotation);
    shape.setOrigin(SFMLConv::toSFML(rect.origin));
    shape.setFillColor(SFMLConv::toSFML(rect.fillColor));
    shape.setOutlineColor(SFMLConv::toSFML(rect.outlineColor));
    shape.setOutlineThickness(rect.outlineThickness);
    
    sf::RenderStates states;
    if(m_boundShader != INVALID_HANDLE) {
        auto sit = m_shaders.find(m_boundShader);
        if(sit != m_shaders.end() && sit->second) {
            states.shader = sit->second.get();
        }
    }
    m_window->draw(shape, states);
}

void SFMLGraphicsBackend::drawText(const TextData& text) {
    if(text.font == INVALID_HANDLE || !m_window) return;
    auto fit = m_fonts.find(text.font);
    if(fit == m_fonts.end() || !fit->second) return;
    
    sf::Text sfText;
    sfText.setFont(*fit->second);
    sfText.setString(text.text);
    sfText.setCharacterSize(text.characterSize);
    sfText.setFillColor(SFMLConv::toSFML(text.fillColor));
    sfText.setOutlineColor(SFMLConv::toSFML(text.outlineColor));
    sfText.setOutlineThickness(text.outlineThickness);
    sfText.setStyle(SFMLConv::toSFMLStyle(text.style));
    sfText.setPosition(SFMLConv::toSFML(text.position));
    sfText.setRotation(text.rotation);
    sfText.setScale(SFMLConv::toSFML(text.scale));
    sfText.setOrigin(SFMLConv::toSFML(text.origin));
    
    sf::RenderStates states;
    states.blendMode = SFMLConv::toSFML(text.blendMode);
    if(m_boundShader != INVALID_HANDLE) {
        auto sit = m_shaders.find(m_boundShader);
        if(sit != m_shaders.end() && sit->second) {
            states.shader = sit->second.get();
        }
    }
    m_window->draw(sfText, states);
}

ShaderHandle SFMLGraphicsBackend::loadShader(const String& vertexPath, const String& fragmentPath) {
    if(vertexPath.empty() || fragmentPath.empty()) return INVALID_HANDLE;
    auto shader = std::make_unique<sf::Shader>();
    if(!shader->loadFromFile(vertexPath, fragmentPath)) return INVALID_HANDLE;
    ShaderHandle handle = m_nextShaderHandle++;
    m_shaders[handle] = std::move(shader);
    return handle;
}

void SFMLGraphicsBackend::bindShader(ShaderHandle handle) { 
    m_boundShader = handle; 
}

void SFMLGraphicsBackend::unloadShader(ShaderHandle handle) { 
    if(handle == INVALID_HANDLE) return;
    if(m_boundShader == handle) m_boundShader = INVALID_HANDLE;
    m_shaders.erase(handle); 
}

sf::Texture* SFMLGraphicsBackend::getSFMLTexture(TextureHandle handle) const {
    if(handle == INVALID_HANDLE) return nullptr;
    auto it = m_textures.find(handle);
    return (it != m_textures.end()) ? it->second.get() : nullptr;
}

sf::Font* SFMLGraphicsBackend::getSFMLFont(FontHandle handle) const {
    if(handle == INVALID_HANDLE) return nullptr;
    auto it = m_fonts.find(handle);
    return (it != m_fonts.end()) ? it->second : nullptr;
}

void SFMLGraphicsBackend::registerFont(FontHandle handle, sf::Font* font) {
    if(handle == INVALID_HANDLE || !font) return;
    m_fonts[handle] = font;
}
}
