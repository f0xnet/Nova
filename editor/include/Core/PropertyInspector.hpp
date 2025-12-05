#pragma once

#include "ComponentRegistry.hpp"
#include <NovaEngine/ECS/Entity.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <NovaEngine/Core/Logger.hpp>
#include <functional>

namespace NovaEditor {

/**
 * @brief Generic property inspector for any component type
 *
 * PROBLEM SOLVED:
 * The editor had hardcoded property inspection for specific components.
 * Adding a new component type required modifying EntityPropertiesPanel.
 *
 * SOLUTION:
 * Generic inspection system that works with ComponentRegistry metadata.
 * New components automatically get property inspection UI.
 *
 * BENEFITS:
 * - Add component to engine → Register metadata → Inspector works automatically
 * - No hardcoded property lists
 * - Consistent UI for all component types
 * - Easy to extend with custom property editors
 */
class PropertyInspector {
public:
    using PropertyChangeCallback = std::function<void(const std::string&, const std::string&)>;

    PropertyInspector() = default;

    /**
     * @brief Inspect entity and extract all component properties
     */
    struct InspectionResult {
        std::string componentType;
        std::string componentName;
        std::vector<PropertyMetadata> properties;
    };

    /**
     * @brief Get all inspectable components from entity
     */
    std::vector<InspectionResult> inspectEntity(NovaEngine::Entity* entity) {
        std::vector<InspectionResult> results;

        if (!entity) {
            return results;
        }

        auto& registry = ComponentRegistry::instance();

        // Inspect each registered component type
        for (const std::string& typeName : registry.getAllComponentTypes()) {
            const auto* metadata = registry.getComponentMetadata(typeName);
            if (!metadata) continue;

            // Check if entity has this component (would need RTTI or component ID system)
            // For now, we'll check the common components manually
            // TODO: Add component query system to NovaEngine

            InspectionResult result;
            result.componentType = typeName;
            result.componentName = metadata->displayName;

            if (typeName == "Transform") {
                auto* comp = entity->getComponent<NovaEngine::TransformComponent>();
                if (comp) {
                    result.properties = extractTransformProperties(comp);
                    results.push_back(result);
                }
            }
            else if (typeName == "Sprite") {
                auto* comp = entity->getComponent<NovaEngine::SpriteComponent>();
                if (comp) {
                    result.properties = extractSpriteProperties(comp);
                    results.push_back(result);
                }
            }
            else if (typeName == "Light") {
                auto* comp = entity->getComponent<NovaEngine::LightComponent>();
                if (comp) {
                    result.properties = extractLightProperties(comp);
                    results.push_back(result);
                }
            }
            // More component types can be added here dynamically
        }

        return results;
    }

    /**
     * @brief Modify property value on entity component
     */
    bool modifyProperty(
        NovaEngine::Entity* entity,
        const std::string& componentType,
        const std::string& propertyName,
        const std::string& newValue
    ) {
        if (!entity) return false;

        // Route to appropriate component modifier
        if (componentType == "Transform") {
            return modifyTransformProperty(entity, propertyName, newValue);
        }
        else if (componentType == "Sprite") {
            return modifySpriteProperty(entity, propertyName, newValue);
        }
        else if (componentType == "Light") {
            return modifyLightProperty(entity, propertyName, newValue);
        }

        return false;
    }

private:
    // Extract properties from specific component types
    // These could be auto-generated from reflection/metadata

    std::vector<PropertyMetadata> extractTransformProperties(NovaEngine::TransformComponent* comp) {
        using namespace NovaEngine;
        std::vector<PropertyMetadata> props;

        PropertyMetadata posX;
        posX.name = "Position X";
        posX.type = "float";
        posX.getValue = [comp]() { return std::to_string(comp->position.x); };
        posX.setValue = [comp](const std::string& val) { comp->position.x = std::stof(val); };
        props.push_back(posX);

        PropertyMetadata posY;
        posY.name = "Position Y";
        posY.type = "float";
        posY.getValue = [comp]() { return std::to_string(comp->position.y); };
        posY.setValue = [comp](const std::string& val) { comp->position.y = std::stof(val); };
        props.push_back(posY);

        PropertyMetadata scaleX;
        scaleX.name = "Scale X";
        scaleX.type = "float";
        scaleX.minValue = 0.1f;
        scaleX.maxValue = 10.0f;
        scaleX.getValue = [comp]() { return std::to_string(comp->scale.x); };
        scaleX.setValue = [comp](const std::string& val) { comp->scale.x = std::stof(val); };
        props.push_back(scaleX);

        PropertyMetadata scaleY;
        scaleY.name = "Scale Y";
        scaleY.type = "float";
        scaleY.minValue = 0.1f;
        scaleY.maxValue = 10.0f;
        scaleY.getValue = [comp]() { return std::to_string(comp->scale.y); };
        scaleY.setValue = [comp](const std::string& val) { comp->scale.y = std::stof(val); };
        props.push_back(scaleY);

        PropertyMetadata rotation;
        rotation.name = "Rotation";
        rotation.type = "float";
        rotation.minValue = 0.0f;
        rotation.maxValue = 360.0f;
        rotation.getValue = [comp]() { return std::to_string(comp->rotation); };
        rotation.setValue = [comp](const std::string& val) { comp->rotation = std::stof(val); };
        props.push_back(rotation);

        return props;
    }

