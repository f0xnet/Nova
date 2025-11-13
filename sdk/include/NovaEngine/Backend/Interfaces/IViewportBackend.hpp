#pragma once
#include "../Core/BackendTypes.hpp"

namespace NovaEngine {
class IViewportBackend {
public:
    virtual ~IViewportBackend() = default;
    
    virtual void setViewport(const Rect& viewport) = 0;
    virtual Rect getViewport() const = 0;
    virtual void resetViewport() = 0;
    
    virtual void setView(const ViewportData& view) = 0;
    virtual ViewportData getView() const = 0;
    virtual void resetView() = 0;
    
    virtual void setViewCenter(const Vec2f& center) = 0;
    virtual Vec2f getViewCenter() const = 0;
    
    virtual void setViewSize(const Vec2f& size) = 0;
    virtual Vec2f getViewSize() const = 0;
    
    virtual void moveView(const Vec2f& offset) = 0;
    virtual void zoomView(f32 factor) = 0;
    
    virtual Vec2f screenToWorld(const Vec2i& point) const = 0;
    virtual Vec2i worldToScreen(const Vec2f& point) const = 0;
};
}
