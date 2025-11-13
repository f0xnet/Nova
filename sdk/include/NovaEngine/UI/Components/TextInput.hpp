#pragma once

#include "../UIComponent.hpp"
#include "../../Backend/Core/BackendTypes.hpp"
#include <string>
#include <functional>

namespace NovaEngine {

class TextInput : public UIComponent {
public:
    TextInput();
    ~TextInput() override = default;

    void setFont(FontHandle font);
    const std::string& getText() const;
    void setText(const std::string& text);
    void setSize(const Vec2f& size) override;
    void setOnTextChanged(std::function<void(const std::string&)> callback);

    void onEvent(const Event& event) override;
    void update(f32 deltaTime) override;
    void render() const override;
    Rect getBounds() const override;

private:
    RectData m_box;
    TextData m_text;
    std::string m_buffer;

    bool m_focused;
    bool m_cursorVisible;
    f32 m_cursorTimer;

    std::function<void(const std::string&)> m_onTextChanged;

    void updateTextPosition();
    void updateVisual();
};

}
