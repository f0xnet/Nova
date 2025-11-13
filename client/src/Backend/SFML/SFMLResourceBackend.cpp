#include "NovaEngine/Backend/SFML/SFMLResourceBackend.hpp"
#include "NovaEngine/Backend/SFML/SFMLGraphicsBackend.hpp"

namespace NovaEngine {
SFMLResourceBackend::SFMLResourceBackend() : m_graphics(nullptr) {}
SFMLResourceBackend::~SFMLResourceBackend() { clearCache(); }

TextureHandle SFMLResourceBackend::loadTexture(const String& path) {
    if(path.empty() || !m_graphics) return INVALID_HANDLE;
    auto it = m_texturePaths.find(path);
    if(it != m_texturePaths.end()) return it->second;
    TextureHandle handle = m_graphics->loadTexture(path);
    if(handle != INVALID_HANDLE) m_texturePaths[path] = handle;
    return handle;
}

Vec2u SFMLResourceBackend::getTextureSize(TextureHandle handle) const {
    return m_graphics ? m_graphics->getTextureSize(handle) : Vec2u(0, 0);
}

void SFMLResourceBackend::unloadTexture(TextureHandle handle) {
    if(handle == INVALID_HANDLE) return;
    if(m_graphics) m_graphics->unloadTexture(handle);
    for(auto it = m_texturePaths.begin(); it != m_texturePaths.end(); ) {
        if(it->second == handle) it = m_texturePaths.erase(it);
        else ++it;
    }
}

bool SFMLResourceBackend::isTextureLoaded(TextureHandle handle) const {
    if(handle == INVALID_HANDLE) return false;
    return m_graphics && m_graphics->getSFMLTexture(handle) != nullptr;
}

FontHandle SFMLResourceBackend::loadFont(const String& path) {
    if(path.empty()) return INVALID_HANDLE;
    auto it = m_fontPaths.find(path);
    if(it != m_fontPaths.end()) return it->second;
    auto font = std::make_unique<sf::Font>();
    if(!font->loadFromFile(path)) return INVALID_HANDLE;
    FontHandle handle = m_nextFontHandle++;
    m_fontPaths[path] = handle;
    if(m_graphics) m_graphics->registerFont(handle, font.get());
    m_fonts[handle] = std::move(font);
    return handle;
}

void SFMLResourceBackend::unloadFont(FontHandle handle) {
    if(handle == INVALID_HANDLE) return;
    m_fonts.erase(handle);
    for(auto it = m_fontPaths.begin(); it != m_fontPaths.end(); ) {
        if(it->second == handle) it = m_fontPaths.erase(it);
        else ++it;
    }
}

bool SFMLResourceBackend::isFontLoaded(FontHandle handle) const {
    if(handle == INVALID_HANDLE) return false;
    return m_fonts.find(handle) != m_fonts.end();
}

SoundHandle SFMLResourceBackend::loadSound(const String& path) {
    if(path.empty()) return INVALID_HANDLE;
    auto it = m_soundPaths.find(path);
    if(it != m_soundPaths.end()) return it->second;
    auto buffer = std::make_unique<sf::SoundBuffer>();
    if(!buffer->loadFromFile(path)) return INVALID_HANDLE;
    SoundHandle handle = m_nextSoundHandle++;
    m_soundPaths[path] = handle;
    m_soundBuffers[handle] = std::move(buffer);
    return handle;
}

void SFMLResourceBackend::unloadSound(SoundHandle handle) {
    if(handle == INVALID_HANDLE) return;
    m_soundBuffers.erase(handle);
    for(auto it = m_soundPaths.begin(); it != m_soundPaths.end(); ) {
        if(it->second == handle) it = m_soundPaths.erase(it);
        else ++it;
    }
}

bool SFMLResourceBackend::isSoundLoaded(SoundHandle handle) const {
    if(handle == INVALID_HANDLE) return false;
    return m_soundBuffers.find(handle) != m_soundBuffers.end();
}

MusicHandle SFMLResourceBackend::loadMusic(const String& path) {
    if(path.empty()) return INVALID_HANDLE;
    MusicHandle handle = m_nextMusicHandle++;
    m_musicPaths[handle] = path;
    return handle;
}

void SFMLResourceBackend::unloadMusic(MusicHandle handle) {
    if(handle == INVALID_HANDLE) return;
    m_musicPaths.erase(handle);
}

bool SFMLResourceBackend::isMusicLoaded(MusicHandle handle) const {
    if(handle == INVALID_HANDLE) return false;
    return m_musicPaths.find(handle) != m_musicPaths.end();
}

void SFMLResourceBackend::clearCache() {
    m_texturePaths.clear();
    m_fontPaths.clear();
    m_fonts.clear();
    m_soundPaths.clear();
    m_soundBuffers.clear();
    m_musicPaths.clear();
}

sf::SoundBuffer* SFMLResourceBackend::getSoundBuffer(SoundHandle handle) {
    if(handle == INVALID_HANDLE) return nullptr;
    auto it = m_soundBuffers.find(handle);
    return (it != m_soundBuffers.end()) ? it->second.get() : nullptr;
}

String SFMLResourceBackend::getMusicPath(MusicHandle handle) {
    if(handle == INVALID_HANDLE) return "";
    auto it = m_musicPaths.find(handle);
    return (it != m_musicPaths.end()) ? it->second : "";
}
}
