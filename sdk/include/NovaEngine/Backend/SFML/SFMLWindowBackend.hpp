#pragma once
#include "../Interfaces/IWindowBackend.hpp"
#include "SFMLConversions.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>

namespace NovaEngine {
class SFMLWindowBackend : public IWindowBackend {
public:
    SFMLWindowBackend();
    ~SFMLWindowBackend() override;
    
    bool create(u32 width, u32 height, const String& title, bool fullscreen) override;
    void close() override;
    bool isOpen() const override;
    void display() override;
    void clear(const Color& color) override;
    
    void setTitle(const String& title) override;
    void setVSync(bool enabled) override;
    void setFramerateLimit(u32 fps) override;
    
    u32 getWidth() const override;
    u32 getHeight() const override;
    
    void setFullscreen(bool fullscreen) override;
    
    bool hasFocus() const override;
    void requestFocus() override;
    
    void* getNativeHandle() override;
    
    void setIcon(u32 width, u32 height, const u8* pixels) override;
    void setMouseCursorVisible(bool visible) override;
    void setMouseCursorGrabbed(bool grabbed) override;
    
    sf::RenderWindow* getSFMLWindow() { return m_window.get(); }
    
private:
    std::unique_ptr<sf::RenderWindow> m_window;
    u32 m_width, m_height;
    bool m_fullscreen;
    String m_title;
};
}
