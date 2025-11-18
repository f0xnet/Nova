#include "NovaEngine/Backend/SFML/SFMLGraphicsBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLWindowBackend.hpp"

namespace NovaEngine {
SFMLGraphicsBackend::SFMLGraphicsBackend() : m_window(nullptr), m_activeRenderTarget(nullptr) {}
SFMLGraphicsBackend::~SFMLGraphicsBackend() { shutdown(); }

bool SFMLGraphicsBackend::initialize(void* windowHandle) {
    auto* windowBackend = static_cast<SFMLWindowBackend*>(windowHandle);
    if(!windowBackend) return false;
    m_window = windowBackend->getSFMLWindow();
    m_activeRenderTarget = m_window;
    return m_window != nullptr;
}

void SFMLGraphicsBackend::shutdown() {
    m_textures.clear();
    m_shaders.clear();
    m_renderTextures.clear();
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
    if(sprite.texture == INVALID_HANDLE || !m_activeRenderTarget) return;
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
    m_activeRenderTarget->draw(sfSprite, states);
}

void SFMLGraphicsBackend::drawRect(const RectData& rect) {
    if(!m_activeRenderTarget) return;

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
    m_activeRenderTarget->draw(shape, states);
}

void SFMLGraphicsBackend::drawText(const TextData& text) {
    if(text.font == INVALID_HANDLE || !m_activeRenderTarget) return;
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
    m_activeRenderTarget->draw(sfText, states);
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

void SFMLGraphicsBackend::unbindShader() {
    m_boundShader = INVALID_HANDLE;
}

void SFMLGraphicsBackend::setShaderParameter(ShaderHandle handle, const String& name, f32 value) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_shaders.find(handle);
    if(it != m_shaders.end() && it->second) {
        it->second->setUniform(name, value);
    }
}

void SFMLGraphicsBackend::setShaderParameter(ShaderHandle handle, const String& name, const Vec2f& value) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_shaders.find(handle);
    if(it != m_shaders.end() && it->second) {
        it->second->setUniform(name, sf::Vector2f(value.x, value.y));
    }
}

void SFMLGraphicsBackend::unloadShader(ShaderHandle handle) {
    if(handle == INVALID_HANDLE) return;
    if(m_boundShader == handle) m_boundShader = INVALID_HANDLE;
    m_shaders.erase(handle);
}

RenderTextureHandle SFMLGraphicsBackend::createRenderTexture(u32 width, u32 height) {
    if(width == 0 || height == 0) return INVALID_HANDLE;
    auto renderTexture = std::make_unique<sf::RenderTexture>();
    if(!renderTexture->create(width, height)) return INVALID_HANDLE;
    RenderTextureHandle handle = m_nextRenderTextureHandle++;
    m_renderTextures[handle] = std::move(renderTexture);
    return handle;
}

void SFMLGraphicsBackend::bindRenderTexture(RenderTextureHandle handle) {
    if(handle == INVALID_HANDLE || !m_window) return;
    auto it = m_renderTextures.find(handle);
    if(it != m_renderTextures.end() && it->second) {
        // Copy the window's view (size, center, rotation) to the render texture
        // But use full viewport since render texture is sized to logical resolution
        sf::View view = m_window->getView();
        view.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
        it->second->setView(view);
        m_activeRenderTarget = it->second.get();
    }
}

void SFMLGraphicsBackend::unbindRenderTexture() {
    m_activeRenderTarget = m_window;
}

void SFMLGraphicsBackend::clearRenderTexture(RenderTextureHandle handle, const Color& color) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_renderTextures.find(handle);
    if(it != m_renderTextures.end() && it->second) {
        it->second->clear(SFMLConv::toSFML(color));
    }
}

void SFMLGraphicsBackend::displayRenderTexture(RenderTextureHandle handle) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_renderTextures.find(handle);
    if(it != m_renderTextures.end() && it->second) {
        it->second->display();
    }
}

void SFMLGraphicsBackend::drawRenderTextureToScreen(RenderTextureHandle handle, ShaderHandle shader) {
    if(handle == INVALID_HANDLE || !m_window) return;

    auto rtIt = m_renderTextures.find(handle);
    if(rtIt == m_renderTextures.end() || !rtIt->second) return;

    // Sauvegarder la vue actuelle
    sf::View previousView = m_window->getView();

    // Utiliser la vue par défaut pour dessiner le sprite plein écran
    m_window->setView(m_window->getDefaultView());

    // Créer un sprite avec la texture de la render texture
    const sf::Texture& texture = rtIt->second->getTexture();
    sf::Sprite sprite(texture);

    // Calculer l'échelle pour remplir la fenêtre
    sf::Vector2u texSize = texture.getSize();
    sf::Vector2u windowSize = m_window->getSize();
    float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(texSize.x);
    float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(texSize.y);

    // IMPORTANT: Les render textures SFML sont inversées verticalement
    // On doit appliquer une échelle négative sur Y pour les afficher correctement
    sprite.setOrigin(0, static_cast<float>(texSize.y));
    sprite.setScale(scaleX, -scaleY);
    sprite.setPosition(0, static_cast<float>(windowSize.y));

    // Préparer les render states avec le shader si fourni
    sf::RenderStates states;
    if(shader != INVALID_HANDLE) {
        auto shaderIt = m_shaders.find(shader);
        if(shaderIt != m_shaders.end() && shaderIt->second) {
            states.shader = shaderIt->second.get();
        }
    }

    // Dessiner sur la fenêtre
    m_window->draw(sprite, states);

    // Restaurer la vue précédente
    m_window->setView(previousView);
}

void SFMLGraphicsBackend::unloadRenderTexture(RenderTextureHandle handle) {
    if(handle == INVALID_HANDLE) return;
    m_renderTextures.erase(handle);
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
