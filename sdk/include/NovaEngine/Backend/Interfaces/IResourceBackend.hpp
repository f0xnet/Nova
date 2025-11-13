#pragma once
#include "../Core/BackendTypes.hpp"

namespace NovaEngine {
class IResourceBackend {
public:
    virtual ~IResourceBackend() = default;
    
    virtual TextureHandle loadTexture(const String& path) = 0;
    virtual Vec2u getTextureSize(TextureHandle handle) const = 0;
    virtual void unloadTexture(TextureHandle handle) = 0;
    virtual bool isTextureLoaded(TextureHandle handle) const = 0;
    
    virtual FontHandle loadFont(const String& path) = 0;
    virtual void unloadFont(FontHandle handle) = 0;
    virtual bool isFontLoaded(FontHandle handle) const = 0;
    
    virtual SoundHandle loadSound(const String& path) = 0;
    virtual void unloadSound(SoundHandle handle) = 0;
    virtual bool isSoundLoaded(SoundHandle handle) const = 0;
    
    virtual MusicHandle loadMusic(const String& path) = 0;
    virtual void unloadMusic(MusicHandle handle) = 0;
    virtual bool isMusicLoaded(MusicHandle handle) const = 0;
    
    virtual void clearCache() = 0;
};
}
