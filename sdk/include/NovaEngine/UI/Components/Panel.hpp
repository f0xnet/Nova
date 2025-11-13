#pragma once

#include "../UIComponent.hpp"
#include "../../Backend/Core/BackendTypes.hpp"

namespace NovaEngine {

class Panel : public UIComponent {
public:
    Panel();
    ~Panel() override = default;

    void setColor(const Color& color);
    void onEvent(const Event& event) override;
    void render() const override;
    Rect getBounds() const override;

private:
    RectData m_background;
};

}
