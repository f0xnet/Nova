#include "NovaEngine/Backend/SFML/SFMLViewportBackend.hpp"

namespace NovaEngine {
SFMLViewportBackend::SFMLViewportBackend() : m_window(nullptr) {}

void SFMLViewportBackend::setViewport(const Rect& viewport) {
    if(!m_window) return;
    sf::View view = m_window->getView();
    view.setViewport(SFMLConv::toSFML(viewport));
    m_window->setView(view);
    m_currentView.viewport = viewport;
}

Rect SFMLViewportBackend::getViewport() const {
    return m_window ? SFMLConv::fromSFML(m_window->getView().getViewport()) : Rect();
}

void SFMLViewportBackend::resetViewport() {
    if(m_window) {
        sf::View defaultView = m_window->getDefaultView();
        m_window->setView(defaultView);
        m_currentView.center = SFMLConv::fromSFML(defaultView.getCenter());
        m_currentView.size = SFMLConv::fromSFML(defaultView.getSize());
        m_currentView.rotation = defaultView.getRotation();
        m_currentView.viewport = SFMLConv::fromSFML(defaultView.getViewport());
    }
}

void SFMLViewportBackend::setView(const ViewportData& view) {
    if(!m_window) return;
    if(view.size.x <= 0.0f || view.size.y <= 0.0f) return;
    
    m_currentView = view;
    sf::View sfView;
    sfView.setCenter(SFMLConv::toSFML(view.center));
    sfView.setSize(SFMLConv::toSFML(view.size));
    sfView.setRotation(view.rotation);
    sfView.setViewport(SFMLConv::toSFML(view.viewport));
    m_window->setView(sfView);
}

ViewportData SFMLViewportBackend::getView() const { 
    return m_currentView; 
}

void SFMLViewportBackend::resetView() { 
    resetViewport(); 
}

void SFMLViewportBackend::setViewCenter(const Vec2f& center) { 
    m_currentView.center = center; 
    setView(m_currentView); 
}

Vec2f SFMLViewportBackend::getViewCenter() const { 
    return m_currentView.center; 
}

void SFMLViewportBackend::setViewSize(const Vec2f& size) { 
    if(size.x <= 0.0f || size.y <= 0.0f) return;
    m_currentView.size = size; 
    setView(m_currentView); 
}

Vec2f SFMLViewportBackend::getViewSize() const { 
    return m_currentView.size; 
}

void SFMLViewportBackend::moveView(const Vec2f& offset) {
    m_currentView.center.x += offset.x;
    m_currentView.center.y += offset.y;
    setView(m_currentView);
}

void SFMLViewportBackend::zoomView(f32 factor) {
    if(factor <= 0.0f) return;
    m_currentView.size.x *= factor;
    m_currentView.size.y *= factor;
    setView(m_currentView);
}

Vec2f SFMLViewportBackend::screenToWorld(const Vec2i& point) const {
    if(!m_window) return Vec2f();
    return SFMLConv::fromSFML(m_window->mapPixelToCoords(SFMLConv::toSFML(point)));
}

Vec2i SFMLViewportBackend::worldToScreen(const Vec2f& point) const {
    if(!m_window) return Vec2i();
    return SFMLConv::fromSFML(m_window->mapCoordsToPixel(SFMLConv::toSFML(point)));
}
}
