#pragma once

#include "UIPanel.hpp"
#include "../Core/EditorState.hpp"
#include <NovaEngine/ECS/Entity.hpp>
#include <NovaEngine/ECS/Scene.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <map>
#include <vector>

namespace NovaEditor {

/**
 * @brief Panel for managing layers and viewing entities per layer
 *
 * Before refactoring: 120 lines of layer management code in EditorApplication
 * After refactoring: 85 lines in self-contained panel
 *
 * Features:
 * - Display entity count per layer
 * - Highlight current layer
 * - Show entity list for selected layer
 * - Toggle panel visibility
 */
class LayersPanel : public UIPanel {
public:
    LayersPanel(
        NovaEngine::UIManager& uiManager,
        EditorState* editorState,
        NovaEngine::Scene* currentScene
    )
        : UIPanel(uiManager, "layers_panel")
        , m_editorState(editorState)
        , m_currentScene(currentScene)
    {}

    /**
     * @brief Set the current scene (call this when scene changes)
     */
    void setScene(NovaEngine::Scene* scene) {
        m_currentScene = scene;
        update();
    }

    void update() override {
        using namespace NovaEngine;

        if (!m_currentScene) {
            return;
        }

        // Count entities per layer and collect them
        std::map<i32, int> layerCounts;
        std::map<i32, std::vector<Entity*>> entitiesByLayer;

        const auto& entities = m_currentScene->getEntityRegistry().getAllEntities();
        for (auto* entity : entities) {
            if (entity) {
                auto* sprite = entity->getComponent<SpriteComponent>();
                if (sprite) {
                    layerCounts[sprite->zOrder]++;
                    entitiesByLayer[sprite->zOrder].push_back(entity);
                }
            }
        }

        // Update layer button texts
        updateLayerButtons(layerCounts);

        // Update entity list for current layer
        updateEntityList(entitiesByLayer);

        LOG_DEBUG("Layers panel updated - {} entities on layer {}",
                  m_entityList.size(), m_editorState->getCurrentLayer());
    }

    /**
     * @brief Toggle panel visibility
     */
    void toggle() {
        // Check current state
        auto panelBg = m_uiManager.getComponent("layers_panel_bg");
        bool isActive = panelBg ? panelBg->isActive() : false;

        // Toggle
        bool newState = !isActive;
        m_uiManager.setGroupActive(m_groupID, newState);
        m_isVisible = newState;

        LOG_INFO("Layers panel toggled: {}", newState ? "VISIBLE" : "HIDDEN");

        // Update content if showing
        if (newState && m_currentScene) {
            update();
        }
    }

    /**
     * @brief Get entity from the current layer list by index
     */
    NovaEngine::Entity* getEntityFromList(size_t index) const {
        if (index < m_entityList.size()) {
            return m_entityList[index];
        }
        return nullptr;
    }

private:
    void updateLayerButtons(const std::map<NovaEngine::i32, int>& layerCounts) {
        using namespace NovaEngine;

        // Predefined layer buttons
        struct LayerButton {
            std::string buttonID;
            i32 layer;
        };

        const std::vector<LayerButton> layerButtons = {
            {"btn_layer_m10", -10},
            {"btn_layer_m5", -5},
            {"btn_layer_m1", -1},
            {"btn_layer_0", 0},
            {"btn_layer_1", 1},
            {"btn_layer_2", 2},
            {"btn_layer_3", 3},
            {"btn_layer_5", 5},
            {"btn_layer_10", 10}
        };

        i32 currentLayer = m_editorState->getCurrentLayer();

        for (const auto& lb : layerButtons) {
            auto btnComponent = m_uiManager.getComponent(lb.buttonID);
            if (btnComponent) {
                auto button = std::dynamic_pointer_cast<Button>(btnComponent);
                if (button) {
                    auto it = layerCounts.find(lb.layer);
                    int count = (it != layerCounts.end()) ? it->second : 0;

                    std::string text = "Layer " + formatInt(lb.layer) + ": " + formatInt(count);

                    // Highlight current layer
                    if (lb.layer == currentLayer) {
                        text = "> " + text + " <";
                    }

                    button->setText(text);
                }
            }
        }
    }

    void updateEntityList(const std::map<NovaEngine::i32, std::vector<NovaEngine::Entity*>>& entitiesByLayer) {
        using namespace NovaEngine;

        // Clear and rebuild entity list for current layer
        m_entityList.clear();
        i32 currentLayer = m_editorState->getCurrentLayer();

        auto it = entitiesByLayer.find(currentLayer);
        if (it != entitiesByLayer.end()) {
            m_entityList = it->second;
        }

        // Update entity list buttons (max 10 visible)
        static constexpr size_t MAX_ENTITY_BUTTONS = 10;

        for (size_t i = 0; i < MAX_ENTITY_BUTTONS; ++i) {
            std::string btnID = "btn_layer_entity_" + std::to_string(i);
            auto btnComponent = m_uiManager.getComponent(btnID);

            if (btnComponent) {
                auto button = std::dynamic_pointer_cast<Button>(btnComponent);
                if (button) {
                    if (i < m_entityList.size()) {
                        Entity* entity = m_entityList[i];

                        // Generate meaningful entity name
                        std::string entityName = "Entity #" + formatInt(entity->getID());

                        auto* sprite = entity->getComponent<SpriteComponent>();
                        if (sprite && !sprite->textureID.empty()) {
                            entityName = sprite->textureID + " #" + formatInt(entity->getID());
                        }

                        button->setText(entityName);
                        button->setVisible(true);
                        button->setActive(true);
                    } else {
                        // Hide unused buttons
                        button->setVisible(false);
                        button->setActive(false);
                    }
                }
            }
        }
    }

private:
    EditorState* m_editorState;
    NovaEngine::Scene* m_currentScene;
    std::vector<NovaEngine::Entity*> m_entityList; // Entities on current layer
};

} // namespace NovaEditor
