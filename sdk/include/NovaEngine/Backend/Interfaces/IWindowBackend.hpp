#pragma once
#include "../Core/BackendTypes.hpp"

namespace NovaEngine {
class IWindowBackend {
public:
    virtual ~IWindowBackend() = default;
    
    virtual bool create(u32 width, u32 height, const String& title, bool fullscreen = false) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    
    virtual void display() = 0;
    virtual void clear(const Color& color) = 0;
    
    virtual void setTitle(const String& title) = 0;
    virtual void setVSync(bool enabled) = 0;
    virtual void setFramerateLimit(u32 fps) = 0;
    
    virtual u32 getWidth() const = 0;
    virtual u32 getHeight() const = 0;
    
    virtual void setFullscreen(bool fullscreen) = 0;
    
    virtual bool hasFocus() const = 0;
    virtual void requestFocus() = 0;
    
    virtual void* getNativeHandle() = 0;
    
    virtual void setIcon(u32 width, u32 height, const u8* pixels) = 0;
    virtual void setMouseCursorVisible(bool visible) = 0;
    virtual void setMouseCursorGrabbed(bool grabbed) = 0;
};
}
