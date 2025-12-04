#pragma once

#include <NovaEngine/UI/UIManager.hpp>
#include <NovaEngine/UI/Components/Text.hpp>
#include <NovaEngine/UI/Components/Button.hpp>
#include <NovaEngine/Core/Logger.hpp>
#include "../Core/DataBinding.hpp"
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

    // ===== DATA BINDING SUPPORT =====

    /**
     * @brief Bind a value to a text component for automatic updates
     *
     * When the bindable value changes, the text component is automatically updated.
     * This eliminates the need for manual update() calls.
     *
     * @param bindable The bindable value to observe
     * @param componentID The text component to update
     * @param formatter Optional function to format the value (default: toString)
     */
    template<typename T>
    void bindToText(Bindable<T>* bindable, const std::string& componentID,
                    std::function<std::string(const T&)> formatter = nullptr) {
        if (!formatter) {
            // Default formatter
            if constexpr (std::is_same_v<T, std::string>) {
                formatter = [](const T& val) { return val; };
            } else if constexpr (std::is_same_v<T, int>) {
                formatter = [](const T& val) { return std::to_string(val); };
            } else if constexpr (std::is_same_v<T, float>) {
                formatter = [this](const T& val) { return formatFloat(val, 2); };
            } else {
                formatter = [](const T& val) { return std::to_string(val); };
            }
        }

        // Create observer that updates text when value changes
        auto observer = [this, componentID, formatter](const T& newValue) {
            setText(componentID, formatter(newValue));
        };

        bindable->addObserver(observer);
        m_bindingContext.addBinding(componentID, bindable, observer);

        // Initial update
        setText(componentID, formatter(bindable->get()));
    }

    /**
     * @brief Bind a boolean to a button text (e.g., "ON"/"OFF")
     */
    void bindBoolToButton(Bindable<bool>* bindable, const std::string& componentID,
                          const std::string& trueText, const std::string& falseText) {
        auto observer = [this, componentID, trueText, falseText](const bool& value) {
            setButtonText(componentID, value ? trueText : falseText);
        };

        bindable->addObserver(observer);
        m_bindingContext.addBinding(componentID, bindable, observer);

        // Initial update
        setButtonText(componentID, bindable->get() ? trueText : falseText);
    }

    // Member variables
    NovaEngine::UIManager& m_uiManager;
    std::string m_groupID;
    bool m_isVisible;
    BindingContext m_bindingContext;
};

} // namespace NovaEditor
