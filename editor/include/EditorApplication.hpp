#pragma once

#include <NovaEngine/Core/Application.hpp>
#include <NovaEngine/ECS/SceneManager.hpp>
#include <NovaEngine/UI/UIManager.hpp>
#include "EditorState.hpp"
#include "EditorCamera.hpp"
#include "EditorUI.hpp"
#include "EntityPalette.hpp"

namespace NovaEditor {

/**
 * @brief Application principale de l'éditeur de niveau NovaEngine
 *
 * Éditeur visuel intégré permettant de :
 * - Créer et éditer des scènes
 * - Placer visuellement des entités (sprites, lumières, NPCs, etc.)
 * - Éditer les propriétés des entités
 * - Prévisualiser en temps réel
 * - Sauvegarder/charger en JSON
 */
class EditorApplication : public NovaEngine::Application {
public:
    EditorApplication();
    ~EditorApplication() override;

protected:
    // Hooks Application
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onEvent(const NovaEngine::Event& event) override;
    void onShutdown() override;

private:
    // Initialisation
    void initializeEditor();
    void loadDefinitions();

    // Update
    void updateCamera(float deltaTime);
    void updateSelection();
    void updateEntityManipulation(float deltaTime);

    // Render
    void renderScene();
    void renderGrid();
    void renderGizmos();
    void renderUI();

    // Input handling
    void handleEditorInput(const NovaEngine::InputEvent& input);
    void handleEntityPlacement(const NovaEngine::Vec2f& worldPos);
    void handleEntitySelection(const NovaEngine::Vec2f& worldPos);
    void handleEntityDrag(const NovaEngine::Vec2f& worldPos);

    // Scene management
    void newScene();
    void loadScene(const std::string& path);
    void saveScene(const std::string& path);

    // Entity operations
    void createEntity(const std::string& type, const NovaEngine::Vec2f& position);
    void deleteSelectedEntity();
    void duplicateSelectedEntity();

    // UI callbacks
    void onUIAction(const std::string& action, const std::string& value);

private:
    // Core systems
    NovaEngine::SceneManager m_sceneManager;
    NovaEngine::UIManager m_uiManager;

    // Editor-specific
    std::unique_ptr<EditorState> m_editorState;
    std::unique_ptr<EditorCamera> m_editorCamera;
    std::unique_ptr<EditorUI> m_editorUI;
    std::unique_ptr<EntityPalette> m_entityPalette;

    // Current scene
    NovaEngine::Scene* m_currentScene;
    std::string m_currentScenePath;
    bool m_sceneModified;

    // Dragging state
    bool m_isDragging;
    NovaEngine::Vec2f m_dragStartPos;
    NovaEngine::Vec2f m_dragOffset;

    // Grid
    bool m_showGrid;
    float m_gridSize;
    bool m_snapToGrid;
};

} // namespace NovaEditor
