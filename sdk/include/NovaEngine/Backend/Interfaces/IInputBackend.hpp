#pragma once
#include "../Core/BackendTypes.hpp"

namespace NovaEngine {
class IInputBackend {
public:
    virtual ~IInputBackend() = default;
    
    virtual bool pollEvent(InputEvent& event) = 0;
    virtual bool waitEvent(InputEvent& event) = 0;
    
    virtual bool isKeyPressed(KeyCode key) const = 0;
    virtual bool isMouseButtonPressed(MouseButton button) const = 0;
    virtual Vec2i getMousePosition() const = 0;
    virtual void setMousePosition(const Vec2i& position) = 0;
    
    virtual void setKeyRepeatEnabled(bool enabled) = 0;
};
}
