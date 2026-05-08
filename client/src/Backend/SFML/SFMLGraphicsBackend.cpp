#include "NovaEngine/Backend/SFML/SFMLGraphicsBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLConversions.hpp"
#include "NovaEngine/Backend/SFML/SFMLWindowBackend.hpp"
#include "NovaEngine/Core/Logger.hpp"

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>
#include <vector>

namespace NovaEngine {

struct SFMLGraphicsBackend::Impl {
    sf::RenderWindow*  window            = nullptr;
    sf::RenderTarget*  activeRenderTarget = nullptr;

    std::unordered_map<TextureHandle,       std::unique_ptr<sf::Texture>>       textures;
    std::unordered_map<ShaderHandle,        std::unique_ptr<sf::Shader>>        shaders;
    std::unordered_map<RenderTextureHandle, std::unique_ptr<sf::RenderTexture>> renderTextures;
    std::unordered_map<FontHandle,          sf::Font*>                          fonts;

    u64 nextTextureHandle       = 1;
    u64 nextShaderHandle        = 1;
    u64 nextRenderTextureHandle = 1;

    ShaderHandle    boundShader        = INVALID_HANDLE;
    sf::VertexArray rectBatch;
    bool            rectBatchActive    = false;
    sf::VertexArray spriteBatch;
    TextureHandle   spriteBatchTexture = INVALID_HANDLE;
    bool            spriteBatchActive  = false;
};

SFMLGraphicsBackend::SFMLGraphicsBackend() : m_impl(std::make_unique<Impl>()) {}
SFMLGraphicsBackend::~SFMLGraphicsBackend() { shutdown(); }

bool SFMLGraphicsBackend::initialize(void* windowHandle) {
    auto* windowBackend = static_cast<SFMLWindowBackend*>(windowHandle);
    if(!windowBackend) return false;
    m_impl->window = windowBackend->getSFMLWindow();
    m_impl->activeRenderTarget = m_impl->window;
    return m_impl->window != nullptr;
}

void SFMLGraphicsBackend::shutdown() {
    m_impl->textures.clear();
    m_impl->shaders.clear();
    m_impl->renderTextures.clear();
    m_impl->fonts.clear();
}

TextureHandle SFMLGraphicsBackend::loadTexture(const String& path) {
    if(path.empty()) return INVALID_HANDLE;
    auto tex = std::make_unique<sf::Texture>();
    if(!tex->loadFromFile(path)) return INVALID_HANDLE;
    TextureHandle handle = m_impl->nextTextureHandle++;
    m_impl->textures[handle] = std::move(tex);
    return handle;
}

TextureHandle SFMLGraphicsBackend::createTexture(u32 width, u32 height) {
    if(width == 0 || height == 0) return INVALID_HANDLE;
    auto tex = std::make_unique<sf::Texture>();
    if(!tex->create(width, height)) return INVALID_HANDLE;
    TextureHandle handle = m_impl->nextTextureHandle++;
    m_impl->textures[handle] = std::move(tex);
    return handle;
}

void SFMLGraphicsBackend::updateTexture(TextureHandle handle, const u8* pixels, u32 width, u32 height, u32 x, u32 y) {
    if(!pixels || width == 0 || height == 0 || handle == INVALID_HANDLE) return;
    auto it = m_impl->textures.find(handle);
    if(it != m_impl->textures.end() && it->second)
        it->second->update(pixels, width, height, x, y);
}

Vec2u SFMLGraphicsBackend::getTextureSize(TextureHandle handle) const {
    if(handle == INVALID_HANDLE) return Vec2u(0, 0);
    auto it = m_impl->textures.find(handle);
    if(it != m_impl->textures.end() && it->second)
        return SFMLConv::fromSFML(it->second->getSize());
    return Vec2u(0, 0);
}

void SFMLGraphicsBackend::setTextureSmooth(TextureHandle handle, bool smooth) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_impl->textures.find(handle);
    if(it != m_impl->textures.end() && it->second)
        it->second->setSmooth(smooth);
}

void SFMLGraphicsBackend::setTextureRepeated(TextureHandle handle, bool repeated) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_impl->textures.find(handle);
    if(it != m_impl->textures.end() && it->second)
        it->second->setRepeated(repeated);
}

