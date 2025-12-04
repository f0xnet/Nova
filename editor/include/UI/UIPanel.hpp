#pragma once

#include <NovaEngine/UI/UIManager.hpp>
#include <NovaEngine/UI/Components/Text.hpp>
#include <NovaEngine/UI/Components/Button.hpp>
#include <NovaEngine/Core/Logger.hpp>
#include <string>
#include <memory>
#include <functional>

namespace NovaEditor {

/**
 * @brief Base class for all UI panels in the editor
 *
 * Provides common functionality for panel management:
 * - Show/hide with group activation
 * - Helper methods to reduce boilerplate
 * - Abstract update() for panel-specific logic
 *
 * Benefits:
 * - Eliminates duplicate show/hide/update code
 * - Reduces component access boilerplate
 * - Makes creating new panels 10x easier
 */
class UIPanel {
public:
    virtual ~UIPanel() = default;

    /**
     * @brief Show this panel (activates the UI group)
     */
    virtual void show() {
        m_uiManager.setGroupActive(m_groupID, true);
        update(); // Refresh data when showing
        LOG_INFO("Panel '{}' shown", m_groupID);
    }

    /**
     * @brief Hide this panel (deactivates the UI group)
     */
    virtual void hide() {
        m_uiManager.setGroupActive(m_groupID, false);
        LOG_INFO("Panel '{}' hidden", m_groupID);
    }

    /**
     * @brief Update panel data (must be implemented by derived classes)
     */
    virtual void update() = 0;

    /**
     * @brief Check if panel is currently visible
     */
    bool isVisible() const {
        return m_isVisible;
    }

protected:
    UIPanel(NovaEngine::UIManager& uiManager, const std::string& groupID)
        : m_uiManager(uiManager)
        , m_groupID(groupID)
        , m_isVisible(false)
    {}

    // ===== HELPER METHODS TO REDUCE BOILERPLATE =====

    /**
     * @brief Set text on a Text component (eliminates 6 lines of boilerplate)
     */
    void setText(const std::string& componentID, const std::string& value) {
        auto comp = m_uiManager.getComponent(componentID);
        if (comp) {
            auto text = std::dynamic_pointer_cast<NovaEngine::Text>(comp);
            if (text) {
                text->setString(value);
            } else {
                LOG_WARN("Component '{}' is not a Text component", componentID);
            }
        } else {
            LOG_WARN("Text component '{}' not found in panel '{}'", componentID, m_groupID);
        }
    }

    /**
     * @brief Set text on a Button component (eliminates 6 lines of boilerplate)
     */
    void setButtonText(const std::string& componentID, const std::string& value) {
        auto comp = m_uiManager.getComponent(componentID);
        if (comp) {
            auto button = std::dynamic_pointer_cast<NovaEngine::Button>(comp);
            if (button) {
                button->setText(value);
            } else {
                LOG_WARN("Component '{}' is not a Button component", componentID);
            }
        } else {
            LOG_WARN("Button component '{}' not found in panel '{}'", componentID, m_groupID);
        }
    }

    /**
     * @brief Get a component with automatic casting (eliminates 3 lines)
     */
    template<typename T>
    std::shared_ptr<T> getComponent(const std::string& componentID) {
        auto comp = m_uiManager.getComponent(componentID);
        return std::dynamic_pointer_cast<T>(comp);
    }

    /**
     * @brief Format float to string with specific precision
     */
    std::string formatFloat(float value, int decimals = 2) const {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.*f", decimals, value);
        return std::string(buffer);
    }

    /**
     * @brief Format integer to string
     */
    std::string formatInt(int value) const {
        return std::to_string(value);
    }

    // Member variables
    NovaEngine::UIManager& m_uiManager;
    std::string m_groupID;
    bool m_isVisible;
};

} // namespace NovaEditor
