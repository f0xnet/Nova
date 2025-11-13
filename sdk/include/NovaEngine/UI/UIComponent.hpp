#pragma once

#include "../Core/Types.hpp"
#include "../Core/Logger.hpp"
#include "../Backend/Core/BackendTypes.hpp"
#include "../Events/EventHandler.hpp"

#include <string>

namespace NovaEngine {

class UIComponent : public EventHandler {
public:
    UIComponent();
    virtual ~UIComponent();

    virtual void update(f32 deltaTime);
    virtual void render() const = 0;
    virtual void onEvent(const Event& event) override = 0;

    void setPosition(const Vec2f& pos);
    virtual void setSize(const Vec2f& size);
    void setVisible(bool visible);
    void setActive(bool active);
    void setID(const ID& id);

    void setGroupID(const ID& groupID);
    void setLayer(i32 layer);
    void setEffect(const std::string& effect);
    void setDescription(const std::string& description);
    void setUIID(const ID& uiID);

    const ID& getID() const;
    const ID& getGroupID() const;
    const ID& getUIID() const;
    i32 getLayer() const;
    const std::string& getEffect() const;
    const std::string& getDescription() const;
    bool isVisible() const;
    bool isActive() const;

    Vec2f getPosition() const { return m_position; }
    Vec2f getSize() const { return m_size; }

    virtual Rect getBounds() const = 0;

protected:
    ID m_id;
    Vec2f m_position;
    Vec2f m_size;
    bool m_visible;
    bool m_active;
    
    ID m_groupID;
    ID m_uiID;
    i32 m_layer;
    std::string m_effect;
    std::string m_description;
};

}