void SFMLGraphicsBackend::unloadTexture(TextureHandle handle) {
    if(handle == INVALID_HANDLE) return;
    m_impl->textures.erase(handle);
}

void SFMLGraphicsBackend::drawSprite(const SpriteData& sprite) {
    if(sprite.texture == INVALID_HANDLE || !m_impl->activeRenderTarget) return;
    auto it = m_impl->textures.find(sprite.texture);
    if(it == m_impl->textures.end() || !it->second) return;

    bool batchable = m_impl->spriteBatchActive
        && sprite.rotation == 0.0f
        && sprite.shader == INVALID_HANDLE
        && m_impl->boundShader == INVALID_HANDLE
        && sprite.blendMode == BlendMode::Alpha;

    if (batchable) {
        if (m_impl->rectBatchActive && m_impl->rectBatch.getVertexCount() > 0) {
            m_impl->activeRenderTarget->draw(m_impl->rectBatch);
            m_impl->rectBatch = sf::VertexArray(sf::Quads, 0);
        }
        if (m_impl->spriteBatchTexture != INVALID_HANDLE && m_impl->spriteBatchTexture != sprite.texture) {
            auto fit = m_impl->textures.find(m_impl->spriteBatchTexture);
            if (fit != m_impl->textures.end() && fit->second) {
                sf::RenderStates st; st.texture = fit->second.get();
                m_impl->activeRenderTarget->draw(m_impl->spriteBatch, st);
            }
            m_impl->spriteBatch = sf::VertexArray(sf::Quads, 0);
        }
        m_impl->spriteBatchTexture = sprite.texture;

        sf::Vector2u texSize = it->second->getSize();
        float tx, ty, tw, th;
        if (sprite.textureRect.width > 0 && sprite.textureRect.height > 0) {
            tx = static_cast<float>(sprite.textureRect.left);
            ty = static_cast<float>(sprite.textureRect.top);
            tw = static_cast<float>(sprite.textureRect.width);
            th = static_cast<float>(sprite.textureRect.height);
        } else {
            tx = 0.0f; ty = 0.0f;
            tw = static_cast<float>(texSize.x);
            th = static_cast<float>(texSize.y);
        }
        float w = (sprite.size.x > 0) ? sprite.size.x * sprite.scale.x : tw * sprite.scale.x;
        float h = (sprite.size.y > 0) ? sprite.size.y * sprite.scale.y : th * sprite.scale.y;
        float x = sprite.position.x - sprite.origin.x;
        float y = sprite.position.y - sprite.origin.y;
        sf::Color color = SFMLConv::toSFML(sprite.color);
        std::size_t base = m_impl->spriteBatch.getVertexCount();
        m_impl->spriteBatch.resize(base + 4);
        m_impl->spriteBatch[base + 0] = {{x,     y},     color, {tx,      ty}};
        m_impl->spriteBatch[base + 1] = {{x + w, y},     color, {tx + tw, ty}};
        m_impl->spriteBatch[base + 2] = {{x + w, y + h}, color, {tx + tw, ty + th}};
        m_impl->spriteBatch[base + 3] = {{x,     y + h}, color, {tx,      ty + th}};
        return;
    }

    sf::Sprite sfSprite(*it->second);
    sfSprite.setPosition(SFMLConv::toSFML(sprite.position));
    sfSprite.setRotation(sprite.rotation);

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

    if(sprite.textureRect.width > 0 && sprite.textureRect.height > 0)
        sfSprite.setTextureRect(SFMLConv::toSFML(sprite.textureRect));

    sf::RenderStates states;
    states.blendMode = SFMLConv::toSFML(sprite.blendMode);

    ShaderHandle shaderToUse = (sprite.shader != INVALID_HANDLE) ? sprite.shader : m_impl->boundShader;
    if(shaderToUse != INVALID_HANDLE) {
        auto sit = m_impl->shaders.find(shaderToUse);
        if(sit != m_impl->shaders.end() && sit->second) {
            sit->second->setUniform("tex", sf::Shader::CurrentTexture);
            sf::Vector2u texSize = it->second->getSize();
            sit->second->setUniform("texSize", sf::Vector2f(
                static_cast<float>(texSize.x), static_cast<float>(texSize.y)));
            states.shader = sit->second.get();
        }
    }

    m_impl->activeRenderTarget->draw(sfSprite, states);
}

