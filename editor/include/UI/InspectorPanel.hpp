#pragma once

#include "UIPanel.hpp"
#include "../Core/EditorState.hpp"
#include <NovaEngine/ECS/SceneManager.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <sstream>
#include <iomanip>

namespace NovaEditor {

/**
 * @brief Inspector panel for viewing entity properties inline
 *
 * Displays selected entity's properties in a clean, read-only format
 * on the right side of the screen.
 */
class InspectorPanel : public UIPanel {
public:
    InspectorPanel(
        NovaEngine::UIManager& uiManager,
        EditorState* editorState,
        NovaEngine::SceneManager* sceneManager
    )
        : UIPanel(uiManager, "inspector")
        , m_editorState(editorState)
        , m_sceneManager(sceneManager)
    {}

    void update() override {
        using namespace NovaEngine;

        Entity* selectedEntity = m_editorState->getSelectedEntity();

        if (!selectedEntity) {
            // No entity selected - show placeholder text, hide property fields
            setComponentVisible("inspector_no_selection", true);
            setComponentVisible("inspector_hint", true);
            hidePropertyFields();
            return;
        }

        // Entity is selected - hide placeholder, show properties
        setComponentVisible("inspector_no_selection", false);
        setComponentVisible("inspector_hint", false);
        showPropertyFields();

        // Get components
        auto* transform = selectedEntity->getComponent<TransformComponent>();
        auto* sprite = selectedEntity->getComponent<SpriteComponent>();

        if (!transform) {
            return;
        }

        // Update property values
        setText("inspector_value_id", std::to_string(selectedEntity->getID()));
        setText("inspector_value_pos_x", formatInt(static_cast<int>(transform->position.x)));
        setText("inspector_value_pos_y", formatInt(static_cast<int>(transform->position.y)));
        setText("inspector_value_scale_x", formatFloat(transform->scale.x, 2));
        setText("inspector_value_scale_y", formatFloat(transform->scale.y, 2));
        setText("inspector_value_rotation", formatFloat(transform->rotation, 1) + "°");

        if (sprite) {
            setText("inspector_value_layer", std::to_string(sprite->zOrder));
            setText("inspector_value_texture", sprite->textureID.empty() ? "None" : sprite->textureID);
        } else {
            setText("inspector_value_layer", "N/A");
            setText("inspector_value_texture", "None");
        }

        LOG_DEBUG("Inspector panel updated for entity ID {}", selectedEntity->getID());
    }

private:
    EditorState* m_editorState;
    NovaEngine::SceneManager* m_sceneManager;

    /**
     * @brief Set component visibility
     */
    void setComponentVisible(const std::string& componentID, bool visible) {
        auto comp = m_uiManager.getComponent(componentID);
        if (comp) {
            comp->setVisible(visible);
            comp->setActive(visible);
        }
    }

    /**
     * @brief Show all property field elements
     */
    void showPropertyFields() {
        // Section headers
        setComponentVisible("inspector_section_basic", true);
        setComponentVisible("inspector_section_transform", true);
        setComponentVisible("inspector_section_rendering", true);

        // ID field
        setComponentVisible("inspector_label_id", true);
        setComponentVisible("inspector_value_id", true);

        // Transform fields
        setComponentVisible("inspector_label_pos_x", true);
        setComponentVisible("inspector_value_pos_x", true);
        setComponentVisible("inspector_label_pos_y", true);
        setComponentVisible("inspector_value_pos_y", true);
        setComponentVisible("inspector_label_scale_x", true);
        setComponentVisible("inspector_value_scale_x", true);
        setComponentVisible("inspector_label_scale_y", true);
        setComponentVisible("inspector_value_scale_y", true);
        setComponentVisible("inspector_label_rotation", true);
        setComponentVisible("inspector_value_rotation", true);

        // Rendering fields
        setComponentVisible("inspector_label_layer", true);
        setComponentVisible("inspector_value_layer", true);
        setComponentVisible("inspector_label_texture", true);
        setComponentVisible("inspector_value_texture", true);
    }

    /**
     * @brief Hide all property field elements
     */
    void hidePropertyFields() {
        // Section headers
        setComponentVisible("inspector_section_basic", false);
        setComponentVisible("inspector_section_transform", false);
        setComponentVisible("inspector_section_rendering", false);

        // ID field
        setComponentVisible("inspector_label_id", false);
        setComponentVisible("inspector_value_id", false);

        // Transform fields
        setComponentVisible("inspector_label_pos_x", false);
        setComponentVisible("inspector_value_pos_x", false);
        setComponentVisible("inspector_label_pos_y", false);
        setComponentVisible("inspector_value_pos_y", false);
        setComponentVisible("inspector_label_scale_x", false);
        setComponentVisible("inspector_value_scale_x", false);
        setComponentVisible("inspector_label_scale_y", false);
        setComponentVisible("inspector_value_scale_y", false);
        setComponentVisible("inspector_label_rotation", false);
        setComponentVisible("inspector_value_rotation", false);

        // Rendering fields
        setComponentVisible("inspector_label_layer", false);
        setComponentVisible("inspector_value_layer", false);
        setComponentVisible("inspector_label_texture", false);
        setComponentVisible("inspector_value_texture", false);
    }

    /**
     * @brief Format integer value as string
     */
    std::string formatInt(int value) const {
        return std::to_string(value);
    }

    /**
     * @brief Format float value with specified precision
     */
    std::string formatFloat(float value, int precision) const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }
};

} // namespace NovaEditor
