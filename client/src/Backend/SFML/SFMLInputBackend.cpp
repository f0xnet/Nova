#include "NovaEngine/Backend/SFML/SFMLInputBackend.hpp"

namespace NovaEngine {
SFMLInputBackend::SFMLInputBackend() : m_window(nullptr) {}

bool SFMLInputBackend::pollEvent(InputEvent& event) {
    if(!m_window) return false;
    sf::Event sfEvent;
    if(m_window->pollEvent(sfEvent)) {
        return convertEvent(sfEvent, event);
    }
    return false;
}

bool SFMLInputBackend::waitEvent(InputEvent& event) {
    if(!m_window) return false;
    sf::Event sfEvent;
    if(m_window->waitEvent(sfEvent)) {
        return convertEvent(sfEvent, event);
    }
    return false;
}

bool SFMLInputBackend::convertEvent(const sf::Event& se, InputEvent& oe) {
    switch(se.type) {
        case sf::Event::Closed: 
            oe.type = InputEventType::Closed; 
            return true;
            
        case sf::Event::Resized: 
            oe.type = InputEventType::Resized; 
            oe.size.width = se.size.width; 
            oe.size.height = se.size.height; 
            return true;
            
        case sf::Event::KeyPressed: 
            oe.type = InputEventType::KeyPressed; 
            oe.key.code = SFMLConv::fromSFMLKey(se.key.code); 
            oe.key.alt = se.key.alt; 
            oe.key.control = se.key.control; 
            oe.key.shift = se.key.shift; 
            oe.key.system = se.key.system; 
            return true;
            
        case sf::Event::KeyReleased: 
            oe.type = InputEventType::KeyReleased; 
            oe.key.code = SFMLConv::fromSFMLKey(se.key.code); 
            oe.key.alt = se.key.alt; 
            oe.key.control = se.key.control; 
            oe.key.shift = se.key.shift; 
            oe.key.system = se.key.system; 
            return true;
            
        case sf::Event::MouseButtonPressed: 
            oe.type = InputEventType::MouseButtonPressed; 
            oe.mouseButton.button = SFMLConv::fromSFMLMouse(se.mouseButton.button); 
            oe.mouseButton.x = se.mouseButton.x; 
            oe.mouseButton.y = se.mouseButton.y; 
            return true;
            
        case sf::Event::MouseButtonReleased: 
            oe.type = InputEventType::MouseButtonReleased; 
            oe.mouseButton.button = SFMLConv::fromSFMLMouse(se.mouseButton.button); 
            oe.mouseButton.x = se.mouseButton.x; 
            oe.mouseButton.y = se.mouseButton.y; 
            return true;
            
        case sf::Event::MouseMoved: 
            oe.type = InputEventType::MouseMoved; 
            oe.mouseMove.x = se.mouseMove.x; 
            oe.mouseMove.y = se.mouseMove.y; 
            return true;
            
        case sf::Event::TextEntered: 
            oe.type = InputEventType::TextEntered; 
            oe.text.unicode = se.text.unicode; 
            return true;
            
        default: 
            return false;
    }
}

bool SFMLInputBackend::isKeyPressed(KeyCode key) const { 
    return sf::Keyboard::isKeyPressed(SFMLConv::toSFMLKey(key)); 
}

bool SFMLInputBackend::isMouseButtonPressed(MouseButton button) const { 
    return sf::Mouse::isButtonPressed(SFMLConv::toSFMLMouse(button)); 
}

Vec2i SFMLInputBackend::getMousePosition() const { 
    if(!m_window) return Vec2i(0,0); 
    return SFMLConv::fromSFML(sf::Mouse::getPosition(*m_window)); 
}

void SFMLInputBackend::setMousePosition(const Vec2i& pos) { 
    if(m_window) sf::Mouse::setPosition(SFMLConv::toSFML(pos), *m_window); 
}

void SFMLInputBackend::setKeyRepeatEnabled(bool enabled) { 
    if(m_window) m_window->setKeyRepeatEnabled(enabled); 
}
}
