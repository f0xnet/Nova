#pragma once

#include "ComponentRegistry.hpp"
#include <NovaEngine/ECS/Components.hpp>

namespace NovaEditor {

/**
 * @brief Initialize all NovaEngine components in the registry
 *
 * HOW TO ADD A NEW COMPONENT TO THE ENGINE:
 *
 * 1. Add component class to NovaEngine (e.g., PhysicsComponent)
 * 2. Add registration function here:
 *    ```cpp
 *    inline void registerPhysicsComponent() {
 *        auto& registry = ComponentRegistry::instance();
 *        registry.registerComponent<NovaEngine::PhysicsComponent>(
 *            "Physics",
 *            "Physics Body",
 *            "Physics",
 *            [](auto* comp) {
 *                ComponentMetadata meta;
 *                // Define properties...
 *                return meta;
 *            }
 *        );
 *    }
 *    ```
 * 3. Call it in initializeEngineComponents()
 * 4. Done! Editor automatically supports the new component
 *
 * NO NEED TO:
 * - Modify EntityPropertiesPanel
 * - Modify EntityEditorController
 * - Add hardcoded UI logic
 * - Update any other editor code
 */

inline void registerTransformComponent() {
    auto& registry = ComponentRegistry::instance();

    registry.registerComponent<NovaEngine::TransformComponent>(
        "Transform",
        "Transform",
        "Core",
        [](NovaEngine::TransformComponent* comp) {
            ComponentMetadata meta;

            // Position X
            PropertyMetadata posX;
            posX.name = "Position X";
            posX.type = "float";
            posX.getValue = [comp]() { return std::to_string(comp->position.x); };
            posX.setValue = [comp](const std::string& v) { comp->position.x = std::stof(v); };
            meta.properties.push_back(posX);

            // Position Y
            PropertyMetadata posY;
            posY.name = "Position Y";
            posY.type = "float";
            posY.getValue = [comp]() { return std::to_string(comp->position.y); };
            posY.setValue = [comp](const std::string& v) { comp->position.y = std::stof(v); };
            meta.properties.push_back(posY);

            // Scale X
            PropertyMetadata scaleX;
            scaleX.name = "Scale X";
            scaleX.type = "float";
            scaleX.minValue = 0.1f;
            scaleX.maxValue = 10.0f;
            scaleX.getValue = [comp]() { return std::to_string(comp->scale.x); };
            scaleX.setValue = [comp](const std::string& v) { comp->scale.x = std::stof(v); };
            meta.properties.push_back(scaleX);

            // Scale Y
            PropertyMetadata scaleY;
            scaleY.name = "Scale Y";
            scaleY.type = "float";
            scaleY.minValue = 0.1f;
            scaleY.maxValue = 10.0f;
            scaleY.getValue = [comp]() { return std::to_string(comp->scale.y); };
            scaleY.setValue = [comp](const std::string& v) { comp->scale.y = std::stof(v); };
            meta.properties.push_back(scaleY);

            // Rotation
            PropertyMetadata rotation;
            rotation.name = "Rotation";
            rotation.type = "float";
            rotation.minValue = 0.0f;
            rotation.maxValue = 360.0f;
            rotation.getValue = [comp]() { return std::to_string(comp->rotation); };
            rotation.setValue = [comp](const std::string& v) { comp->rotation = std::stof(v); };
            meta.properties.push_back(rotation);

            return meta;
        }
    );
}

inline void registerSpriteComponent() {
    auto& registry = ComponentRegistry::instance();

    registry.registerComponent<NovaEngine::SpriteComponent>(
        "Sprite",
        "Sprite Renderer",
        "Rendering",
        [](NovaEngine::SpriteComponent* comp) {
            ComponentMetadata meta;

            // Layer (zOrder)
            PropertyMetadata zOrder;
            zOrder.name = "Layer";
            zOrder.type = "int";
            zOrder.minValue = -10.0f;
            zOrder.maxValue = 10.0f;
            zOrder.getValue = [comp]() { return std::to_string(comp->zOrder); };
            zOrder.setValue = [comp](const std::string& v) { comp->zOrder = std::stoi(v); };
            meta.properties.push_back(zOrder);

            // Visible
            PropertyMetadata visible;
            visible.name = "Visible";
            visible.type = "bool";
            visible.getValue = [comp]() { return comp->visible ? "true" : "false"; };
            visible.setValue = [comp](const std::string& v) { comp->visible = (v == "true"); };
            meta.properties.push_back(visible);

            // Texture ID
            PropertyMetadata textureID;
            textureID.name = "Texture ID";
            textureID.type = "string";
            textureID.getValue = [comp]() { return comp->textureID; };
            textureID.setValue = [comp](const std::string& v) { comp->textureID = v; };
            meta.properties.push_back(textureID);

            return meta;
        }
    );
}

inline void registerLightComponent() {
    auto& registry = ComponentRegistry::instance();

    registry.registerComponent<NovaEngine::LightComponent>(
        "Light",
        "Light Source",
        "Rendering",
        [](NovaEngine::LightComponent* comp) {
            ComponentMetadata meta;

            // Radius
            PropertyMetadata radius;
            radius.name = "Radius";
            radius.type = "float";
            radius.minValue = 10.0f;
            radius.maxValue = 1000.0f;
            radius.getValue = [comp]() { return std::to_string(comp->radius); };
            radius.setValue = [comp](const std::string& v) { comp->radius = std::stof(v); };
            meta.properties.push_back(radius);

            // Intensity
            PropertyMetadata intensity;
            intensity.name = "Intensity";
            intensity.type = "float";
            intensity.minValue = 0.0f;
            intensity.maxValue = 2.0f;
            intensity.step = 0.1f;
            intensity.getValue = [comp]() { return std::to_string(comp->intensity); };
            intensity.setValue = [comp](const std::string& v) { comp->intensity = std::stof(v); };
            meta.properties.push_back(intensity);

            // Enabled
            PropertyMetadata enabled;
            enabled.name = "Enabled";
            enabled.type = "bool";
            enabled.getValue = [comp]() { return comp->enabled ? "true" : "false"; };
            enabled.setValue = [comp](const std::string& v) { comp->enabled = (v == "true"); };
            meta.properties.push_back(enabled);

            return meta;
        }
    );
}

/**
 * @brief Initialize all engine components - call this at editor startup
 */
inline void initializeEngineComponents() {
    registerTransformComponent();
    registerSpriteComponent();
    registerLightComponent();

    // TODO: Add more components as they are added to the engine
    // registerPhysicsComponent();
    // registerAudioComponent();
    // registerAnimationComponent();
    // etc.
}

} // namespace NovaEditor
