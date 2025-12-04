#pragma once

#include <NovaEngine/Core/Application.hpp>
#include <NovaEngine/ECS/SceneManager.hpp>
#include <NovaEngine/UI/UIManager.hpp>
#include <NovaEngine/Systems/LightingSystem.hpp>
#include <NovaEngine/Rendering/PostProcessPipeline.hpp>
#include <NovaEngine/Rendering/Effects/DynamicLightingEffect.hpp>
#include <NovaEngine/Rendering/Effects/BloomEffect.hpp>
#include <NovaEngine/Rendering/Effects/ColorGradingEffect.hpp>
#include "Core/EditorState.hpp"
#include "Core/EditorConfig.hpp"
#include "Core/EditorPanelManager.hpp"
#include "Core/EntityEditorController.hpp"
#include "Core/SceneEditorController.hpp"
#include "Core/EditorInputHandler.hpp"
#include "UI/EditorUIManager.hpp"
#include "EditorCamera.hpp"
#include <memory>

namespace NovaEditor {

/**
 * @brief Main editor application - Orchestrator pattern
 *
 * REFACTORED: Now uses specialized controllers instead of doing everything directly.
 * Acts as an orchestrator that coordinates between subsystems.
 *
 * Responsibilities (REDUCED from 50+ methods to ~15):
 * - Application lifecycle management
 * - Coordinate between controllers
 * - Rendering orchestration
 * - Event distribution
 *
 * Controllers:
 * - EditorPanelManager: All UI panels
 * - EntityEditorController: Entity operations
 * - SceneEditorController: Scene operations
 * - EditorInputHandler: Input processing
 */
class EditorApplication : public NovaEngine::Application {
public:
    EditorApplication();
    ~EditorApplication() override;

protected:
    // Application lifecycle hooks
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onEvent(const NovaEngine::Event& event) override;
    void onShutdown() override;

private:
    // Initialization
    void initializeEditor();
    void printControls();

    // Rendering
    void renderScene();
    void renderGrid();
    void renderSelectionBox();
    void renderUI();

    // Input handling (delegates to InputHandler)
    void handleEditorInput(const NovaEngine::InputEvent& input);

    // UI action routing (delegates to controllers)
    void onUIAction(const std::string& action, const std::string& value);

    // Entity selection helper
    void selectEntityAtPosition(const NovaEngine::Vec2f& worldPos);

    // Scene utilities
    std::vector<std::string> getAvailableScenes() const;
    void showSceneSelectionDialog();

    // Entity utilities
    std::vector<std::string> getAvailableEntities(const std::string& category) const;
    void showEntitySelectionDialog(const std::string& category);
private:
    // Core systems
    NovaEngine::SceneManager m_sceneManager;
    NovaEngine::UIManager m_uiManager;
    std::unique_ptr<NovaEngine::LightingSystem> m_lightingSystem;
    std::unique_ptr<NovaEngine::PostProcessPipeline> m_postProcessPipeline;

    // Effect pointers (owned by PostProcessPipeline)
    NovaEngine::DynamicLightingEffect* m_dynamicLightingEffect;
    NovaEngine::BloomEffect* m_bloomEffect;
    NovaEngine::ColorGradingEffect* m_colorGradingEffect;

    // CONTROLLERS (NEW - Single Responsibility Pattern)
    std::unique_ptr<EditorPanelManager> m_panelManager;
    std::unique_ptr<EntityEditorController> m_entityController;
    std::unique_ptr<SceneEditorController> m_sceneController;
    std::unique_ptr<EditorInputHandler> m_inputHandler;

    // Editor subsystems
    std::unique_ptr<EditorConfig> m_editorConfig;
    std::unique_ptr<EditorState> m_state;
    std::unique_ptr<EditorCamera> m_camera;
    std::unique_ptr<EditorUIManager> m_uiManager_editor;

    // Current scene
    NovaEngine::Scene* m_currentScene;

    // UI dialog state
    std::vector<std::string> m_availableScenes;
    std::vector<std::string> m_availableEntities;
    size_t m_sceneDialogPage;
    size_t m_entityDialogPage;
    std::string m_currentEntityCategory;

    static constexpr size_t SCENES_PER_PAGE = 10;
    static constexpr size_t ENTITIES_PER_PAGE = 10;
};

} // namespace NovaEditor
