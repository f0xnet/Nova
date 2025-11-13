#pragma once
#include "../Interfaces/IFontBackend.hpp"
#include "SFMLConversions.hpp"
#include <SFML/Graphics.hpp>

namespace NovaEngine {
class SFMLResourceBackend;
class SFMLGraphicsBackend;

class SFMLFontBackend : public IFontBackend {
public:
    SFMLFontBackend();
    
    void setResourceBackend(SFMLResourceBackend* res) { m_resources = res; }
    void setGraphicsBackend(SFMLGraphicsBackend* gfx) { m_graphics = gfx; }
    
    FontHandle loadFont(const String& path) override;
    void unloadFont(FontHandle handle) override;
    bool isFontLoaded(FontHandle handle) const override;
    
    TextMetrics measureText(const String& text, FontHandle font, u32 characterSize, TextStyle style) const override;
    f32 getLineSpacing(FontHandle font, u32 characterSize) const override;
    
private:
    SFMLResourceBackend* m_resources;
    SFMLGraphicsBackend* m_graphics;
};
}
