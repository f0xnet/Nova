#pragma once

#include "Component.hpp"
#include "../Backend/Core/BackendTypes.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace NovaEngine {

/**
 * @brief Entity - a container for components
 *
 * An entity is simply an ID with a collection of components.
 * Systems operate on entities that have specific components.
 */
class Entity {
private:
    u64 m_id;
    std::unordered_map<ComponentTypeID, std::unique_ptr<Component>> m_components;

public:
    /**
     * @brief Constructor
     * @param id Unique entity ID
     */
    explicit Entity(u64 id) : m_id(id) {}

    /**
     * @brief Get the entity ID
     * @return The unique entity ID
     */
    u64 getID() const { return m_id; }

    /**
     * @brief Add a component to this entity
     * @tparam T The component type (must derive from Component)
     * @param component The component to add (ownership is transferred)
     * @return Pointer to the added component
     */
    template<typename T>
    T* addComponent(std::unique_ptr<T> component) {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        ComponentTypeID typeID = component->getTypeID();
        T* ptr = component.get();
        m_components[typeID] = std::move(component);
        return ptr;
    }

    /**
     * @brief Get a component by type
     * @tparam T The component type
     * @return Pointer to the component, or nullptr if not found
     */
    template<typename T>
    T* getComponent() {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        T temp_instance;
        ComponentTypeID typeID = temp_instance.getTypeID();

        auto it = m_components.find(typeID);
        if (it != m_components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    /**
     * @brief Get a component by type (const version)
     * @tparam T The component type
     * @return Const pointer to the component, or nullptr if not found
     */
    template<typename T>
    const T* getComponent() const {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        T temp_instance;
        ComponentTypeID typeID = temp_instance.getTypeID();

        auto it = m_components.find(typeID);
        if (it != m_components.end()) {
            return static_cast<const T*>(it->second.get());
        }
        return nullptr;
    }

    /**
     * @brief Check if this entity has a specific component type
     * @tparam T The component type
     * @return true if the component exists
     */
    template<typename T>
    bool hasComponent() const {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        T temp_instance;
        ComponentTypeID typeID = temp_instance.getTypeID();
        return m_components.find(typeID) != m_components.end();
    }

    /**
     * @brief Check if this entity has a specific component type (by type ID string)
     * @param typeID The component type ID
     * @return true if the component exists
     */
    bool hasComponent(const ComponentTypeID& typeID) const {
        return m_components.find(typeID) != m_components.end();
    }

    /**
     * @brief Remove a component from this entity
     * @tparam T The component type
     */
    template<typename T>
    void removeComponent() {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");

        T temp_instance;
        ComponentTypeID typeID = temp_instance.getTypeID();
        m_components.erase(typeID);
    }

    /**
     * @brief Get all component type IDs on this entity
     * @return Vector of component type IDs
     */
    std::vector<ComponentTypeID> getComponentTypes() const {
        std::vector<ComponentTypeID> types;
        types.reserve(m_components.size());
        for (const auto& pair : m_components) {
            types.push_back(pair.first);
        }
        return types;
    }
};

} // namespace NovaEngine
