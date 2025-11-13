#include "NovaEngine/UI/UIManager.hpp"
#include "NovaEngine/Backend/BackendManager.hpp"

namespace NovaEngine {

UIManager::UIManager() : m_renderCacheDirty(true), m_maxLayers(0) {
    LOG_INFO("UIManager created with backend support");
}

UIManager::~UIManager() {
    clear();
    LOG_INFO("UIManager destroyed");
}

void UIManager::addComponent(const std::shared_ptr<UIComponent>& component) {
    if (!component) {
        LOG_WARN("Trying to add null UIComponent");
        return;
    }

    const ID& id = component->getID();
    if (id.empty()) {
        LOG_WARN("Trying to add UIComponent with empty ID");
        return;
    }

    if (m_components.find(id) != m_components.end()) {
        LOG_WARN("UIComponent with ID '{}' already exists. Replacing it.", id);
    }

    ComponentInfo info;
    info.component = component;
    info.uiID = component->getUIID();
    info.groupID = component->getGroupID();
    info.layer = component->getLayer();

    m_components[id] = info;
    
    if (info.layer > m_maxLayers) {
        m_maxLayers = info.layer;
    }
    
    invalidateRenderCache();
    LOG_DEBUG("UIComponent '{}' added to UI '{}', group '{}', layer {}", id, info.uiID, info.groupID, info.layer);
}

void UIManager::removeComponent(const ID& id) {
    if (m_components.erase(id) > 0) {
        invalidateRenderCache();
        LOG_DEBUG("UIComponent '{}' removed from UIManager", id);
    } else {
        LOG_WARN("UIComponent '{}' not found in UIManager", id);
    }
}

void UIManager::removeUI(const ID& uiID) {
    size_t removedCount = 0;
    auto it = m_components.begin();
    
    while (it != m_components.end()) {
        if (it->second.uiID == uiID) {
            LOG_DEBUG("Removing component '{}' from UI '{}'", it->first, uiID);
            it = m_components.erase(it);
            removedCount++;
        } else {
            ++it;
        }
    }
    
    if (removedCount > 0) {
        invalidateRenderCache();
        LOG_INFO("Removed {} components from UI '{}'", removedCount, uiID);
    } else {
        LOG_WARN("No components found in UI '{}'", uiID);
    }
}

void UIManager::removeGroup(const ID& groupID) {
    size_t removedCount = 0;
    auto it = m_components.begin();
    
    while (it != m_components.end()) {
        if (it->second.groupID == groupID) {
            LOG_DEBUG("Removing component '{}' from group '{}'", it->first, groupID);
            it = m_components.erase(it);
            removedCount++;
        } else {
            ++it;
        }
    }
    
    if (removedCount > 0) {
        invalidateRenderCache();
        LOG_INFO("Removed {} components from group '{}'", removedCount, groupID);
    } else {
        LOG_WARN("No components found in group '{}'", groupID);
    }
}

void UIManager::setGroupActive(const ID& groupID, bool active) {
    size_t affectedCount = 0;
    
    for (auto& [id, info] : m_components) {
        if (info.groupID == groupID) {
            info.component->setActive(active);
            affectedCount++;
        }
    }
    
    LOG_INFO("Set {} components in group '{}' to active: {}", affectedCount, groupID, active);
}

void UIManager::setUIActive(const ID& uiID, bool active) {
    size_t affectedCount = 0;
    
    for (auto& [id, info] : m_components) {
        if (info.uiID == uiID) {
            info.component->setActive(active);
            affectedCount++;
        }
    }
    
    LOG_INFO("Set {} components in UI '{}' to active: {}", affectedCount, uiID, active);
}

void UIManager::switchToGroup(const ID& uiID, const ID& newGroupID) {
    size_t affectedCount = 0;
    
    for (auto& [id, info] : m_components) {
        if (info.uiID == uiID) {
            if (info.groupID == "main") {
                info.component->setActive(true);
            } else {
                bool shouldBeActive = (info.groupID == newGroupID);
                info.component->setActive(shouldBeActive);
                if (shouldBeActive) affectedCount++;
            }
        }
    }
    
    LOG_INFO("Switched UI '{}' to group '{}' - {} components activated", uiID, newGroupID, affectedCount);
}

void UIManager::setLayerActive(i32 layer, bool active) {
    size_t affectedCount = 0;
    
    for (auto& [id, info] : m_components) {
        if (info.layer == layer) {
            info.component->setActive(active);
            affectedCount++;
        }
    }
    
    LOG_INFO("Set {} components in layer {} to active: {}", affectedCount, layer, active);
}

void UIManager::clear() {
    size_t count = m_components.size();
    m_components.clear();
    m_maxLayers = 0;
    invalidateRenderCache();
    LOG_INFO("All UIComponents cleared from UIManager (removed {} components)", count);
}

void UIManager::update(float deltaTime) {
    for (auto& [id, info] : m_components) {
        if (info.component->isActive()) {
            info.component->update(deltaTime);
        }
    }
}

void UIManager::render() const {
    if (m_renderCacheDirty) {
        updateRenderCache();
    }

    // Rendu par layer de 0 à maxLayers
    for (const auto& [layer, component] : m_renderCache) {
        if (component->isVisible() && component->isActive()) {
            component->render();
        }
    }
}

void UIManager::dispatchEvent(const Event& event) {
    if (m_renderCacheDirty) {
        updateRenderCache();
    }

    // Parcourir en ordre inverse de layer pour donner priorité aux layers supérieurs
    for (auto it = m_renderCache.rbegin(); it != m_renderCache.rend(); ++it) {
        const auto& component = it->second;
        if (component->isActive() && component->isVisible()) {
            component->onEvent(event);
        }
    }
}

void UIManager::setActionCallback(ActionCallback callback) {
    m_actionCallback = std::move(callback);
    LOG_DEBUG("Action callback set in UIManager");
}

void UIManager::handleAction(const std::string& action, const std::string& value, const ID& componentID) {
    if (m_actionCallback) {
        m_actionCallback(action, value, componentID);
        LOG_DEBUG("Action handled: '{}' with value '{}' from component '{}'", action, value, componentID);
    } else {
        LOG_WARN("Action '{}' triggered but no callback set", action);
    }
}

std::shared_ptr<UIComponent> UIManager::getComponent(const ID& id) {
    auto it = m_components.find(id);
    if (it != m_components.end()) {
        return it->second.component;
    }
    return nullptr;
}

std::vector<std::shared_ptr<UIComponent>> UIManager::getGroup(const ID& groupID) {
    std::vector<std::shared_ptr<UIComponent>> result;
    
    for (const auto& [id, info] : m_components) {
        if (info.groupID == groupID) {
            result.push_back(info.component);
        }
    }
    
    return result;
}

std::vector<std::shared_ptr<UIComponent>> UIManager::getUI(const ID& uiID) {
    std::vector<std::shared_ptr<UIComponent>> result;
    
    for (const auto& [id, info] : m_components) {
        if (info.uiID == uiID) {
            result.push_back(info.component);
        }
    }
    
    return result;
}

i32 UIManager::getMaxLayers() const {
    return m_maxLayers;
}

void UIManager::invalidateRenderCache() {
    m_renderCacheDirty = true;
}

void UIManager::updateRenderCache() const {
    m_renderCache.clear();
    
    for (const auto& [id, info] : m_components) {
        m_renderCache.emplace_back(info.layer, info.component);
    }
    
    // Trier par layer (ordre croissant: 0, 1, 2, 3...)
    std::sort(m_renderCache.begin(), m_renderCache.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        });
    
    m_renderCacheDirty = false;
    LOG_TRACE("Render cache updated with {} components across {} layers", m_renderCache.size(), m_maxLayers + 1);
}

}