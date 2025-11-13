#pragma once

#include "../UIComponent.hpp"
#include "../../Backend/Core/BackendTypes.hpp"
#include "../../Core/Logger.hpp"
#include <functional>
#include <unordered_map>

namespace NovaEngine {

enum class ButtonState {
    NOT_HOVER = 0,
    HOVER = 1,
    PRESSED = 2
};

}

namespace std {
    template<>
    struct hash<NovaEngine::ButtonState> {
        std::size_t operator()(const NovaEngine::ButtonState& state) const {
            return std::hash<int>()(static_cast<int>(state));
        }
    };
}

namespace NovaEngine {

class Button : public UIComponent {
public:
    using ActionCallback = std::function<void(const std::string& action, const std::string& value, const ID& componentID)>;

    Button();
    ~Button() override = default;

    void setText(const std::string& text);
    void setFont(FontHandle font);
    void setFontSize(i32 fontSize);
    void setTextColor(const Color& color);
    void setSize(const Vec2f& size) override;
    
    void setTextures(TextureHandle normal, 
                    TextureHandle hover = INVALID_HANDLE, 
                    TextureHandle pressed = INVALID_HANDLE);
    
    void setAction(const std::string& action);
    void setValue(const std::string& value);
    void setHaveText(bool haveText);
    void setOnClick(std::function<void()> callback);
    void setOnClickWithAction(ActionCallback callback);

    const std::string& getAction() const;
    const std::string& getValue() const;
    ButtonState getCurrentState() const;
    bool getHaveText() const;

    void onEvent(const Event& event) override;
    void update(float deltaTime) override;
    void render() const override;
    Rect getBounds() const override;

private:
    RectData m_shape;
    SpriteData m_sprite;
    TextData m_text;

    ButtonState m_currentState;
    bool m_haveText;
    bool m_buttonPressed;
    std::string m_action;
    std::string m_value;

    std::unordered_map<ButtonState, TextureHandle> m_textures;

    std::function<void()> m_callback;
    ActionCallback m_actionCallback;

    void updateVisual();
    void updateTextPosition();
    ButtonState getStateFromMouse(const Vec2f& mousePos) const;
    void checkMouseEvent(const InputEvent& inputEvent);
    
    TextureHandle getTextureForState(ButtonState state) const;
};

}
