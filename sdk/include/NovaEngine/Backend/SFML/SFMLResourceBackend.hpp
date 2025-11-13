#pragma once
#include "../Interfaces/IResourceBackend.hpp"
#include "SFMLConversions.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <memory>

namespace NovaEngine {
class SFMLGraphicsBackend;

class SFMLResourceBackend : public IResourceBackend {
public:
    SFMLResourceBackend();
    ~SFMLResourceBackend() override;
    
    void setGraphicsBackend(SFMLGraphicsBackend* gfx) { m_graphics = gfx; }
    
    TextureHandle loadTexture(const String& path) override;
    Vec2u getTextureSize(TextureHandle handle) const override;
    void unloadTexture(TextureHandle handle) override;
    bool isTextureLoaded(TextureHandle handle) const override;
    
    FontHandle loadFont(const String& path) override;
    void unloadFont(FontHandle handle) override;
    bool isFontLoaded(FontHandle handle) const override;
    
    SoundHandle loadSound(const String& path) override;
    void unloadSound(SoundHandle handle) override;
    bool isSoundLoaded(SoundHandle handle) const override;
    
    MusicHandle loadMusic(const String& path) override;
    void unloadMusic(MusicHandle handle) override;
    bool isMusicLoaded(MusicHandle handle) const override;
    
    void clearCache() override;
    
    sf::SoundBuffer* getSoundBuffer(SoundHandle handle);
    String getMusicPath(MusicHandle handle);
    
private:
    SFMLGraphicsBackend* m_graphics;
    std::unordered_map<String, TextureHandle> m_texturePaths;
    std::unordered_map<String, FontHandle> m_fontPaths;
    std::unordered_map<FontHandle, std::unique_ptr<sf::Font>> m_fonts;
    std::unordered_map<String, SoundHandle> m_soundPaths;
    std::unordered_map<SoundHandle, std::unique_ptr<sf::SoundBuffer>> m_soundBuffers;
    std::unordered_map<MusicHandle, String> m_musicPaths;
    u64 m_nextFontHandle = 1;
    u64 m_nextSoundHandle = 1;
    u64 m_nextMusicHandle = 1;
};
}
