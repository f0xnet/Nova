#pragma once
#include "../BackendManager.hpp"
#include "SFMLWindowBackend.hpp"
#include "SFMLGraphicsBackend.hpp"
#include "SFMLResourceBackend.hpp"
#include <SFML/Graphics.hpp>

namespace NovaEngine {
namespace SFMLHelpers {
    inline sf::RenderWindow* getWindow() {
        auto* window = dynamic_cast<SFMLWindowBackend*>(&BACKEND().window());
        return window ? window->getSFMLWindow() : nullptr;
    }
    
    inline sf::Texture* getTexture(TextureHandle handle) {
        auto* gfx = dynamic_cast<SFMLGraphicsBackend*>(&BACKEND().graphics());
        return gfx ? gfx->getSFMLTexture(handle) : nullptr;
    }
    
    inline sf::Font* getFont(FontHandle handle) {
        auto* gfx = dynamic_cast<SFMLGraphicsBackend*>(&BACKEND().graphics());
        return gfx ? gfx->getSFMLFont(handle) : nullptr;
    }
}
}
