#include "NovaEngine/UI/Components/TextInput.hpp"
#include "NovaEngine/Backend/BackendManager.hpp"

namespace NovaEngine {

TextInput::TextInput()
    : m_focused(false)
    , m_cursorVisible(true)
    , m_cursorTimer(0)
{
    m_box.fillColor = Color(255, 255, 255, 255);
    m_box.outlineColor = Color(0, 0, 0, 255);
    m_box.outlineThickness = 2;
    m_box.size = Vec2f(200, 30);
    
    m_text.characterSize = 20;
    m_text.fillColor = Color::Black;
}

void TextInput::setFont(FontHandle font) {
    m_text.font = font;
}

const std::string& TextInput::getText() const {
    return m_buffer;
}

void TextInput::setText(const std::string& text) {
    m_buffer = text;
    m_text.text = text;
}

void TextInput::setSize(const Vec2f& size) {
    UIComponent::setSize(size);
    m_box.size = size;
}

void TextInput::setOnTextChanged(std::function<void(const std::string&)> callback) {
    m_onTextChanged = callback;
}

void TextInput::onEvent(const Event& event) {
    if(!m_active || !m_visible) return;
    if(event.type != EventType::Input) return;
    
    const InputEvent& ie = event.inputEvent;
    
    if(ie.type == InputEventType::MouseButtonPressed) {
        Vec2f mousePos(static_cast<f32>(ie.mouseButton.x), static_cast<f32>(ie.mouseButton.y));
        m_focused = getBounds().contains(mousePos.x, mousePos.y);
    }
    
    if(m_focused && ie.type == InputEventType::TextEntered) {
        if(ie.text.unicode >= 32 && ie.text.unicode < 127) {
            m_buffer += static_cast<char>(ie.text.unicode);
            m_text.text = m_buffer;
            if(m_onTextChanged) m_onTextChanged(m_buffer);
        }
    }
    
    if(m_focused && ie.type == InputEventType::KeyPressed) {
        if(ie.key.code == KeyCode::Backspace && !m_buffer.empty()) {
            m_buffer.pop_back();
            m_text.text = m_buffer;
            if(m_onTextChanged) m_onTextChanged(m_buffer);
        }
    }
}

void TextInput::update(f32 deltaTime) {
    if(m_focused) {
        m_cursorTimer += deltaTime;
        if(m_cursorTimer >= 0.5f) {
            m_cursorVisible = !m_cursorVisible;
            m_cursorTimer = 0;
        }
    }
}

void TextInput::render() const {
    if(!m_visible) return;
    
    RectData box = m_box;
    box.position = m_position;
    GRAPHICS().drawRect(box);
    
    if(m_text.font != INVALID_HANDLE) {
        TextData text = m_text;
        text.position = Vec2f(
            m_position.x + 5,
            m_position.y + 5
        );
        GRAPHICS().drawText(text);
    }
}

Rect TextInput::getBounds() const {
    Vec2f pos = m_position;
    return Rect(pos.x, pos.y, m_size.x, m_size.y);
}

void TextInput::updateTextPosition() {}
void TextInput::updateVisual() {}

}