void SFMLGraphicsBackend::drawRect(const RectData& rect) {
    if(!m_impl->activeRenderTarget) return;

    if (m_impl->rectBatchActive && rect.outlineThickness == 0.0f && rect.rotation == 0.0f && m_impl->boundShader == INVALID_HANDLE) {
        if (m_impl->spriteBatchActive && m_impl->spriteBatch.getVertexCount() > 0) {
            auto fit = m_impl->textures.find(m_impl->spriteBatchTexture);
            if (fit != m_impl->textures.end() && fit->second) {
                sf::RenderStates st; st.texture = fit->second.get();
                m_impl->activeRenderTarget->draw(m_impl->spriteBatch, st);
            }
            m_impl->spriteBatch = sf::VertexArray(sf::Quads, 0);
            m_impl->spriteBatchTexture = INVALID_HANDLE;
        }
        float x = rect.position.x - rect.origin.x;
        float y = rect.position.y - rect.origin.y;
        float w = rect.size.x;
        float h = rect.size.y;
        sf::Color color = SFMLConv::toSFML(rect.fillColor);
        std::size_t base = m_impl->rectBatch.getVertexCount();
        m_impl->rectBatch.resize(base + 4);
        m_impl->rectBatch[base + 0] = {{x,     y},     color};
        m_impl->rectBatch[base + 1] = {{x + w, y},     color};
        m_impl->rectBatch[base + 2] = {{x + w, y + h}, color};
        m_impl->rectBatch[base + 3] = {{x,     y + h}, color};
        return;
    }

    sf::RectangleShape shape(SFMLConv::toSFML(rect.size));
    shape.setPosition(SFMLConv::toSFML(rect.position));
    shape.setRotation(rect.rotation);
    shape.setOrigin(SFMLConv::toSFML(rect.origin));
    shape.setFillColor(SFMLConv::toSFML(rect.fillColor));
    shape.setOutlineColor(SFMLConv::toSFML(rect.outlineColor));
    shape.setOutlineThickness(rect.outlineThickness);

    sf::RenderStates states;
    if(m_impl->boundShader != INVALID_HANDLE) {
        auto sit = m_impl->shaders.find(m_impl->boundShader);
        if(sit != m_impl->shaders.end() && sit->second)
            states.shader = sit->second.get();
    }
    m_impl->activeRenderTarget->draw(shape, states);
}

void SFMLGraphicsBackend::beginRectBatch() {
    m_impl->rectBatchActive = true;
    m_impl->rectBatch = sf::VertexArray(sf::Quads, 0);
}

void SFMLGraphicsBackend::endRectBatch() {
    if (!m_impl->rectBatchActive || !m_impl->activeRenderTarget) return;
    m_impl->rectBatchActive = false;
    if (m_impl->rectBatch.getVertexCount() > 0)
        m_impl->activeRenderTarget->draw(m_impl->rectBatch);
    m_impl->rectBatch = sf::VertexArray();
}

void SFMLGraphicsBackend::beginSpriteBatch() {
    m_impl->spriteBatchActive = true;
    m_impl->spriteBatch = sf::VertexArray(sf::Quads, 0);
    m_impl->spriteBatchTexture = INVALID_HANDLE;
}

void SFMLGraphicsBackend::endSpriteBatch() {
    if (!m_impl->spriteBatchActive || !m_impl->activeRenderTarget) return;
    m_impl->spriteBatchActive = false;
    if (m_impl->spriteBatch.getVertexCount() > 0 && m_impl->spriteBatchTexture != INVALID_HANDLE) {
        auto fit = m_impl->textures.find(m_impl->spriteBatchTexture);
        if (fit != m_impl->textures.end() && fit->second) {
            sf::RenderStates st; st.texture = fit->second.get();
            m_impl->activeRenderTarget->draw(m_impl->spriteBatch, st);
        }
    }
    m_impl->spriteBatch = sf::VertexArray();
    m_impl->spriteBatchTexture = INVALID_HANDLE;
}

