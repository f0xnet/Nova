#pragma once

#include "UI/TimeOfDayPanel.hpp"
#include "UI/PostProcessPanel.hpp"
#include "UI/EntityPropertiesPanel.hpp"
#include "UI/LayersPanel.hpp"
#include "EditorState.hpp"
#include <NovaEngine/UI/UIManager.hpp>
#include <NovaEngine/Systems/LightingSystem.hpp>
#include <NovaEngine/Rendering/Effects/DynamicLightingEffect.hpp>
#include <NovaEngine/Rendering/Effects/BloomEffect.hpp>
#include <NovaEngine/Rendering/Effects/ColorGradingEffect.hpp>
#include <NovaEngine/ECS/SceneManager.hpp>
#include <NovaEngine/ECS/Scene.hpp>
#include <memory>

namespace NovaEditor {

/**
 * @brief Centralized manager for all editor UI panels
 *
 * Responsibilities:
 * - Create and initialize all panels
 * - Route UI actions to appropriate panels
 * - Manage panel lifecycle and dependencies
 * - Coordinate inter-panel communication
 *
 * Benefits:
 * - Single Responsibility: Only manages panels
 * - Reduces EditorApplication complexity
 * - Centralized panel coordination
 */
class EditorPanelManager {
public:
    EditorPanelManager(
        NovaEngine::UIManager& uiManager,
        EditorState* editorState,
        NovaEngine::SceneManager* sceneManager
    )
        : m_uiManager(uiManager)
        , m_editorState(editorState)
        , m_sceneManager(sceneManager)
    {}

    /**
     * @brief Initialize all panels with required systems
     */
    void initialize(
        NovaEngine::LightingSystem* lightingSystem,
        NovaEngine::DynamicLightingEffect* lightingEffect,
        NovaEngine::BloomEffect* bloomEffect,
        NovaEngine::ColorGradingEffect* colorGradingEffect,
        NovaEngine::Scene* currentScene
    ) {
        LOG_INFO("Initializing editor panels...");

        // Time of Day panel
        if (lightingEffect) {
            m_timeOfDayPanel = std::make_unique<TimeOfDayPanel>(
                m_uiManager,
                lightingSystem,
                lightingEffect
            );
            LOG_INFO("TimeOfDayPanel initialized");
        }

        // Post-processing panel
        if (bloomEffect && colorGradingEffect) {
            m_postProcessPanel = std::make_unique<PostProcessPanel>(
                m_uiManager,
                bloomEffect,
                colorGradingEffect
            );
            LOG_INFO("PostProcessPanel initialized");
        }

        // Entity properties panel (with layer change callback)
        m_entityPropertiesPanel = std::make_unique<EntityPropertiesPanel>(
            m_uiManager,
            m_editorState,
            m_sceneManager,
            [this]() {
                // Sync layers panel when entity layer changes
                if (m_layersPanel) {
                    m_layersPanel->update();
                }
            }
        );
        LOG_INFO("EntityPropertiesPanel initialized");

        // Layers panel
        m_layersPanel = std::make_unique<LayersPanel>(
            m_uiManager,
            m_editorState,
            currentScene
        );
        LOG_INFO("LayersPanel initialized");

        LOG_INFO("All editor panels initialized successfully");
    }

