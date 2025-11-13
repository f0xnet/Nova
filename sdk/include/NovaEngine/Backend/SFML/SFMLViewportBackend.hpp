#pragma once
#include "../Interfaces/IViewportBackend.hpp"
#include "SFMLConversions.hpp"
#include <SFML/Graphics.hpp>

namespace NovaEngine {
class SFMLViewportBackend : public IViewportBackend {
public:
    SFMLViewportBackend();
    
    void setWindow(sf::RenderWindow* window) { m_window = window; }
    
    void setViewport(const Rect& viewport) override;
    Rect getViewport() const override;
    void resetViewport() override;
    
    void setView(const ViewportData& view) override;
    ViewportData getView() const override;
    void resetView() override;
    
    void setViewCenter(const Vec2f& center) override;
    Vec2f getViewCenter() const override;
    
    void setViewSize(const Vec2f& size) override;
    Vec2f getViewSize() const override;
    
    void moveView(const Vec2f& offset) override;
    void zoomView(f32 factor) override;
    
    Vec2f screenToWorld(const Vec2i& point) const override;
    Vec2i worldToScreen(const Vec2f& point) const override;
    
private:
    sf::RenderWindow* m_window;
    ViewportData m_currentView;
};
}
