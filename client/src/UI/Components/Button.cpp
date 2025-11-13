#include "NovaEngine/UI/Components/Button.hpp"
#include "NovaEngine/Backend/BackendManager.hpp"

namespace NovaEngine {

Button::Button()
    : m_currentState(ButtonState::NOT_HOVER)
    , m_haveText(true)
    , m_buttonPressed(false)
{
    m_shape.fillColor = Color(100, 100, 100, 255);
    m_shape.size = Vec2f(100, 30);
    m_text.text = "";
    m_text.characterSize = 20;
    m_text.fillColor = Color::White;
}

void Button::setText(const std::string& text) {
    m_text.text = text;
    updateTextPosition();
}

void Button::setFont(FontHandle font) {
    m_text.font = font;
    updateTextPosition();
}

void Button::setFontSize(i32 fontSize) {
    m_text.characterSize = static_cast<u32>(fontSize);
    updateTextPosition();
}

void Button::setTextColor(const Color& color) {
    m_text.fillColor = color;
}

void Button::setSize(const Vec2f& size) {
    UIComponent::setSize(size);
    m_shape.size = size;
    m_sprite.size = size;
    updateVisual();
    updateTextPosition();
}

void Button::setTextures(TextureHandle normal, TextureHandle hover, TextureHandle pressed) {
    m_textures[ButtonState::NOT_HOVER] = normal;
    if(hover != INVALID_HANDLE) m_textures[ButtonState::HOVER] = hover;
    if(pressed != INVALID_HANDLE) m_textures[ButtonState::PRESSED] = pressed;
    updateVisual();
}

void Button::setAction(const std::string& action) {
    m_action = action;
}

void Button::setValue(const std::string& value) {
    m_value = value;
}

void Button::setHaveText(bool haveText) {
    m_haveText = haveText;
}

void Button::setOnClick(std::function<void()> callback) {
    m_callback = callback;
}

void Button::setOnClickWithAction(ActionCallback callback) {
    m_actionCallback = callback;
}

const std::string& Button::getAction() const { return m_action; }
const std::string& Button::getValue() const { return m_value; }
ButtonState Button::getCurrentState() const { return m_currentState; }
bool Button::getHaveText() const { return m_haveText; }

void Button::onEvent(const Event& event) {
    if(!m_active || !m_visible) return;
    if(event.type == EventType::Input) {
        checkMouseEvent(event.inputEvent);
    }
}

void Button::update(f32 deltaTime) {}

void Button::render() const {
    if(!m_visible) return;
    
    TextureHandle tex = getTextureForState(m_currentState);
    if(tex != INVALID_HANDLE) {
        SpriteData sprite = m_sprite;
        sprite.texture = tex;
        sprite.position = m_position;
        sprite.size = m_size;
        GRAPHICS().drawSprite(sprite);
    } else {
        RectData rect = m_shape;
        rect.position = m_position;
        GRAPHICS().drawRect(rect);
    }
    
    if(m_haveText && m_text.font != INVALID_HANDLE && !m_text.text.empty()) {
        TextData text = m_text;
        text.position = Vec2f(
            m_position.x + m_text.position.x,
            m_position.y + m_text.position.y
        );
        GRAPHICS().drawText(text);
    }
}

Rect Button::getBounds() const {
    return Rect(m_position.x, m_position.y, m_size.x, m_size.y);
}

void Button::updateVisual() {}

void Button::updateTextPosition() {
    if(m_text.font == INVALID_HANDLE || m_text.text.empty()) return;
    
    TextMetrics metrics = FONTS().measureText(m_text.text, m_text.font, m_text.characterSize);
    
    f32 offsetY = metrics.height * 0.33f;
    
    m_text.position = Vec2f(
        (m_size.x - metrics.width) / 2.0f,
        (m_size.y - metrics.height) / 2.0f - offsetY
    );
}

ButtonState Button::getStateFromMouse(const Vec2f& mousePos) const {
    Rect bounds = getBounds();
    if(bounds.contains(mousePos.x, mousePos.y)) {
        return m_buttonPressed ? ButtonState::PRESSED : ButtonState::HOVER;
    }
    return ButtonState::NOT_HOVER;
}

void Button::checkMouseEvent(const InputEvent& inputEvent) {
    if(inputEvent.type == InputEventType::MouseMoved) {
        Vec2f mousePos(static_cast<f32>(inputEvent.mouseMove.x), 
                      static_cast<f32>(inputEvent.mouseMove.y));
        m_currentState = getStateFromMouse(mousePos);
    }
    else if(inputEvent.type == InputEventType::MouseButtonPressed) {
        if(inputEvent.mouseButton.button == MouseButton::Left) {
            Vec2f mousePos(static_cast<f32>(inputEvent.mouseButton.x), 
                          static_cast<f32>(inputEvent.mouseButton.y));
            if(getBounds().contains(mousePos.x, mousePos.y)) {
                m_buttonPressed = true;
                m_currentState = ButtonState::PRESSED;
            }
        }
    }
    else if(inputEvent.type == InputEventType::MouseButtonReleased) {
        if(inputEvent.mouseButton.button == MouseButton::Left && m_buttonPressed) {
            m_buttonPressed = false;
            Vec2f mousePos(static_cast<f32>(inputEvent.mouseButton.x), 
                          static_cast<f32>(inputEvent.mouseButton.y));
            if(getBounds().contains(mousePos.x, mousePos.y)) {
                if(m_callback) m_callback();
                if(m_actionCallback) m_actionCallback(m_action, m_value, m_id);
            }
            m_currentState = getStateFromMouse(mousePos);
        }
    }
}

TextureHandle Button::getTextureForState(ButtonState state) const {
    auto it = m_textures.find(state);
    if(it != m_textures.end()) return it->second;
    
    it = m_textures.find(ButtonState::NOT_HOVER);
    return (it != m_textures.end()) ? it->second : INVALID_HANDLE;
}

}