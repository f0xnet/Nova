#include "EditorApplication.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>

namespace NovaEditor {

EditorApplication::EditorApplication()
    : Application()
    , m_currentScene(nullptr)
{
    // Configure application for 4K windowed borderless
    m_config.windowTitle = "Nova Level Editor";
    m_config.windowWidth = 3840;
    m_config.windowHeight = 2160;
    m_config.fullscreen = false;

    LOG_INFO("EditorApplication created");
}

EditorApplication::~EditorApplication() {
    LOG_INFO("EditorApplication destroyed");
}

bool EditorApplication::onInitialize() {
    LOG_INFO("=== Nova Level Editor - Phase 1 ===");

    // Create editor subsystems
    m_editorConfig = std::make_unique<EditorConfig>();
    m_state = std::make_unique<EditorState>();
    m_camera = std::make_unique<EditorCamera>();
    m_uiManager_editor = std::make_unique<EditorUIManager>(m_uiManager, m_state.get(), m_editorConfig.get());

    // Initialize SceneManager with entity definitions
    if (!m_sceneManager.initialize("data/definitions", "data/scenegraph.json")) {
        LOG_WARN("Failed to initialize SceneManager - scenes may not load properly");
    }

    // Initialize editor
    initializeEditor();

    // Initialize UI Manager and load JSON layouts
    if (!m_uiManager_editor->initialize()) {
        LOG_ERROR("Failed to initialize EditorUIManager");
        return false;
    }

    // Set up UI action callback
    m_uiManager_editor->setActionCallback([this](const std::string& action, const std::string& value) {
        onUIAction(action, value);
    });

    // Apply camera settings from config
    m_camera->setMovementSpeed(m_editorConfig->getCameraMovementSpeed());
    m_camera->setZoomSpeed(m_editorConfig->getCameraZoomSpeed());
    m_camera->setZoomLimits(m_editorConfig->getCameraMinZoom(), m_editorConfig->getCameraMaxZoom());

    // Create a default empty scene
    m_currentScene = new NovaEngine::Scene("EditorScene");

    printControls();

    LOG_INFO("EditorApplication initialized successfully");
    return true;
}

void EditorApplication::onUpdate(float deltaTime) {
    // Update camera
    updateCamera(deltaTime);

    // Update UI
    m_uiManager.update(deltaTime);
    m_uiManager_editor->update(deltaTime);

    // Update current scene
    if (m_currentScene) {
        m_currentScene->update(deltaTime);
    }
}

void EditorApplication::onRender() {
    // Render scene with camera view
    renderScene();

    // Render grid
    if (m_state->isGridEnabled()) {
        renderGrid();
    }

    // Render UI (in screen space)
    renderUI();
}

void EditorApplication::onEvent(const NovaEngine::Event& event) {
    using namespace NovaEngine;

    // UI gets first priority
    m_uiManager.dispatchEvent(event);

    // Then editor input
    if (event.type == EventType::Input) {
        handleEditorInput(event.inputEvent);
    }
}

void EditorApplication::onShutdown() {
    LOG_INFO("EditorApplication shutting down");

    // Cleanup scene
    if (m_currentScene) {
        delete m_currentScene;
        m_currentScene = nullptr;
    }

    // Cleanup subsystems (unique_ptr handles this automatically)
}

void EditorApplication::initializeEditor() {
    LOG_INFO("Initializing editor subsystems...");

    // Set grid from config
    m_state->setGridEnabled(m_editorConfig->isGridEnabledByDefault());
    m_state->setGridSize(m_editorConfig->getDefaultGridSize());

    LOG_INFO("Editor subsystems initialized");
}

void EditorApplication::printControls() {
    LOG_INFO("=== Editor Controls ===");
    LOG_INFO("Camera: WASD or Arrow keys to move");
    LOG_INFO("UI: Click buttons to trigger actions (Phase 1: logs only)");
    LOG_INFO("ESC: Exit editor");
}

void EditorApplication::updateCamera(float deltaTime) {
    m_camera->update(deltaTime);
}

void EditorApplication::renderScene() {
    using namespace NovaEngine;

    // Apply camera viewport
    // (Camera's updateViewport() is called in update())

    // Render current scene
    if (m_currentScene) {
        m_currentScene->render();
    }
}

void EditorApplication::renderGrid() {
    using namespace NovaEngine;

    // TODO: Implement grid rendering
    // For Phase 1, we skip this (not critical for UI testing)
}

void EditorApplication::renderUI() {
    using namespace NovaEngine;

    // Reset viewport to screen space for UI
    VIEWPORT().resetView();

    // Render UI
    m_uiManager.render();
}

void EditorApplication::handleEditorInput(const NovaEngine::InputEvent& input) {
    using namespace NovaEngine;

    if (input.type == InputEventType::KeyPressed) {
        handleKeyPress(input);
    }
    else if (input.type == InputEventType::MouseButtonPressed) {
        // Only handle editor clicks if not over UI
        Vec2i mousePos{input.mouseButton.x, input.mouseButton.y};
        if (!m_uiManager.isMouseOverUI(mousePos)) {
            handleMouseClick(input);
        }
    }
    // Note: Mouse wheel scrolling not supported in current InputEvent
}

void EditorApplication::handleKeyPress(const NovaEngine::InputEvent& input) {
    using namespace NovaEngine;

    // ESC: Exit
    if (input.key.code == KeyCode::Escape) {
        quit();
    }

    // Ctrl+N: New scene
    if (input.key.code == KeyCode::N && input.key.control) {
        newScene();
    }

    // Ctrl+O: Load scene
    if (input.key.code == KeyCode::O && input.key.control) {
        loadScene();
    }

    // Ctrl+S: Save scene
    if (input.key.code == KeyCode::S && input.key.control) {
        saveScene();
    }

    // G: Toggle grid
    if (input.key.code == KeyCode::G) {
        m_state->setGridEnabled(!m_state->isGridEnabled());
        LOG_INFO("Grid: {}", m_state->isGridEnabled() ? "ON" : "OFF");
    }
}

void EditorApplication::handleMouseClick(const NovaEngine::InputEvent& input) {
    using namespace NovaEngine;

    Vec2i screenPos{input.mouseButton.x, input.mouseButton.y};
    Vec2f worldPos = m_camera->screenToWorld(screenPos);

    LOG_DEBUG("Editor click at screen ({}, {}) -> world ({}, {})",
              screenPos.x, screenPos.y, worldPos.x, worldPos.y);

    // Phase 1: Just log clicks
    // Future phases will handle entity selection here
}

void EditorApplication::onUIAction(const std::string& action, const std::string& value) {
    using namespace NovaEngine;

    LOG_INFO("=== UI ACTION === '{}' with value '{}'", action, value);

    // Handle UI actions
    if (action == "scene_new") {
        newScene();
    }
    else if (action == "scene_load") {
        loadScene();
    }
    else if (action == "scene_save") {
        saveScene();
    }
    else if (action == "edit_undo") {
        LOG_INFO("Undo requested (not yet implemented)");
    }
    else if (action == "edit_redo") {
        LOG_INFO("Redo requested (not yet implemented)");
    }
    else if (action == "toggle_grid") {
        m_state->setGridEnabled(!m_state->isGridEnabled());
        LOG_INFO("Grid toggled: {}", m_state->isGridEnabled() ? "ON" : "OFF");
        // TODO: Update button text in UI
    }
    else if (action == "palette_category") {
        m_uiManager_editor->updateEntityPalette(value);
    }
    else {
        LOG_WARN("Unknown action: '{}'", action);
    }
}

void EditorApplication::newScene() {
    LOG_INFO("New scene requested");

    // Phase 1: Just log
    // Future: Clear current scene, create new empty scene
}

void EditorApplication::loadScene() {
    LOG_INFO("Load scene requested");

    // TODO: Show file dialog to select scene
    // For now, we'll try to load a test scene
    std::string scenePath = "data/scenes/test_scene.json";
    std::string sceneName = "LoadedScene";

    LOG_INFO("Attempting to load scene from: {}", scenePath);

    // Unload current scene if exists
    if (m_currentScene) {
        LOG_INFO("Unloading current scene...");
        delete m_currentScene;
        m_currentScene = nullptr;
    }

    // Load scene using SceneManager
    if (m_sceneManager.loadScene(scenePath, sceneName)) {
        // Get the loaded scene
        m_currentScene = m_sceneManager.getScene(sceneName);

        if (m_currentScene) {
            // Set as active scene for rendering
            m_sceneManager.setActiveScene(sceneName);

            // Update editor state
            m_state->setCurrentScenePath(scenePath);
            m_state->setSceneModified(false);

            // Center camera on scene (0,0 for now)
            m_camera->focusOn(NovaEngine::Vec2f{0.0f, 0.0f});
            m_camera->setZoom(1.0f);

            // Refresh UI
            m_uiManager_editor->refreshSceneHierarchy();

            LOG_INFO("Scene loaded successfully: {} entities",
                     m_currentScene->getEntityRegistry().getEntityCount());
        } else {
            LOG_ERROR("Scene was loaded but could not be retrieved from SceneManager");
        }
    } else {
        LOG_ERROR("Failed to load scene from: {}", scenePath);
        LOG_INFO("Creating empty scene instead...");

        // Create empty scene as fallback
        m_currentScene = new NovaEngine::Scene("EditorScene");
        m_state->setCurrentScenePath("");
        m_state->setSceneModified(false);
    }
}

void EditorApplication::saveScene() {
    LOG_INFO("Save scene requested");

    // Phase 1: Just log
    // Future: Show save dialog if needed, save scene to JSON
}

} // namespace NovaEditor
