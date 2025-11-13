#include "NovaEngine/UI/UIComponent.hpp"

namespace NovaEngine {

UIComponent::UIComponent()
    : m_position(0, 0)
    , m_size(0, 0)
    , m_visible(true)
    , m_active(true)
    , m_layer(0)
{}

UIComponent::~UIComponent() {}

void UIComponent::update(f32 deltaTime) {}

void UIComponent::setPosition(const Vec2f& pos) {
    m_position = pos;
}

void UIComponent::setSize(const Vec2f& size) {
    m_size = size;
}

void UIComponent::setVisible(bool visible) {
    m_visible = visible;
}

void UIComponent::setActive(bool active) {
    m_active = active;
}

void UIComponent::setID(const ID& id) {
    m_id = id;
}

void UIComponent::setGroupID(const ID& groupID) {
    m_groupID = groupID;
}

void UIComponent::setLayer(i32 layer) {
    m_layer = layer;
}

void UIComponent::setEffect(const std::string& effect) {
    m_effect = effect;
}

void UIComponent::setDescription(const std::string& description) {
    m_description = description;
}

void UIComponent::setUIID(const ID& uiID) {
    m_uiID = uiID;
}

const ID& UIComponent::getID() const { return m_id; }
const ID& UIComponent::getGroupID() const { return m_groupID; }
const ID& UIComponent::getUIID() const { return m_uiID; }
i32 UIComponent::getLayer() const { return m_layer; }
const std::string& UIComponent::getEffect() const { return m_effect; }
const std::string& UIComponent::getDescription() const { return m_description; }
bool UIComponent::isVisible() const { return m_visible; }
bool UIComponent::isActive() const { return m_active; }

}