void SFMLGraphicsBackend::drawText(const TextData& text) {
    if(text.font == INVALID_HANDLE || !m_impl->activeRenderTarget) return;

    // Flush pending batches so text always renders on top of previously batched sprites/rects
    if (m_impl->spriteBatchActive && m_impl->spriteBatch.getVertexCount() > 0 && m_impl->spriteBatchTexture != INVALID_HANDLE) {
        auto tfit = m_impl->textures.find(m_impl->spriteBatchTexture);
        if (tfit != m_impl->textures.end() && tfit->second) {
            sf::RenderStates st; st.texture = tfit->second.get();
            m_impl->activeRenderTarget->draw(m_impl->spriteBatch, st);
        }
        m_impl->spriteBatch = sf::VertexArray(sf::Quads, 0);
        m_impl->spriteBatchTexture = INVALID_HANDLE;
    }
    if (m_impl->rectBatchActive && m_impl->rectBatch.getVertexCount() > 0) {
        m_impl->activeRenderTarget->draw(m_impl->rectBatch);
        m_impl->rectBatch = sf::VertexArray(sf::Quads, 0);
    }

    auto fit = m_impl->fonts.find(text.font);
    if(fit == m_impl->fonts.end() || !fit->second) return;

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
    if(m_impl->boundShader != INVALID_HANDLE) {
        auto sit = m_impl->shaders.find(m_impl->boundShader);
        if(sit != m_impl->shaders.end() && sit->second)
            states.shader = sit->second.get();
    }
    m_impl->activeRenderTarget->draw(sfText, states);
}

ShaderHandle SFMLGraphicsBackend::loadShader(const String& vertexPath, const String& fragmentPath) {
    if(vertexPath.empty() || fragmentPath.empty()) return INVALID_HANDLE;
    auto shader = std::make_unique<sf::Shader>();
    if(!shader->loadFromFile(vertexPath, fragmentPath)) return INVALID_HANDLE;
    ShaderHandle handle = m_impl->nextShaderHandle++;
    m_impl->shaders[handle] = std::move(shader);
    return handle;
}

void SFMLGraphicsBackend::bindShader(ShaderHandle handle) {
    m_impl->boundShader = handle;
}

void SFMLGraphicsBackend::unbindShader() {
    m_impl->boundShader = INVALID_HANDLE;
}

void SFMLGraphicsBackend::setShaderParameter(ShaderHandle handle, const String& name, f32 value) {
    if(handle == INVALID_HANDLE) { LOG_WARN("setShaderParameter: INVALID_HANDLE for {}", name); return; }
    auto it = m_impl->shaders.find(handle);
    if(it != m_impl->shaders.end() && it->second)
        it->second->setUniform(name, value);
    else
        LOG_WARN("setShaderParameter: shader not found (handle={}) for uniform {}", handle, name);
}

void SFMLGraphicsBackend::setShaderParameter(ShaderHandle handle, const String& name, const Vec2f& value) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_impl->shaders.find(handle);
    if(it != m_impl->shaders.end() && it->second)
        it->second->setUniform(name, sf::Vector2f(value.x, value.y));
}

void SFMLGraphicsBackend::setShaderParameter(ShaderHandle handle, const String& name, i32 value) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_impl->shaders.find(handle);
    if(it != m_impl->shaders.end() && it->second)
        it->second->setUniform(name, value);
}

void SFMLGraphicsBackend::setShaderParameter(ShaderHandle handle, const String& name, const Vec3f& value) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_impl->shaders.find(handle);
    if(it != m_impl->shaders.end() && it->second)
        it->second->setUniform(name, sf::Glsl::Vec3(value.x, value.y, value.z));
}

void SFMLGraphicsBackend::setShaderParameterArray(ShaderHandle handle, const String& name, const f32* values, size_t count) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_impl->shaders.find(handle);
    if(it != m_impl->shaders.end() && it->second)
        it->second->setUniformArray(name, values, count);
}

