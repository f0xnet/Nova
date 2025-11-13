#pragma once
#include "../Core/BackendTypes.hpp"

namespace NovaEngine {
class IFontBackend {
public:
    virtual ~IFontBackend() = default;
    
    virtual FontHandle loadFont(const String& path) = 0;
    virtual void unloadFont(FontHandle handle) = 0;
    virtual bool isFontLoaded(FontHandle handle) const = 0;
    
    virtual TextMetrics measureText(const String& text, FontHandle font, u32 characterSize, TextStyle style = TextStyle::Regular) const = 0;
    virtual f32 getLineSpacing(FontHandle font, u32 characterSize) const = 0;
};
}
