#pragma once

#include "../UIComponent.hpp"
#include "../../Backend/Core/BackendTypes.hpp"

namespace NovaEngine {

class Text : public UIComponent {
public:
    Text();
    ~Text() override = default;

    void setString(const std::string& str);
    void setFont(FontHandle font);
    void setCharacterSize(u32 size);
    void setTextColor(const Color& color);
    void setTextStyle(TextStyle style);
    void setSize(const Vec2f& size) override;

    const std::string& getString() const;
    const Color& getTextColor() const;
    u32 getCharacterSize() const;
    TextStyle getTextStyle() const;

    void onEvent(const Event& event) override;
    void update(float deltaTime) override;
    void render() const override;
    Rect getBounds() const override;

private:
    TextData m_textData;
    std::string m_content;

    void updateTextProperties();
};

}
