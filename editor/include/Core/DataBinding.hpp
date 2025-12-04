#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <string>

namespace NovaEditor {

/**
 * @brief Data binding system for automatic UI updates
 *
 * This system eliminates manual update() calls by automatically notifying
 * observers when bound data changes. Panels can subscribe to data changes
 * and update themselves automatically.
 *
 * Example usage:
 * @code
 * Bindable<float> brightness(0.0f);
 * brightness.addObserver([](float newValue) {
 *     // Update UI automatically
 *     updateBrightnessDisplay(newValue);
 * });
 * brightness.set(0.5f); // Observers notified automatically
 * @endcode
 */
template<typename T>
class Bindable {
public:
    using Observer = std::function<void(const T&)>;

    /**
     * @brief Construct bindable value
     */
    explicit Bindable(const T& initialValue = T())
        : m_value(initialValue)
    {}

    /**
     * @brief Get current value
     */
    const T& get() const {
        return m_value;
    }

    /**
     * @brief Set value and notify observers
     */
    void set(const T& newValue) {
        if (m_value != newValue) {
            m_value = newValue;
            notifyObservers();
        }
    }

    /**
     * @brief Set value without notifying observers
     */
    void setSilent(const T& newValue) {
        m_value = newValue;
    }

    /**
     * @brief Add observer that gets notified on value change
     */
    void addObserver(Observer observer) {
        m_observers.push_back(observer);
    }

    /**
     * @brief Clear all observers
     */
    void clearObservers() {
        m_observers.clear();
    }

    /**
     * @brief Manually notify all observers
     */
    void notifyObservers() {
        for (const auto& observer : m_observers) {
            observer(m_value);
        }
    }

    /**
     * @brief Implicit conversion to T for convenience
     */
    operator const T&() const {
        return m_value;
    }

    /**
     * @brief Assignment operator
     */
    Bindable<T>& operator=(const T& newValue) {
        set(newValue);
        return *this;
    }

private:
    T m_value;
    std::vector<Observer> m_observers;
};

/**
 * @brief Two-way data binding between model and UI
 *
 * Automatically synchronizes a value between the model layer and UI layer.
 * When the model changes, the UI updates. When the UI changes, the model updates.
 */
template<typename T>
class TwoWayBinding {
public:
    using Getter = std::function<T()>;
    using Setter = std::function<void(const T&)>;
    using UIUpdater = std::function<void(const T&)>;

    TwoWayBinding(Getter getter, Setter setter, UIUpdater uiUpdater)
        : m_getter(getter)
        , m_setter(setter)
        , m_uiUpdater(uiUpdater)
    {}

    /**
     * @brief Update UI from model
     */
    void updateUI() {
        if (m_getter && m_uiUpdater) {
            m_uiUpdater(m_getter());
        }
    }

    /**
     * @brief Update model from UI
     */
    void updateModel(const T& value) {
        if (m_setter) {
            m_setter(value);
        }
    }

private:
    Getter m_getter;
    Setter m_setter;
    UIUpdater m_uiUpdater;
};

/**
 * @brief Binding context for managing multiple bindings
 *
 * Manages a collection of bindings for a panel, allowing batch updates
 * and cleanup.
 */
class BindingContext {
public:
    /**
     * @brief Add a binding to the context
     */
    template<typename T>
    void addBinding(const std::string& name, Bindable<T>* bindable, typename Bindable<T>::Observer observer) {
        bindable->addObserver(observer);
        m_bindingNames.push_back(name);
    }

    /**
     * @brief Clear all bindings
     */
    void clear() {
        m_bindingNames.clear();
    }

    /**
     * @brief Get number of bindings
     */
    size_t count() const {
        return m_bindingNames.size();
    }

private:
    std::vector<std::string> m_bindingNames;
};

} // namespace NovaEditor
