#pragma once

#include "UIPanel.hpp"
#include "../Core/EditorState.hpp"
#include <NovaEngine/ECS/Scene.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <vector>

namespace NovaEditor {

/**
 * @brief Hierarchy panel for viewing all entities in the scene
 *
 * Displays a flat list of all entities in the current scene.
 * Users can click on entities to select them.
 */
class HierarchyPanel : public UIPanel {
public:
    HierarchyPanel(
        NovaEngine::UIManager& uiManager,
        EditorState* editorState,
        NovaEngine::Scene* scene
    )
        : UIPanel(uiManager, "hierarchy")
        , m_editorState(editorState)
        , m_scene(scene)
    {}

    void setScene(NovaEngine::Scene* scene) {
        m_scene = scene;
        update();
    }

    void update() override {
        if (!m_scene) {
            // No scene - show placeholder text, hide entity list
            setVisible("hierarchy_empty", true);
            setVisible("hierarchy_hint", true);
            setVisible("hierarchy_hint_2", true);
            setVisible("hierarchy_entity_count", false);
            hideAllEntityButtons();
            m_entityList.clear();
            return;
        }

        // Scene loaded - hide placeholder, show entity list
        setVisible("hierarchy_empty", false);
        setVisible("hierarchy_hint", false);
        setVisible("hierarchy_hint_2", false);
        setVisible("hierarchy_entity_count", true);

        // Get all entities from scene
        m_entityList.clear();
        auto& entities = m_scene->getEntities();
        for (auto* entity : entities) {
            if (entity) {
                m_entityList.push_back(entity);
            }
        }

        // Update entity count text
        setText("hierarchy_entity_count", "Entities: " + std::to_string(m_entityList.size()));

        // Update entity buttons (show up to 15 entities)
        const size_t maxVisible = 15;
        for (size_t i = 0; i < maxVisible; ++i) {
            std::string btnID = "btn_hierarchy_entity_" + std::to_string(i);

            if (i < m_entityList.size()) {
                // Show this entity button
                NovaEngine::Entity* entity = m_entityList[i];
                std::string entityName = getEntityDisplayName(entity);
                setText(btnID, entityName);
                setVisible(btnID, true);
            } else {
                // Hide unused button
                setVisible(btnID, false);
            }
        }

        LOG_DEBUG("Hierarchy panel updated with {} entities", m_entityList.size());
    }

    /**
     * @brief Get entity from the hierarchy list by index
     */
    NovaEngine::Entity* getEntityFromList(size_t index) const {
        if (index < m_entityList.size()) {
            return m_entityList[index];
        }
        return nullptr;
    }

private:
    EditorState* m_editorState;
    NovaEngine::Scene* m_scene;
    std::vector<NovaEngine::Entity*> m_entityList;

    /**
     * @brief Hide all entity buttons
     */
    void hideAllEntityButtons() {
        for (size_t i = 0; i < 15; ++i) {
            std::string btnID = "btn_hierarchy_entity_" + std::to_string(i);
            setVisible(btnID, false);
        }
    }

    /**
     * @brief Get display name for an entity
     */
    std::string getEntityDisplayName(NovaEngine::Entity* entity) const {
        if (!entity) {
            return "Unknown";
        }

        // Try to get a meaningful name from the entity's sprite component
        auto* sprite = entity->getComponent<NovaEngine::SpriteComponent>();
        if (sprite && !sprite->textureID.empty()) {
            // Use texture ID as entity name (e.g., "player_sprite", "enemy_01")
            return "Entity " + std::to_string(entity->getID()) + ": " + sprite->textureID;
        }

        // Fallback to just entity ID
        return "Entity " + std::to_string(entity->getID());
    }
};

} // namespace NovaEditor
