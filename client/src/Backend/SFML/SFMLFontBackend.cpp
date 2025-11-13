#include "NovaEngine/Backend/SFML/SFMLFontBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLResourceBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLGraphicsBackend.hpp"

namespace NovaEngine {
SFMLFontBackend::SFMLFontBackend() : m_resources(nullptr), m_graphics(nullptr) {}

FontHandle SFMLFontBackend::loadFont(const String& path) {
    return m_resources ? m_resources->loadFont(path) : INVALID_HANDLE;
}

void SFMLFontBackend::unloadFont(FontHandle handle) {
    if(m_resources) m_resources->unloadFont(handle);
}

bool SFMLFontBackend::isFontLoaded(FontHandle handle) const {
    return m_resources ? m_resources->isFontLoaded(handle) : false;
}

TextMetrics SFMLFontBackend::measureText(const String& text, FontHandle font, u32 characterSize, TextStyle style) const {
    if(!m_graphics) return TextMetrics();
    sf::Font* sfFont = m_graphics->getSFMLFont(font);
    if(!sfFont) return TextMetrics();
    
    sf::Text tempText;
    tempText.setFont(*sfFont);
    tempText.setString(text);
    tempText.setCharacterSize(characterSize);
    tempText.setStyle(SFMLConv::toSFMLStyle(style));
    
    sf::FloatRect bounds = tempText.getLocalBounds();
    TextMetrics metrics;
    metrics.width = bounds.width;
    metrics.height = bounds.height;
    metrics.baseline = bounds.top;
    return metrics;
}

f32 SFMLFontBackend::getLineSpacing(FontHandle font, u32 characterSize) const {
    if(!m_graphics) return 0.0f;
    sf::Font* sfFont = m_graphics->getSFMLFont(font);
    return sfFont ? sfFont->getLineSpacing(characterSize) : 0.0f;
}
}
