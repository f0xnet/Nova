#pragma once
#include "../Core/BackendTypes.hpp"
#include "SFMLConversions.hpp"
#include "SFMLHelpers.hpp"
#include <SFML/Graphics.hpp>

namespace NovaEngine {
class SFMLDrawableWrapper {
public:
    static void draw(const sf::Drawable& drawable, const Transform& transform = Transform()) {
        auto* window = SFMLHelpers::getWindow();
        if(!window) return;
        
        sf::RenderStates states;
        sf::Transform sfTransform;
        sfTransform.translate(SFMLConv::toSFML(transform.position));
        sfTransform.rotate(transform.rotation, SFMLConv::toSFML(transform.origin));
        sfTransform.scale(SFMLConv::toSFML(transform.scale), SFMLConv::toSFML(transform.origin));
        states.transform = sfTransform;
        
        window->draw(drawable, states);
    }
};
}
