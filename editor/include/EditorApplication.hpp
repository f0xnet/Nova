#pragma once

#include <NovaEngine/Core/Application.hpp>
#include <NovaEngine/ECS/SceneManager.hpp>
#include <NovaEngine/UI/UIManager.hpp>
#include "Core/EditorState.hpp"
#include "Core/EditorConfig.hpp"
#include "UI/EditorUIManager.hpp"
#include "EditorCamera.hpp"
#include <memory>

namespace NovaEditor {

/**
 * @brief Main editor application (Phase 1: JSON-based UI foundation)
 *
 * Phase 1 Goals:
 * - Launch editor with JSON-based UI
 * - Display toolbar, palette, inspector, hierarchy
 * - Button actions trigger console logs
 * - Clean modular architecture
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

    // Update
    void updateCamera(float deltaTime);

    // Render
    void renderScene();
    void renderGrid();
    void renderSelectionBox();
    void renderUI();

    // Input handling
    void handleEditorInput(const NovaEngine::InputEvent& input);
    void handleKeyPress(const NovaEngine::InputEvent& input);
    void handleMouseClick(const NovaEngine::InputEvent& input);

    // UI action callback
    void onUIAction(const std::string& action, const std::string& value);

    // Scene management
    void newScene();
    void loadScene();
    void loadScene(const std::string& scenePath);
    void saveScene();

    // Scene utilities
    std::vector<std::string> getAvailableScenes() const;
    void showSceneSelectionDialog();

    // Entity management
    std::vector<std::string> getAvailableEntities(const std::string& category) const;
    void showEntitySelectionDialog(const std::string& category);
    void enterPlacementMode(const std::string& entityType, const std::string& entityId);
    void exitPlacementMode();
    void placeEntity(const NovaEngine::Vec2f& position);
    void selectEntity(NovaEngine::Entity* entity);
    void startDraggingEntity();
    void updateDraggingEntity(const NovaEngine::Vec2f& worldPos);
    void stopDraggingEntity();

    // Layers panel
    void updateLayersPanel();
    void toggleLayersPanel();

    // Entity properties panel
    void showEntityProperties();
    void hideEntityProperties();
    void updateEntityPropertiesPanel();

private:
    // Core systems
    NovaEngine::SceneManager m_sceneManager;
    NovaEngine::UIManager m_uiManager;

    // Editor subsystems
    std::unique_ptr<EditorConfig> m_editorConfig;
    std::unique_ptr<EditorState> m_state;
    std::unique_ptr<EditorCamera> m_camera;
    std::unique_ptr<EditorUIManager> m_uiManager_editor;

    // Current scene
    NovaEngine::Scene* m_currentScene;

    // Scene selection dialog
    std::vector<std::string> m_availableScenes;
    size_t m_sceneDialogPage;
    static constexpr size_t SCENES_PER_PAGE = 10;

    // Entity selection and placement
    static constexpr size_t ENTITIES_PER_PAGE = 10;
    std::vector<std::string> m_availableEntities;
    size_t m_entityDialogPage;
    std::string m_currentEntityCategory;
    std::string m_placementEntityType;
    std::string m_placementEntityId;
    bool m_isPlacementMode;

    // Entity dragging
    NovaEngine::Entity* m_draggedEntity;
    NovaEngine::Vec2f m_dragOffset;
    bool m_isDragging;

    // Layers panel entity list
    std::vector<NovaEngine::Entity*> m_layerEntitiesList;
};

} // namespace NovaEditor
