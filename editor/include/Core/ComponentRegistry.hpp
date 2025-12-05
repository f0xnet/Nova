#pragma once

#include <NovaEngine/ECS/Component.hpp>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <typeindex>

namespace NovaEditor {

/**
 * @brief Property metadata for component inspection
 */
struct PropertyMetadata {
    std::string name;
    std::string type; // "float", "int", "vec2", "color", "bool", "string"
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float step = 0.1f;

    // Getter/Setter callbacks
    std::function<std::string()> getValue;
    std::function<void(const std::string&)> setValue;
};

/**
 * @brief Component metadata for registration
 */
struct ComponentMetadata {
    std::string typeName;
    std::string displayName;
    std::string category; // "Transform", "Rendering", "Physics", "Logic", etc.
    std::string icon;

    std::vector<PropertyMetadata> properties;

    // Factory function to create component
    std::function<std::unique_ptr<NovaEngine::Component>()> createComponent;
};

/**
 * @brief Dynamic component registry for engine-editor integration
 *
 * PROBLEM SOLVED:
 * When NovaEngine adds a new component type, the editor needed to be
 * manually updated with hardcoded logic. This registry allows components
 * to be registered dynamically, making the editor automatically compatible
 * with engine changes.
 *
 * BENEFITS:
 * - Add component to engine → Register it → Editor supports it immediately
 * - No hardcoded component types in editor
 * - Generic property inspection
 * - Automatic UI generation for new components
 *
 * USAGE:
 * @code
 * // In editor initialization:
 * registry.registerComponent<TransformComponent>(
 *     "Transform", "Transform",
 *     [](auto* comp) { return metadata; }
 * );
 *
 * // Editor can now discover and edit any registered component
 * auto metadata = registry.getComponentMetadata("Transform");
 * @endcode
 */
class ComponentRegistry {
public:
    static ComponentRegistry& instance() {
        static ComponentRegistry instance;
        return instance;
    }

    /**
     * @brief Register a component type with metadata
     */
    template<typename T>
    void registerComponent(
        const std::string& typeName,
        const std::string& displayName,
        const std::string& category,
        std::function<ComponentMetadata(T*)> metadataBuilder
    ) {
        static_assert(std::is_base_of<NovaEngine::Component, T>::value,
                     "T must derive from NovaEngine::Component");

        ComponentMetadata metadata;
        metadata.typeName = typeName;
        metadata.displayName = displayName;
        metadata.category = category;

        // Create a dummy instance to extract metadata
        auto dummy = std::make_unique<T>();
        metadata = metadataBuilder(dummy.get());
        metadata.typeName = typeName;
        metadata.displayName = displayName;
        metadata.category = category;

        // Factory function
        metadata.createComponent = []() -> std::unique_ptr<NovaEngine::Component> {
            return std::make_unique<T>();
        };

        m_components[typeName] = metadata;
        m_componentsByType[std::type_index(typeid(T))] = typeName;

        // Add to category list
        m_categories[category].push_back(typeName);
    }

    /**
     * @brief Get component metadata by type name
     */
    const ComponentMetadata* getComponentMetadata(const std::string& typeName) const {
        auto it = m_components.find(typeName);
        return (it != m_components.end()) ? &it->second : nullptr;
    }

    /**
     * @brief Get all registered component types
     */
    std::vector<std::string> getAllComponentTypes() const {
        std::vector<std::string> types;
        for (const auto& [name, _] : m_components) {
            types.push_back(name);
        }
        return types;
    }

    /**
     * @brief Get components by category
     */
    std::vector<std::string> getComponentsByCategory(const std::string& category) const {
        auto it = m_categories.find(category);
        return (it != m_categories.end()) ? it->second : std::vector<std::string>();
    }

    /**
     * @brief Get all categories
     */
    std::vector<std::string> getAllCategories() const {
        std::vector<std::string> cats;
        for (const auto& [cat, _] : m_categories) {
            cats.push_back(cat);
        }
        return cats;
    }

    /**
     * @brief Create component by type name
     */
    std::unique_ptr<NovaEngine::Component> createComponent(const std::string& typeName) const {
        auto it = m_components.find(typeName);
        if (it != m_components.end() && it->second.createComponent) {
            return it->second.createComponent();
        }
        return nullptr;
    }

    /**
     * @brief Get type name from type_index
     */
    std::string getTypeName(std::type_index typeIdx) const {
        auto it = m_componentsByType.find(typeIdx);
        return (it != m_componentsByType.end()) ? it->second : "";
    }

private:
    ComponentRegistry() = default;

    std::unordered_map<std::string, ComponentMetadata> m_components;
    std::unordered_map<std::type_index, std::string> m_componentsByType;
    std::unordered_map<std::string, std::vector<std::string>> m_categories;
};

} // namespace NovaEditor
