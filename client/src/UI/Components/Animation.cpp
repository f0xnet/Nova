#include "NovaEngine/UI/Components/Animation.hpp"
#include "NovaEngine/Backend/BackendManager.hpp"

namespace NovaEngine {

Animation::Animation()
    : m_frameSize(0, 0)
    , m_frameTime(0.1f)
    , m_elapsed(0)
    , m_playing(false)
    , m_currentFrame(0)
    , m_totalFrames(0)
{}

void Animation::setTexture(TextureHandle texture) {
    m_sprite.texture = texture;
}

void Animation::setFrameSize(const Vec2u& size) {
    m_frameSize = size;
    if(m_sprite.texture != INVALID_HANDLE) {
        Vec2u texSize = GRAPHICS().getTextureSize(m_sprite.texture);
        u32 cols = texSize.x / m_frameSize.x;
        u32 rows = texSize.y / m_frameSize.y;
        m_totalFrames = cols * rows;
    }
}

void Animation::setFrameTime(f32 seconds) {
    m_frameTime = seconds;
}

void Animation::setPlaying(bool playing) {
    m_playing = playing;
}

void Animation::update(f32 deltaTime) {
    if(!m_playing || m_totalFrames == 0) return;
    
    m_elapsed += deltaTime;
    if(m_elapsed >= m_frameTime) {
        m_elapsed = 0;
        m_currentFrame = (m_currentFrame + 1) % m_totalFrames;
        updateTextureRect();
    }
}

void Animation::onEvent(const Event& event) {}

void Animation::render() const {
    if(!m_visible || m_sprite.texture == INVALID_HANDLE) return;
    
    SpriteData sprite = m_sprite;
    sprite.position = m_position;
    GRAPHICS().drawSprite(sprite);
}

Rect Animation::getBounds() const {
    Vec2f pos = m_position;
    return Rect(pos.x, pos.y, static_cast<f32>(m_frameSize.x), static_cast<f32>(m_frameSize.y));
}

void Animation::updateTextureRect() {
    if(m_sprite.texture == INVALID_HANDLE) return;
    
    Vec2u texSize = GRAPHICS().getTextureSize(m_sprite.texture);
    u32 cols = texSize.x / m_frameSize.x;
    u32 col = m_currentFrame % cols;
    u32 row = m_currentFrame / cols;
    
    m_sprite.textureRect = IntRect(
        col * m_frameSize.x,
        row * m_frameSize.y,
        m_frameSize.x,
        m_frameSize.y
    );
}

}
