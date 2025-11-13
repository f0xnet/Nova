#include "NovaEngine/UI/Components/Panel.hpp"
#include "NovaEngine/Backend/BackendManager.hpp"

namespace NovaEngine {

Panel::Panel() {
    m_background.fillColor = Color(50, 50, 50, 255);
    m_background.size = Vec2f(100, 100);
}

void Panel::setColor(const Color& color) {
    m_background.fillColor = color;
}

void Panel::onEvent(const Event& event) {}

void Panel::render() const {
    if(!m_visible) return;
    
    RectData rect = m_background;
    rect.position = m_position;
    rect.size = m_size;
    GRAPHICS().drawRect(rect);
}

Rect Panel::getBounds() const {
    Vec2f pos = m_position;
    return Rect(pos.x, pos.y, m_size.x, m_size.y);
}

}
