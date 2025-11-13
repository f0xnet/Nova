#pragma once

#include "../UIComponent.hpp"
#include "../../Backend/Core/BackendTypes.hpp"
#include <functional>

namespace NovaEngine {

class Slider : public UIComponent {
public:
    Slider();
    ~Slider() override = default;

    void setValue(f32 value);
    f32 getValue() const;
    void setSize(const Vec2f& size) override;
    void setOnValueChanged(std::function<void(f32)> callback);

    void onEvent(const Event& event) override;
    void render() const override;
    Rect getBounds() const override;

private:
    RectData m_track;
    RectData m_handle;

    f32 m_value;
    bool m_dragging;
    std::function<void(f32)> m_onValueChanged;

    void updateHandlePosition();
    void updateValueFromMouse(const Vec2f& mousePos);
};

}
