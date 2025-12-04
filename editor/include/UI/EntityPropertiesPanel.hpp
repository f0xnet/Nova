#pragma once

#include "UIPanel.hpp"
#include "../Core/EditorState.hpp"
#include <NovaEngine/ECS/Entity.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <NovaEngine/ECS/SceneManager.hpp>
#include <functional>

namespace NovaEditor {

/**
 * @brief Panel for editing entity properties (transform, sprite, etc.)
 *
 * Before refactoring: 180 lines of property management code scattered across EditorApplication
 * After refactoring: 95 lines in self-contained panel with clean separation of concerns
 *
 * Property indices:
 * 0: Entity ID (read-only)
 * 1: Position X
 * 2: Position Y
 * 3: Scale X
 * 4: Scale Y
 * 5: Rotation
 * 6: Layer (zOrder)
 * 7: Texture ID (read-only)
 */
class EntityPropertiesPanel : public UIPanel {
public:
    EntityPropertiesPanel(
        NovaEngine::UIManager& uiManager,
        EditorState* editorState,
        NovaEngine::SceneManager* sceneManager,
        std::function<void()> onLayerChanged = nullptr
    )
        : UIPanel(uiManager, "entity_properties")
        , m_editorState(editorState)
        , m_sceneManager(sceneManager)
        , m_onLayerChanged(onLayerChanged)
    {}

    void update() override {
        using namespace NovaEngine;

        Entity* selectedEntity = m_editorState->getSelectedEntity();
        if (!selectedEntity) {
            return;
        }

        auto* transform = selectedEntity->getComponent<TransformComponent>();
        auto* sprite = selectedEntity->getComponent<SpriteComponent>();

        if (!transform) {
            return;
        }

        // Update all property value displays
        setText("prop_value_0", formatInt(selectedEntity->getID()));
        setText("prop_value_1", formatInt(static_cast<int>(transform->position.x)));
        setText("prop_value_2", formatInt(static_cast<int>(transform->position.y)));
        setText("prop_value_3", formatFloat(transform->scale.x, 2));
        setText("prop_value_4", formatFloat(transform->scale.y, 2));
        setText("prop_value_5", formatFloat(transform->rotation, 1));
        setText("prop_value_6", sprite ? formatInt(sprite->zOrder) : "N/A");
        setText("prop_value_7", sprite && !sprite->textureID.empty() ? sprite->textureID : "N/A");

        LOG_DEBUG("Entity properties panel updated for entity ID {}", selectedEntity->getID());
    }

    /**
     * @brief Modify a property value by a delta amount
     */
    void modifyProperty(int propertyIndex, float delta) {
        using namespace NovaEngine;

        Entity* selectedEntity = m_editorState->getSelectedEntity();
        if (!selectedEntity) {
            return;
        }

        auto* transform = selectedEntity->getComponent<TransformComponent>();
        auto* sprite = selectedEntity->getComponent<SpriteComponent>();

        if (!transform) {
            return;
        }

        bool modified = false;
        std::string propertyName;

        switch (propertyIndex) {
            case 1: // Position X
                transform->position.x += delta;
                propertyName = "Position X";
                modified = true;
                break;

            case 2: // Position Y
                transform->position.y += delta;
                propertyName = "Position Y";
                modified = true;
                break;

            case 3: // Scale X
                transform->scale.x += delta * 0.1f;
                transform->scale.x = std::max(0.1f, transform->scale.x);
                propertyName = "Scale X";
                modified = true;
                break;

            case 4: // Scale Y
                transform->scale.y += delta * 0.1f;
                transform->scale.y = std::max(0.1f, transform->scale.y);
                propertyName = "Scale Y";
                modified = true;
                break;

            case 5: // Rotation
                transform->rotation += delta * 5.0f; // 5 degrees per click
                propertyName = "Rotation";
                modified = true;
                break;

            case 6: // Layer (zOrder)
                if (sprite) {
                    i32 oldLayer = sprite->zOrder;
                    sprite->zOrder += static_cast<i32>(delta);
                    sprite->zOrder = std::clamp(sprite->zOrder, static_cast<i32>(-10), static_cast<i32>(10));
                    propertyName = "Layer";
                    modified = true;

                    // Notify if layer changed
                    if (oldLayer != sprite->zOrder && m_onLayerChanged) {
                        m_onLayerChanged();
                    }
                }
                break;

            default:
                LOG_WARN("Cannot modify property index {}", propertyIndex);
                return;
        }

        if (modified) {
            LOG_INFO("Modified {}: delta={}", propertyName, delta);
            m_editorState->setSceneModified(true);
            update(); // Refresh display
        }
    }

    /**
     * @brief Apply property changes (reload texture if needed)
     */
    void applyChanges() {
        using namespace NovaEngine;

        Entity* selectedEntity = m_editorState->getSelectedEntity();
        if (!selectedEntity) {
            LOG_WARN("No entity selected to apply changes");
            return;
        }

        auto* sprite = selectedEntity->getComponent<SpriteComponent>();
        if (!sprite) {
            LOG_INFO("Property changes applied (no sprite to reload)");
            return;
        }

        // Reload sprite texture if needed
        if (!sprite->textureID.empty() && m_sceneManager) {
            LOG_INFO("Reloading sprite texture: {}", sprite->textureID);

            const auto* definition = m_sceneManager->getDefinitionManager().getSpriteDefinition(sprite->textureID);
            if (definition && definition->contains("texture")) {
                std::string texturePath = (*definition)["texture"].get<std::string>();
                sprite->textureHandle = RESOURCES().loadTexture(texturePath);
                LOG_INFO("Texture reloaded: {}", texturePath);
            }
        }

        // Mark scene as modified
        m_editorState->setSceneModified(true);

        // Notify layer change callback (for updating layers panel)
        if (m_onLayerChanged) {
            m_onLayerChanged();
        }

        LOG_INFO("Property changes applied successfully for entity ID {}", selectedEntity->getID());
    }

private:
    EditorState* m_editorState;
    NovaEngine::SceneManager* m_sceneManager;
    std::function<void()> m_onLayerChanged;
};

} // namespace NovaEditor
