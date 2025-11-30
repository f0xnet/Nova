#include "EditorApplication.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>
#include <NovaEngine/UI/Components/Button.hpp>
#include <filesystem>
#include <algorithm>

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
    if (!m_sceneManager.initialize("data/definitions/", "data/scenegraph.json")) {
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

    // Get viewport information
    Vec2f viewCenter = VIEWPORT().getViewCenter();
    Vec2f viewSize = VIEWPORT().getViewSize();

    // Calculate visible bounds (with some padding)
    f32 padding = 100.0f;
    f32 left = viewCenter.x - (viewSize.x / 2.0f) - padding;
    f32 right = viewCenter.x + (viewSize.x / 2.0f) + padding;
    f32 top = viewCenter.y - (viewSize.y / 2.0f) - padding;
    f32 bottom = viewCenter.y + (viewSize.y / 2.0f) + padding;

    // Get grid size
    f32 gridSize = m_state->getGridSize();

    // Grid color (subtle gray)
    Color gridColor(80, 80, 80, 100);  // Semi-transparent gray
    f32 lineThickness = 1.0f;

    // Draw vertical lines
    i32 startX = static_cast<i32>(left / gridSize) - 1;
    i32 endX = static_cast<i32>(right / gridSize) + 1;
    for (i32 i = startX; i <= endX; ++i) {
        f32 x = i * gridSize;

        RectData line;
        line.position = Vec2f{x - lineThickness / 2.0f, top};
        line.size = Vec2f{lineThickness, bottom - top};
        line.fillColor = gridColor;
        line.outlineThickness = 0;

        GRAPHICS().drawRect(line);
    }

    // Draw horizontal lines
    i32 startY = static_cast<i32>(top / gridSize) - 1;
    i32 endY = static_cast<i32>(bottom / gridSize) + 1;
    for (i32 i = startY; i <= endY; ++i) {
        f32 y = i * gridSize;

        RectData line;
        line.position = Vec2f{left, y - lineThickness / 2.0f};
        line.size = Vec2f{right - left, lineThickness};
        line.fillColor = gridColor;
        line.outlineThickness = 0;

        GRAPHICS().drawRect(line);
    }

    // Draw axis lines (thicker and different color for x=0 and y=0)
    Color axisColor(120, 120, 120, 150);  // Slightly brighter
    f32 axisThickness = 2.0f;

    // Y-axis (vertical line at x=0)
    RectData yAxis;
    yAxis.position = Vec2f{-axisThickness / 2.0f, top};
    yAxis.size = Vec2f{axisThickness, bottom - top};
    yAxis.fillColor = axisColor;
    yAxis.outlineThickness = 0;
    GRAPHICS().drawRect(yAxis);

    // X-axis (horizontal line at y=0)
    RectData xAxis;
    xAxis.position = Vec2f{left, -axisThickness / 2.0f};
    xAxis.size = Vec2f{right - left, axisThickness};
    xAxis.fillColor = axisColor;
    xAxis.outlineThickness = 0;
    GRAPHICS().drawRect(xAxis);
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
        bool gridEnabled = m_state->isGridEnabled();
        LOG_INFO("Grid toggled: {}", gridEnabled ? "ON" : "OFF");

        // Update button text to reflect current state
        auto btnToggleGrid = m_uiManager.getComponent("btn_toggle_grid");
        if (btnToggleGrid) {
            auto button = std::dynamic_pointer_cast<NovaEngine::Button>(btnToggleGrid);
            if (button) {
                button->setText(gridEnabled ? "Grid: ON" : "Grid: OFF");
            }
        }
    }
    else if (action == "palette_category") {
        m_uiManager_editor->updateEntityPalette(value);
    }
    else if (action == "load_scene_by_index") {
        // Load scene by index from available scenes list
        try {
            size_t index = std::stoul(value);
            if (index < m_availableScenes.size()) {
                std::string scenePath = m_editorConfig->getScenesPath() + "/" + m_availableScenes[index];
                LOG_INFO("Loading scene: {}", m_availableScenes[index]);

                // Close dialog
                m_uiManager.setGroupActive("scene_dialog", false);

                // Load the scene
                loadScene(scenePath);
            } else {
                LOG_ERROR("Invalid scene index: {}", index);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse scene index: {}", e.what());
        }
    }
    else if (action == "close_scene_dialog") {
        // Close the scene selection dialog
        m_uiManager.setGroupActive("scene_dialog", false);
        LOG_INFO("Scene selection dialog closed");
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

    // Show scene selection dialog
    showSceneSelectionDialog();
}

void EditorApplication::loadScene(const std::string& scenePath) {
    LOG_INFO("Attempting to load scene from: {}", scenePath);

    std::string sceneName = "LoadedScene";

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

std::vector<std::string> EditorApplication::getAvailableScenes() const {
    namespace fs = std::filesystem;
    std::vector<std::string> scenes;

    std::string scenesPath = m_editorConfig->getScenesPath();

    try {
        if (fs::exists(scenesPath) && fs::is_directory(scenesPath)) {
            for (const auto& entry : fs::directory_iterator(scenesPath)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    // Store relative path from data/scenes/
                    scenes.push_back(entry.path().filename().string());
                }
            }
        } else {
            LOG_WARN("Scenes directory not found: {}", scenesPath);
        }
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR("Error scanning scenes directory: {}", e.what());
    }

    // Sort alphabetically
    std::sort(scenes.begin(), scenes.end());

    LOG_INFO("Found {} scene(s) in {}", scenes.size(), scenesPath);
    return scenes;
}

void EditorApplication::showSceneSelectionDialog() {
    using namespace NovaEngine;

    // Get available scenes
    m_availableScenes = getAvailableScenes();

    if (m_availableScenes.empty()) {
        LOG_WARN("No scenes found in data/scenes/");
        return;
    }

    LOG_INFO("=== Available Scenes ===");
    for (size_t i = 0; i < m_availableScenes.size(); ++i) {
        LOG_INFO("  [{}] {}", i, m_availableScenes[i]);
    }

    // Update button text and visibility for each scene slot (max 10)
    for (size_t i = 0; i < 10; ++i) {
        std::string buttonID = "btn_scene_" + std::to_string(i);
        auto btnComponent = m_uiManager.getComponent(buttonID);

        if (btnComponent) {
            auto button = std::dynamic_pointer_cast<NovaEngine::Button>(btnComponent);
            if (button) {
                if (i < m_availableScenes.size()) {
                    // Show button with scene name
                    button->setText(m_availableScenes[i]);
                    button->setActive(true);
                } else {
                    // Hide unused button
                    button->setActive(false);
                }
            }
        }
    }

    // Show the dialog
    m_uiManager.setGroupActive("scene_dialog", true);
    LOG_INFO("Scene selection dialog opened with {} scenes", m_availableScenes.size());
}

} // namespace NovaEditor