    /**
     * @brief Handle UI action routing to appropriate panel
     */
    bool handleAction(const std::string& action, const std::string& value) {
        // Time of Day panel actions
        if (action == "show_time_of_day") {
            if (m_timeOfDayPanel) m_timeOfDayPanel->show();
            return true;
        }
        if (action == "close_time_panel") {
            if (m_timeOfDayPanel) m_timeOfDayPanel->hide();
            return true;
        }
        if (action == "set_time") {
            if (m_timeOfDayPanel) {
                try {
                    float time = std::stof(value);
                    m_timeOfDayPanel->setTimeOfDay(time);
                } catch (const std::exception& e) {
                    LOG_ERROR("Failed to parse time value: {}", e.what());
                }
            }
            return true;
        }
        if (action == "adjust_ambient") {
            if (m_timeOfDayPanel) {
                try {
                    float delta = std::stof(value);
                    m_timeOfDayPanel->adjustAmbientDarkness(delta);
                } catch (const std::exception& e) {
                    LOG_ERROR("Failed to parse ambient delta: {}", e.what());
                }
            }
            return true;
        }

        // Post-processing panel actions
        if (action == "show_postprocess") {
            if (m_postProcessPanel) m_postProcessPanel->show();
            return true;
        }
        if (action == "close_pp_panel") {
            if (m_postProcessPanel) m_postProcessPanel->hide();
            return true;
        }
        if (action == "adjust_bloom") {
            if (m_postProcessPanel) {
                try {
                    float delta = std::stof(value);
                    m_postProcessPanel->adjustBloomIntensity(delta);
                } catch (const std::exception& e) {
                    LOG_ERROR("Failed to parse bloom delta: {}", e.what());
                }
            }
            return true;
        }
        if (action == "adjust_saturation") {
            if (m_postProcessPanel) {
                try {
                    float delta = std::stof(value);
                    m_postProcessPanel->adjustSaturation(delta);
                } catch (const std::exception& e) {
                    LOG_ERROR("Failed to parse saturation delta: {}", e.what());
                }
            }
            return true;
        }
        if (action == "adjust_contrast") {
            if (m_postProcessPanel) {
                try {
                    float delta = std::stof(value);
                    m_postProcessPanel->adjustContrast(delta);
                } catch (const std::exception& e) {
                    LOG_ERROR("Failed to parse contrast delta: {}", e.what());
                }
            }
            return true;
        }
        if (action == "adjust_brightness") {
            if (m_postProcessPanel) {
                try {
                    float delta = std::stof(value);
                    m_postProcessPanel->adjustBrightness(delta);
                } catch (const std::exception& e) {
                    LOG_ERROR("Failed to parse brightness delta: {}", e.what());
                }
            }
            return true;
        }

        // Entity properties panel actions
        if (action == "show_properties") {
            if (m_entityPropertiesPanel && m_editorState->getSelectedEntity()) {
                m_entityPropertiesPanel->show();
            } else {
                LOG_INFO("No entity selected - select an entity first");
            }
            return true;
        }
        if (action == "close_properties") {
            if (m_entityPropertiesPanel) m_entityPropertiesPanel->hide();
            return true;
        }
        if (action == "apply_properties") {
            if (m_entityPropertiesPanel) m_entityPropertiesPanel->applyChanges();
            return true;
        }
        if (action == "modify_property") {
            if (m_entityPropertiesPanel) {
                try {
                    size_t colonPos = value.find(':');
                    if (colonPos != std::string::npos) {
                        int propIndex = std::stoi(value.substr(0, colonPos));
                        float delta = std::stof(value.substr(colonPos + 1));
                        m_entityPropertiesPanel->modifyProperty(propIndex, delta);
                    }
                } catch (const std::exception& e) {
                    LOG_ERROR("Failed to parse modify_property value: {}", e.what());
                }
            }
            return true;
        }

        // Layers panel actions
        if (action == "toggle_layers") {
            if (m_layersPanel) m_layersPanel->toggle();
            return true;
        }
        if (action == "set_layer") {
            try {
                NovaEngine::i32 layer = std::stoi(value);
                m_editorState->setCurrentLayer(layer);
                LOG_INFO("Current layer set to: {}", layer);
                if (m_layersPanel) m_layersPanel->update();
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to parse layer value: {}", e.what());
            }
            return true;
        }
        if (action == "select_entity_from_list") {
            if (m_layersPanel) {
                try {
                    size_t index = std::stoul(value);
                    NovaEngine::Entity* entity = m_layersPanel->getEntityFromList(index);
                    if (entity) {
                        // Return false so EditorApplication can handle entity selection
                        return false;
                    } else {
                        LOG_ERROR("Invalid entity list index: {}", index);
                    }
                } catch (const std::exception& e) {
                    LOG_ERROR("Failed to parse entity index: {}", e.what());
                }
            }
            return true;
        }

        return false; // Action not handled by panels
    }

    /**
     * @brief Update layers panel (called when scene changes)
     */
    void updateLayersPanel(NovaEngine::Scene* scene) {
        if (m_layersPanel) {
            m_layersPanel->setScene(scene);
            m_layersPanel->update();
        }
    }

    /**
     * @brief Get entity from layers panel list
     */
    NovaEngine::Entity* getEntityFromLayersList(size_t index) const {
        return m_layersPanel ? m_layersPanel->getEntityFromList(index) : nullptr;
    }

private:
    NovaEngine::UIManager& m_uiManager;
    EditorState* m_editorState;
    NovaEngine::SceneManager* m_sceneManager;

    // Panels
    std::unique_ptr<TimeOfDayPanel> m_timeOfDayPanel;
    std::unique_ptr<PostProcessPanel> m_postProcessPanel;
    std::unique_ptr<EntityPropertiesPanel> m_entityPropertiesPanel;
    std::unique_ptr<LayersPanel> m_layersPanel;
};

} // namespace NovaEditor
