#include "NovaEngine/Backend/SFML/SFMLWindowBackend.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include <SFML/Window/VideoMode.hpp>

namespace NovaEngine {
SFMLWindowBackend::SFMLWindowBackend() : m_width(800), m_height(600), m_fullscreen(false), m_title("NovaEngine") {}
SFMLWindowBackend::~SFMLWindowBackend() { close(); }

bool SFMLWindowBackend::create(u32 width, u32 height, const String& title, bool fullscreen) {
    m_width = width;
    m_height = height;
    m_title = title; 
    m_fullscreen = fullscreen;
    
    try {
        sf::VideoMode mode(width, height);
        sf::Uint32 style;
        
        if(fullscreen) {
            style = sf::Style::None;
            auto desktopMode = sf::VideoMode::getDesktopMode();
            mode = sf::VideoMode(desktopMode.width, desktopMode.height);
            m_width = mode.width;
            m_height = mode.height;
            LOG_INFO("Creating BORDERLESS FULLSCREEN window");
            LOG_INFO("  Desktop resolution: {}x{}", desktopMode.width, desktopMode.height);
            LOG_INFO("  Style: sf::Style::None (borderless)");
        } else {
            style = sf::Style::Default;
            LOG_INFO("Creating WINDOWED mode: {}x{}", width, height);
        }
        
        m_window = std::make_unique<sf::RenderWindow>(mode, title, style);
        
        if(m_window) {
            if(fullscreen) {
                m_window->setPosition(sf::Vector2i(0, 0));
                LOG_INFO("Window positioned at (0, 0)");
            }
            m_width = m_window->getSize().x;
            m_height = m_window->getSize().y;
            LOG_INFO("Window created successfully: {}x{}", m_width, m_height);
            LOG_INFO("Window style: {}", fullscreen ? "BORDERLESS FULLSCREEN" : "WINDOWED");
            return true;
        }
        return false;
    } catch(...) { 
        return false; 
    }
}

void SFMLWindowBackend::close() { 
    if(m_window && m_window->isOpen()) m_window->close(); 
}

bool SFMLWindowBackend::isOpen() const { 
    return m_window && m_window->isOpen(); 
}

void SFMLWindowBackend::display() { 
    if(m_window) m_window->display(); 
}

void SFMLWindowBackend::clear(const Color& color) { 
    if(m_window) m_window->clear(SFMLConv::toSFML(color)); 
}

void SFMLWindowBackend::setTitle(const String& title) { 
    m_title = title; 
    if(m_window) m_window->setTitle(title); 
}

void SFMLWindowBackend::setVSync(bool enabled) { 
    if(m_window) m_window->setVerticalSyncEnabled(enabled); 
}

void SFMLWindowBackend::setFramerateLimit(u32 fps) { 
    if(m_window) m_window->setFramerateLimit(fps); 
}

u32 SFMLWindowBackend::getWidth() const { 
    return m_width; 
}

u32 SFMLWindowBackend::getHeight() const { 
    return m_height; 
}

void SFMLWindowBackend::setFullscreen(bool fullscreen) {
    if(m_fullscreen == fullscreen) return;
    
    m_fullscreen = fullscreen;
    if(m_window) {
        m_window->close();
        create(m_width, m_height, m_title, fullscreen);
    }
}

bool SFMLWindowBackend::hasFocus() const { 
    return m_window && m_window->hasFocus(); 
}

void SFMLWindowBackend::requestFocus() { 
    if(m_window) m_window->requestFocus(); 
}

void* SFMLWindowBackend::getNativeHandle() { 
    return m_window.get(); 
}

void SFMLWindowBackend::setIcon(u32 width, u32 height, const u8* pixels) { 
    if(m_window && pixels && width > 0 && height > 0) {
        m_window->setIcon(width, height, pixels); 
    }
}

void SFMLWindowBackend::setMouseCursorVisible(bool visible) {
    if(m_window) m_window->setMouseCursorVisible(visible);
}

void SFMLWindowBackend::setMouseCursorGrabbed(bool grabbed) {
    if(m_window) m_window->setMouseCursorGrabbed(grabbed);
}

}