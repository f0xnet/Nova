#pragma once

#include "../UIComponent.hpp"
#include "../../Backend/Core/BackendTypes.hpp"

namespace NovaEngine {

class Image : public UIComponent {
public:
    Image();
    ~Image() override = default;

    void setTexture(TextureHandle texture);
    void setTexture(TextureHandle texture, bool resetRect);
    void setTextureRect(const IntRect& rect);
    void setColor(const Color& color);
    void setSize(const Vec2f& size) override;

    TextureHandle getTexture() const;
    const IntRect& getTextureRect() const;
    const Color& getColor() const;

    void onEvent(const Event& event) override;
    void update(float deltaTime) override;
    void render() const override;
    Rect getBounds() const override;

private:
    SpriteData m_sprite;

    void updateSpriteScale();
};

}
