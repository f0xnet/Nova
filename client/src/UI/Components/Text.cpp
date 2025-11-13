#include "NovaEngine/UI/Components/Text.hpp"
#include "NovaEngine/Backend/BackendManager.hpp"

namespace NovaEngine {

Text::Text() {
    m_textData.characterSize = 30;
    m_textData.fillColor = Color::White;
    m_textData.style = TextStyle::Regular;
}

void Text::setString(const std::string& str) {
    m_content = str;
    m_textData.text = str;
}

void Text::setFont(FontHandle font) {
    m_textData.font = font;
}

void Text::setCharacterSize(u32 size) {
    m_textData.characterSize = size;
}

void Text::setTextColor(const Color& color) {
    m_textData.fillColor = color;
}

void Text::setTextStyle(TextStyle style) {
    m_textData.style = style;
}

void Text::setSize(const Vec2f& size) {
    UIComponent::setSize(size);
}

const std::string& Text::getString() const { return m_content; }
const Color& Text::getTextColor() const { return m_textData.fillColor; }
u32 Text::getCharacterSize() const { return m_textData.characterSize; }
TextStyle Text::getTextStyle() const { return m_textData.style; }

void Text::onEvent(const Event& event) {}

void Text::update(f32 deltaTime) {}

void Text::render() const {
    if(!m_visible || m_textData.font == INVALID_HANDLE || m_content.empty()) return;
    
    TextData text = m_textData;
    text.position = m_position;
    GRAPHICS().drawText(text);
}

Rect Text::getBounds() const {
    if(m_textData.font == INVALID_HANDLE) return Rect(0,0,0,0);
    
    TextMetrics metrics = FONTS().measureText(m_textData.text, m_textData.font, m_textData.characterSize);
    return Rect(m_position.x, m_position.y, metrics.width, metrics.height);
}

void Text::updateTextProperties() {}

}