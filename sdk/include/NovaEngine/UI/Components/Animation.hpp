#pragma once

#include "../UIComponent.hpp"
#include "../../Backend/Core/BackendTypes.hpp"
#include <vector>

namespace NovaEngine {

class Animation : public UIComponent {
public:
    Animation();
    ~Animation() override = default;

    void setTexture(TextureHandle texture);
    void setFrameSize(const Vec2u& size);
    void setFrameTime(f32 seconds);
    void setPlaying(bool playing);

    void update(f32 deltaTime) override;
    void onEvent(const Event& event) override;
    void render() const override;
    Rect getBounds() const override;

private:
    SpriteData m_sprite;
    Vec2u m_frameSize;
    f32 m_frameTime;
    f32 m_elapsed;
    bool m_playing;

    u32 m_currentFrame;
    u32 m_totalFrames;

    void updateTextureRect();
};

}
