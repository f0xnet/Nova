#include "NovaEngine/UI/Components/Slider.hpp"
#include "NovaEngine/Backend/BackendManager.hpp"

namespace NovaEngine {

Slider::Slider()
    : m_value(0.5f)
    , m_dragging(false)
{
    m_track.fillColor = Color(100, 100, 100, 255);
    m_track.size = Vec2f(200, 10);
    
    m_handle.fillColor = Color::White;
    m_handle.size = Vec2f(20, 20);
    
    updateHandlePosition();
}

void Slider::setValue(f32 value) {
    m_value = std::max(0.0f, std::min(1.0f, value));
    updateHandlePosition();
}

f32 Slider::getValue() const {
    return m_value;
}

void Slider::setSize(const Vec2f& size) {
    UIComponent::setSize(size);
    m_track.size = size;
    updateHandlePosition();
}

void Slider::setOnValueChanged(std::function<void(f32)> callback) {
    m_onValueChanged = callback;
}

void Slider::onEvent(const Event& event) {
    if(!m_active || !m_visible) return;
    if(event.type != EventType::Input) return;
    
    const InputEvent& ie = event.inputEvent;
    if(ie.type == InputEventType::MouseButtonPressed && ie.mouseButton.button == MouseButton::Left) {
        Vec2f mousePos(static_cast<f32>(ie.mouseButton.x), static_cast<f32>(ie.mouseButton.y));
        Rect bounds = getBounds();
        if(bounds.contains(mousePos.x, mousePos.y)) {
            m_dragging = true;
            updateValueFromMouse(mousePos);
        }
    }
    else if(ie.type == InputEventType::MouseButtonReleased && ie.mouseButton.button == MouseButton::Left) {
        m_dragging = false;
    }
    else if(ie.type == InputEventType::MouseMoved && m_dragging) {
        Vec2f mousePos(static_cast<f32>(ie.mouseMove.x), static_cast<f32>(ie.mouseMove.y));
        updateValueFromMouse(mousePos);
    }
}

void Slider::render() const {
    if(!m_visible) return;
    
    RectData track = m_track;
    track.position = m_position;
    GRAPHICS().drawRect(track);
    
    RectData handle = m_handle;
    GRAPHICS().drawRect(handle);
}

Rect Slider::getBounds() const {
    Vec2f pos = m_position;
    return Rect(pos.x, pos.y, m_track.size.x, m_track.size.y);
}

void Slider::updateHandlePosition() {
    Vec2f pos = m_position;
    f32 handleX = pos.x + (m_track.size.x - m_handle.size.x) * m_value;
    f32 handleY = pos.y + (m_track.size.y - m_handle.size.y) / 2.0f;
    m_handle.position = Vec2f(handleX, handleY);
}

void Slider::updateValueFromMouse(const Vec2f& mousePos) {
    Vec2f pos = m_position;
    f32 relativeX = mousePos.x - pos.x;
    f32 newValue = relativeX / m_track.size.x;
    
    setValue(newValue);
    
    if(m_onValueChanged) {
        m_onValueChanged(m_value);
    }
}

}