void SFMLGraphicsBackend::setShaderParameterArray(ShaderHandle handle, const String& name, const Vec2f* values, size_t count) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_impl->shaders.find(handle);
    if(it != m_impl->shaders.end() && it->second) {
        std::vector<sf::Glsl::Vec2> sfmlValues(count);
        for(size_t i = 0; i < count; ++i)
            sfmlValues[i] = sf::Glsl::Vec2(values[i].x, values[i].y);
        it->second->setUniformArray(name, sfmlValues.data(), count);
    }
}

void SFMLGraphicsBackend::setShaderParameterArray(ShaderHandle handle, const String& name, const Vec3f* values, size_t count) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_impl->shaders.find(handle);
    if(it != m_impl->shaders.end() && it->second) {
        std::vector<sf::Glsl::Vec3> sfmlValues(count);
        for(size_t i = 0; i < count; ++i)
            sfmlValues[i] = sf::Glsl::Vec3(values[i].x, values[i].y, values[i].z);
        it->second->setUniformArray(name, sfmlValues.data(), count);
    }
}

void SFMLGraphicsBackend::unloadShader(ShaderHandle handle) {
    if(handle == INVALID_HANDLE) return;
    if(m_impl->boundShader == handle) m_impl->boundShader = INVALID_HANDLE;
    m_impl->shaders.erase(handle);
}

RenderTextureHandle SFMLGraphicsBackend::createRenderTexture(u32 width, u32 height) {
    if(width == 0 || height == 0) return INVALID_HANDLE;
    auto renderTexture = std::make_unique<sf::RenderTexture>();
    if(!renderTexture->create(width, height)) return INVALID_HANDLE;
    renderTexture->setSmooth(false);
    RenderTextureHandle handle = m_impl->nextRenderTextureHandle++;
    m_impl->renderTextures[handle] = std::move(renderTexture);
    return handle;
}

void SFMLGraphicsBackend::bindRenderTexture(RenderTextureHandle handle) {
    if(handle == INVALID_HANDLE || !m_impl->window) return;
    auto it = m_impl->renderTextures.find(handle);
    if(it != m_impl->renderTextures.end() && it->second) {
        sf::View view = m_impl->window->getView();
        view.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
        it->second->setView(view);
        m_impl->activeRenderTarget = it->second.get();
    }
}

void SFMLGraphicsBackend::unbindRenderTexture() {
    m_impl->activeRenderTarget = m_impl->window;
}

void SFMLGraphicsBackend::clearRenderTexture(RenderTextureHandle handle, const Color& color) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_impl->renderTextures.find(handle);
    if(it != m_impl->renderTextures.end() && it->second)
        it->second->clear(SFMLConv::toSFML(color));
}

void SFMLGraphicsBackend::displayRenderTexture(RenderTextureHandle handle) {
    if(handle == INVALID_HANDLE) return;
    auto it = m_impl->renderTextures.find(handle);
    if(it != m_impl->renderTextures.end() && it->second)
        it->second->display();
}

void SFMLGraphicsBackend::drawRenderTextureToScreen(RenderTextureHandle handle, ShaderHandle shader) {
    if(handle == INVALID_HANDLE || !m_impl->window) return;
    auto rtIt = m_impl->renderTextures.find(handle);
    if(rtIt == m_impl->renderTextures.end() || !rtIt->second) return;

    sf::View previousView = m_impl->window->getView();
    m_impl->window->setView(m_impl->window->getDefaultView());

    const sf::Texture& texture = rtIt->second->getTexture();
    sf::Vector2u texSize    = texture.getSize();
    sf::Vector2u windowSize = m_impl->window->getSize();

    sf::VertexArray quad(sf::Quads, 4);
    quad[0].position  = sf::Vector2f(0, 0);
    quad[1].position  = sf::Vector2f(static_cast<float>(windowSize.x), 0);
    quad[2].position  = sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y));
    quad[3].position  = sf::Vector2f(0, static_cast<float>(windowSize.y));
    quad[0].texCoords = sf::Vector2f(0, 0);
    quad[1].texCoords = sf::Vector2f(static_cast<float>(texSize.x), 0);
    quad[2].texCoords = sf::Vector2f(static_cast<float>(texSize.x), static_cast<float>(texSize.y));
    quad[3].texCoords = sf::Vector2f(0, static_cast<float>(texSize.y));
    quad[0].color = quad[1].color = quad[2].color = quad[3].color = sf::Color::White;

    sf::RenderStates states;
    states.texture = &texture;

    if(shader != INVALID_HANDLE) {
        auto shaderIt = m_impl->shaders.find(shader);
        if(shaderIt != m_impl->shaders.end() && shaderIt->second) {
            shaderIt->second->setUniform("tex", sf::Shader::CurrentTexture);
            shaderIt->second->setUniform("texSize", sf::Vector2f(
                static_cast<float>(texSize.x), static_cast<float>(texSize.y)));
            states.shader = shaderIt->second.get();
        }
    }

    m_impl->window->draw(quad, states);
    m_impl->window->setView(previousView);
}

