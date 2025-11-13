#pragma once
#include "../Interfaces/IInputBackend.hpp"
#include "SFMLConversions.hpp"
#include <SFML/Window.hpp>

namespace NovaEngine {
class SFMLInputBackend : public IInputBackend {
public:
    SFMLInputBackend();
    
    void setWindow(sf::RenderWindow* window) { m_window = window; }
    
    bool pollEvent(InputEvent& event) override;
    bool waitEvent(InputEvent& event) override;
    
    bool isKeyPressed(KeyCode key) const override;
    bool isMouseButtonPressed(MouseButton button) const override;
    Vec2i getMousePosition() const override;
    void setMousePosition(const Vec2i& position) override;
    
    void setKeyRepeatEnabled(bool enabled) override;
    
private:
    sf::RenderWindow* m_window;
    bool convertEvent(const sf::Event& sfEvent, InputEvent& outEvent);
};
}