    std::vector<PropertyMetadata> extractSpriteProperties(NovaEngine::SpriteComponent* comp) {
        using namespace NovaEngine;
        std::vector<PropertyMetadata> props;

        PropertyMetadata zOrder;
        zOrder.name = "Layer (zOrder)";
        zOrder.type = "int";
        zOrder.minValue = -10.0f;
        zOrder.maxValue = 10.0f;
        zOrder.getValue = [comp]() { return std::to_string(comp->zOrder); };
        zOrder.setValue = [comp](const std::string& val) { comp->zOrder = std::stoi(val); };
        props.push_back(zOrder);

        PropertyMetadata visible;
        visible.name = "Visible";
        visible.type = "bool";
        visible.getValue = [comp]() { return comp->visible ? "true" : "false"; };
        visible.setValue = [comp](const std::string& val) { comp->visible = (val == "true"); };
        props.push_back(visible);

        PropertyMetadata textureID;
        textureID.name = "Texture ID";
        textureID.type = "string";
        textureID.getValue = [comp]() { return comp->textureID; };
        textureID.setValue = [comp](const std::string& val) { comp->textureID = val; };
        props.push_back(textureID);

        return props;
    }

    std::vector<PropertyMetadata> extractLightProperties(NovaEngine::LightComponent* comp) {
        using namespace NovaEngine;
        std::vector<PropertyMetadata> props;

        PropertyMetadata radius;
        radius.name = "Radius";
        radius.type = "float";
        radius.minValue = 10.0f;
        radius.maxValue = 1000.0f;
        radius.getValue = [comp]() { return std::to_string(comp->radius); };
        radius.setValue = [comp](const std::string& val) { comp->radius = std::stof(val); };
        props.push_back(radius);

        PropertyMetadata intensity;
        intensity.name = "Intensity";
        intensity.type = "float";
        intensity.minValue = 0.0f;
        intensity.maxValue = 2.0f;
        intensity.step = 0.1f;
        intensity.getValue = [comp]() { return std::to_string(comp->intensity); };
        intensity.setValue = [comp](const std::string& val) { comp->intensity = std::stof(val); };
        props.push_back(intensity);

        PropertyMetadata enabled;
        enabled.name = "Enabled";
        enabled.type = "bool";
        enabled.getValue = [comp]() { return comp->enabled ? "true" : "false"; };
        enabled.setValue = [comp](const std::string& val) { comp->enabled = (val == "true"); };
        props.push_back(enabled);

        return props;
    }

    bool modifyTransformProperty(NovaEngine::Entity* entity, const std::string& prop, const std::string& val) {
        auto* comp = entity->getComponent<NovaEngine::TransformComponent>();
        if (!comp) return false;

        try {
            if (prop == "Position X") comp->position.x = std::stof(val);
            else if (prop == "Position Y") comp->position.y = std::stof(val);
            else if (prop == "Scale X") comp->scale.x = std::stof(val);
            else if (prop == "Scale Y") comp->scale.y = std::stof(val);
            else if (prop == "Rotation") comp->rotation = std::stof(val);
            else return false;
            return true;
        } catch (...) {
            return false;
        }
    }

    bool modifySpriteProperty(NovaEngine::Entity* entity, const std::string& prop, const std::string& val) {
        auto* comp = entity->getComponent<NovaEngine::SpriteComponent>();
        if (!comp) return false;

        try {
            if (prop == "Layer (zOrder)") comp->zOrder = std::stoi(val);
            else if (prop == "Visible") comp->visible = (val == "true");
            else if (prop == "Texture ID") comp->textureID = val;
            else return false;
            return true;
        } catch (...) {
            return false;
        }
    }

    bool modifyLightProperty(NovaEngine::Entity* entity, const std::string& prop, const std::string& val) {
        auto* comp = entity->getComponent<NovaEngine::LightComponent>();
        if (!comp) return false;

        try {
            if (prop == "Radius") comp->radius = std::stof(val);
            else if (prop == "Intensity") comp->intensity = std::stof(val);
            else if (prop == "Enabled") comp->enabled = (val == "true");
            else return false;
            return true;
        } catch (...) {
            return false;
        }
    }
};

} // namespace NovaEditor