void SFMLGraphicsBackend::drawRenderTextureToRenderTexture(RenderTextureHandle source, RenderTextureHandle dest, ShaderHandle shader) {
    if(source == INVALID_HANDLE || dest == INVALID_HANDLE) return;
    auto srcIt  = m_impl->renderTextures.find(source);
    if(srcIt == m_impl->renderTextures.end() || !srcIt->second) return;
    auto destIt = m_impl->renderTextures.find(dest);
    if(destIt == m_impl->renderTextures.end() || !destIt->second) return;

    sf::View previousView = destIt->second->getView();
    destIt->second->setView(destIt->second->getDefaultView());

    const sf::Texture& texture = srcIt->second->getTexture();
    sf::Vector2u texSize  = texture.getSize();
    sf::Vector2u destSize = destIt->second->getSize();

    sf::VertexArray quad(sf::Quads, 4);
    quad[0].position  = sf::Vector2f(0, 0);
    quad[1].position  = sf::Vector2f(static_cast<float>(destSize.x), 0);
    quad[2].position  = sf::Vector2f(static_cast<float>(destSize.x), static_cast<float>(destSize.y));
    quad[3].position  = sf::Vector2f(0, static_cast<float>(destSize.y));
    quad[0].texCoords = sf::Vector2f(0, 0);
    quad[1].texCoords = sf::Vector2f(static_cast<float>(texSize.x), 0);
    quad[2].texCoords = sf::Vector2f(static_cast<float>(texSize.x), static_cast<float>(texSize.y));
    quad[3].texCoords = sf::Vector2f(0, static_cast<float>(texSize.y));
    quad[0].color = quad[1].color = quad[2].color = quad[3].color = sf::Color::White;

    sf::RenderStates states;
    states.texture = &texture;

    if(shader != INVALID_HANDLE) {
        auto shaderIt = m_impl->shaders.find(shader);
        if(shaderIt != m_impl->shaders.end() && shaderIt->second) {
            shaderIt->second->setUniform("tex", sf::Shader::CurrentTexture);
            shaderIt->second->setUniform("texSize", sf::Vector2f(
                static_cast<float>(texSize.x), static_cast<float>(texSize.y)));
            states.shader = shaderIt->second.get();
        }
    }

    destIt->second->draw(quad, states);
    destIt->second->setView(previousView);
}

void SFMLGraphicsBackend::unloadRenderTexture(RenderTextureHandle handle) {
    if(handle == INVALID_HANDLE) return;
    m_impl->renderTextures.erase(handle);
}

sf::Texture* SFMLGraphicsBackend::getSFMLTexture(TextureHandle handle) const {
    if(handle == INVALID_HANDLE) return nullptr;
    auto it = m_impl->textures.find(handle);
    return (it != m_impl->textures.end()) ? it->second.get() : nullptr;
}

sf::Font* SFMLGraphicsBackend::getSFMLFont(FontHandle handle) const {
    if(handle == INVALID_HANDLE) return nullptr;
    auto it = m_impl->fonts.find(handle);
    return (it != m_impl->fonts.end()) ? it->second : nullptr;
}

void SFMLGraphicsBackend::registerFont(FontHandle handle, sf::Font* font) {
    if(handle == INVALID_HANDLE || !font) return;
    m_impl->fonts[handle] = font;
}

sf::RenderWindow* SFMLGraphicsBackend::getRenderWindow() const {
    return m_impl->window;
}

}