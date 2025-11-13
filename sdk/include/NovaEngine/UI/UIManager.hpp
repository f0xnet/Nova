#pragma once

#include "../Core/Types.hpp"
#include "UIComponent.hpp"
#include "../Core/Logger.hpp"
#include "../Events/Event.hpp"

#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>

namespace NovaEngine {

class UIManager {
public:
    using ActionCallback = std::function<void(const std::string& action, const std::string& value, const ID& componentID)>;

    UIManager();
    ~UIManager();

    void addComponent(const std::shared_ptr<UIComponent>& component);
    void removeComponent(const ID& id);
    void removeUI(const ID& uiID);
    void removeGroup(const ID& groupID);
    void setGroupActive(const ID& groupID, bool active);
    void setUIActive(const ID& uiID, bool active);
    void switchToGroup(const ID& uiID, const ID& newGroupID);
    void setLayerActive(i32 layer, bool active);
    void clear();

    void update(float deltaTime);
    void render() const;
    void dispatchEvent(const Event& event);

    void setActionCallback(ActionCallback callback);
    void handleAction(const std::string& action, const std::string& value, const ID& componentID);

    std::shared_ptr<UIComponent> getComponent(const ID& id);
    std::vector<std::shared_ptr<UIComponent>> getGroup(const ID& groupID);
    std::vector<std::shared_ptr<UIComponent>> getUI(const ID& uiID);
    i32 getMaxLayers() const;

private:
    struct ComponentInfo {
        std::shared_ptr<UIComponent> component;
        ID uiID;
        ID groupID;
        i32 layer;
    };

    std::unordered_map<ID, ComponentInfo> m_components;
    ActionCallback m_actionCallback;
    
    mutable std::vector<std::pair<i32, std::shared_ptr<UIComponent>>> m_renderCache;
    mutable bool m_renderCacheDirty;
    mutable i32 m_maxLayers;
    
    void invalidateRenderCache();
    void updateRenderCache() const;
};

}
