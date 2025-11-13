#include "NovaEngine/UI/Components/Image.hpp"
#include "NovaEngine/Backend/BackendManager.hpp"

namespace NovaEngine {

Image::Image() {
    m_sprite.color = Color::White;
    m_sprite.scale = Vec2f(1, 1);
    LOG_TRACE("Image component created with backend support");
}

void Image::setTexture(TextureHandle texture) {
    m_sprite.texture = texture;
    if(texture != INVALID_HANDLE) {
        Vec2u size = GRAPHICS().getTextureSize(texture);
        if(m_size.x == 0 && m_size.y == 0) {
            m_size = Vec2f(static_cast<f32>(size.x), static_cast<f32>(size.y));
        }
    }
}

void Image::setTexture(TextureHandle texture, bool resetRect) {
    setTexture(texture);
    if(resetRect && texture != INVALID_HANDLE) {
        Vec2u size = GRAPHICS().getTextureSize(texture);
        m_sprite.textureRect = IntRect(0, 0, static_cast<i32>(size.x), static_cast<i32>(size.y));
    }
}

void Image::setTextureRect(const IntRect& rect) {
    m_sprite.textureRect = rect;
}

void Image::setColor(const Color& color) {
    m_sprite.color = color;
}

void Image::setSize(const Vec2f& size) {
    UIComponent::setSize(size);
}

TextureHandle Image::getTexture() const { return m_sprite.texture; }
const IntRect& Image::getTextureRect() const { return m_sprite.textureRect; }
const Color& Image::getColor() const { return m_sprite.color; }

void Image::onEvent(const Event& event) {}

void Image::update(f32 deltaTime) {}

void Image::render() const {
    if(!m_visible || m_sprite.texture == INVALID_HANDLE) return;
    
    SpriteData sprite = m_sprite;
    sprite.position = m_position;
    sprite.size = m_size;
    
    GRAPHICS().drawSprite(sprite);
}

Rect Image::getBounds() const {
    Vec2f pos = m_position;
    return Rect(pos.x, pos.y, m_size.x, m_size.y);
}

void Image::updateSpriteScale() {}

}