#include "EditorApplication.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>
#include <NovaEngine/UI/Components/Button.hpp>
#include <NovaEngine/UI/Components/Text.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <limits>
#include <cmath>

namespace NovaEditor {

EditorApplication::EditorApplication()
    : Application()
    , m_currentScene(nullptr)
    , m_sceneDialogPage(0)
    , m_entityDialogPage(0)
    , m_isPlacementMode(false)
    , m_draggedEntity(nullptr)
    , m_isDragging(false)
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

    auto initStart = std::chrono::high_resolution_clock::now();

    // Create editor subsystems
    LOG_INFO("Creating editor subsystems...");
    auto subsysStart = std::chrono::high_resolution_clock::now();
    m_editorConfig = std::make_unique<EditorConfig>();
    m_state = std::make_unique<EditorState>();
    m_camera = std::make_unique<EditorCamera>();
    m_uiManager_editor = std::make_unique<EditorUIManager>(m_uiManager, m_state.get(), m_editorConfig.get());
    auto subsysEnd = std::chrono::high_resolution_clock::now();
    LOG_INFO("Subsystems creation took {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(subsysEnd - subsysStart).count());

    // Initialize SceneManager with entity definitions
    LOG_INFO("Initializing SceneManager...");
    auto sceneManStart = std::chrono::high_resolution_clock::now();
    if (!m_sceneManager.initialize("data/definitions/", "data/scenegraph.json")) {
        LOG_WARN("Failed to initialize SceneManager - scenes may not load properly");
    }
    auto sceneManEnd = std::chrono::high_resolution_clock::now();
    LOG_INFO("SceneManager initialization took {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(sceneManEnd - sceneManStart).count());

    // Initialize editor
    initializeEditor();

    // Initialize UI Manager and load JSON layouts
    LOG_INFO("Initializing UI Manager...");
    auto uiStart = std::chrono::high_resolution_clock::now();
    if (!m_uiManager_editor->initialize()) {
        LOG_ERROR("Failed to initialize EditorUIManager");
        return false;
    }
    auto uiEnd = std::chrono::high_resolution_clock::now();
    LOG_INFO("UI Manager initialization took {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(uiEnd - uiStart).count());

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

    auto initEnd = std::chrono::high_resolution_clock::now();
    LOG_INFO("Total initialization took {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(initEnd - initStart).count());
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

    // Render selection box around selected entity
    renderSelectionBox();

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

    auto startTime = std::chrono::high_resolution_clock::now();

    // Cleanup scene
    if (m_currentScene) {
        LOG_INFO("Deleting current scene...");
        auto sceneStart = std::chrono::high_resolution_clock::now();
        delete m_currentScene;
        m_currentScene = nullptr;
        auto sceneEnd = std::chrono::high_resolution_clock::now();
        auto sceneDuration = std::chrono::duration_cast<std::chrono::milliseconds>(sceneEnd - sceneStart).count();
        LOG_INFO("Scene deletion took {} ms", sceneDuration);
    }

    // Cleanup subsystems (unique_ptr handles this automatically)
    LOG_INFO("Cleaning up subsystems...");
    auto subsystemsStart = std::chrono::high_resolution_clock::now();
    m_uiManager_editor.reset();
    m_camera.reset();
    m_state.reset();
    m_editorConfig.reset();
    auto subsystemsEnd = std::chrono::high_resolution_clock::now();
    auto subsystemsDuration = std::chrono::duration_cast<std::chrono::milliseconds>(subsystemsEnd - subsystemsStart).count();
    LOG_INFO("Subsystems cleanup took {} ms", subsystemsDuration);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    LOG_INFO("Total shutdown took {} ms", totalDuration);
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

void EditorApplication::renderSelectionBox() {
    using namespace NovaEngine;

    // Only render if an entity is selected
    Entity* selectedEntity = m_state->getSelectedEntity();
    if (!selectedEntity) {
        return;
    }

    // Get transform and sprite components
    auto* transform = selectedEntity->getComponent<TransformComponent>();
    auto* sprite = selectedEntity->getComponent<SpriteComponent>();

    if (!transform || !sprite) {
        return;
    }

    // Calculate entity bounds
    Vec2f entityPos = transform->position;
    Vec2f scale = transform->scale;
    Vec2f spriteSize = sprite->size;
    Vec2f origin = transform->origin;

    // Calculate scaled size
    Vec2f scaledSize{spriteSize.x * scale.x, spriteSize.y * scale.y};

    // Calculate box position (top-left corner, accounting for origin)
    Vec2f boxPos{
        entityPos.x - origin.x * scale.x,
        entityPos.y - origin.y * scale.y
    };

    // Draw selection rectangle
    RectData rectData;
    rectData.position = boxPos;
    rectData.size = scaledSize;
    rectData.fillColor = Color{0, 0, 0, 0};        // Transparent fill
    rectData.outlineColor = Color{255, 255, 0, 255}; // Yellow outline
    rectData.outlineThickness = 2.0f / m_camera->getZoom(); // Keep constant thickness regardless of zoom
    rectData.rotation = transform->rotation;

    GRAPHICS().drawRectangle(rectData);

    // Also draw small handles at the corners for better visibility
    f32 handleSize = 8.0f / m_camera->getZoom();
    Color handleColor{255, 165, 0, 255}; // Orange

    // Top-left handle
    RectData handleTL;
    handleTL.position = boxPos;
    handleTL.size = Vec2f{handleSize, handleSize};
    handleTL.fillColor = handleColor;
    handleTL.outlineColor = Color{0, 0, 0, 255};
    handleTL.outlineThickness = 1.0f / m_camera->getZoom();
    GRAPHICS().drawRectangle(handleTL);

    // Top-right handle
    RectData handleTR;
    handleTR.position = Vec2f{boxPos.x + scaledSize.x - handleSize, boxPos.y};
    handleTR.size = Vec2f{handleSize, handleSize};
    handleTR.fillColor = handleColor;
    handleTR.outlineColor = Color{0, 0, 0, 255};
    handleTR.outlineThickness = 1.0f / m_camera->getZoom();
    GRAPHICS().drawRectangle(handleTR);

    // Bottom-left handle
    RectData handleBL;
    handleBL.position = Vec2f{boxPos.x, boxPos.y + scaledSize.y - handleSize};
    handleBL.size = Vec2f{handleSize, handleSize};
    handleBL.fillColor = handleColor;
    handleBL.outlineColor = Color{0, 0, 0, 255};
    handleBL.outlineThickness = 1.0f / m_camera->getZoom();
    GRAPHICS().drawRectangle(handleBL);

    // Bottom-right handle
    RectData handleBR;
    handleBR.position = Vec2f{boxPos.x + scaledSize.x - handleSize, boxPos.y + scaledSize.y - handleSize};
    handleBR.size = Vec2f{handleSize, handleSize};
    handleBR.fillColor = handleColor;
    handleBR.outlineColor = Color{0, 0, 0, 255};
    handleBR.outlineThickness = 1.0f / m_camera->getZoom();
    GRAPHICS().drawRectangle(handleBR);
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
    else if (input.type == InputEventType::MouseMoved) {
        // Handle dragging
        if (m_isDragging) {
            Vec2i screenPos{input.mouseMove.x, input.mouseMove.y};
            Vec2f worldPos = m_camera->screenToWorld(screenPos);
            updateDraggingEntity(worldPos);
        }
    }
    else if (input.type == InputEventType::MouseButtonReleased) {
        // Stop dragging
        if (input.mouseButton.button == MouseButton::Left) {
            stopDraggingEntity();
        }
    }
    // Note: Mouse wheel scrolling not supported in current InputEvent
}

void EditorApplication::handleKeyPress(const NovaEngine::InputEvent& input) {
    using namespace NovaEngine;

    // ESC: Exit placement mode or quit
    if (input.key.code == KeyCode::Escape) {
        if (m_isPlacementMode) {
            exitPlacementMode();
        } else {
            quit();
        }
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

    // Layer shortcuts (number keys 0-9 for layers 0-9)
    if (input.key.code >= KeyCode::Num0 && input.key.code <= KeyCode::Num9) {
        int layer = static_cast<int>(input.key.code) - static_cast<int>(KeyCode::Num0);

        // If Shift is held, use negative layer
        if (input.key.shift) {
            layer = -layer;
        }

        m_state->setCurrentLayer(layer);
        LOG_INFO("Current layer set to: {}", layer);
    }
}

void EditorApplication::handleMouseClick(const NovaEngine::InputEvent& input) {
    using namespace NovaEngine;

    Vec2i screenPos{input.mouseButton.x, input.mouseButton.y};
    Vec2f worldPos = m_camera->screenToWorld(screenPos);

    LOG_INFO("=== MOUSE CLICK ===");
    LOG_INFO("Screen: ({}, {})", screenPos.x, screenPos.y);
    LOG_INFO("World: ({}, {})", worldPos.x, worldPos.y);
    LOG_INFO("Camera pos: ({}, {}), zoom: {}",
             m_camera->getPosition().x, m_camera->getPosition().y, m_camera->getZoom());

    // Handle placement mode
    if (m_isPlacementMode) {
        placeEntity(worldPos);
        return;
    }

    // Handle entity selection and dragging
    if (input.mouseButton.button == MouseButton::Left) {
        if (m_currentScene) {
            // Check if clicking on an entity
            Entity* clickedEntity = nullptr;
            float closestDistance = std::numeric_limits<float>::max();

            // Iterate through all entities to find one under the mouse
            const auto& entities = m_currentScene->getEntityRegistry().getAllEntities();
            LOG_INFO("Checking {} entities for selection", entities.size());

            for (auto* entity : entities) {
                if (entity) {
                    auto* transform = entity->getComponent<TransformComponent>();
                    if (!transform) continue;

                    Vec2f entityPos = transform->position;
                    Vec2f scale = transform->scale;

                    // Get sprite size if available for accurate hit detection
                    auto* sprite = entity->getComponent<SpriteComponent>();
                    Vec2f spriteSize{100.0f, 100.0f}; // Default size
                    if (sprite && sprite->size.x > 0 && sprite->size.y > 0) {
                        spriteSize = sprite->size;
                    }

                    // Calculate actual bounds with scale applied
                    Vec2f scaledSize{spriteSize.x * scale.x, spriteSize.y * scale.y};
                    Vec2f halfSize{scaledSize.x * 0.5f, scaledSize.y * 0.5f};

                    LOG_DEBUG("Entity {}: pos({}, {}), size({}, {}), bounds({}, {})",
                             entity->getID(), entityPos.x, entityPos.y,
                             scaledSize.x, scaledSize.y, halfSize.x, halfSize.y);

                    // Check if click is within sprite bounds (AABB test)
                    if (worldPos.x >= entityPos.x - halfSize.x &&
                        worldPos.x <= entityPos.x + halfSize.x &&
                        worldPos.y >= entityPos.y - halfSize.y &&
                        worldPos.y <= entityPos.y + halfSize.y) {

                        LOG_INFO("Entity {} is under mouse!", entity->getID());

                        // Calculate distance to center for z-ordering
                        float distance = std::sqrt(
                            (worldPos.x - entityPos.x) * (worldPos.x - entityPos.x) +
                            (worldPos.y - entityPos.y) * (worldPos.y - entityPos.y)
                        );

                        if (distance < closestDistance) {
                            clickedEntity = entity;
                            closestDistance = distance;
                        }
                    }
                }
            }

            if (clickedEntity) {
                LOG_INFO("Selected entity: {}", clickedEntity->getID());
                selectEntity(clickedEntity);
                startDraggingEntity();
            } else {
                LOG_INFO("No entity under mouse - deselecting");
                // Clicked on empty space - deselect
                selectEntity(nullptr);
            }
        }
    }
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
        // Clear previous entity list when changing category
        m_availableEntities.clear();
        m_entityDialogPage = 0;
        // Show entity selection dialog
        showEntitySelectionDialog(value);
    }
    else if (action == "load_scene_by_index") {
        // Load scene by index from available scenes list
        try {
            size_t index = std::stoul(value);
            // Calculate actual scene index based on current page
            size_t actualIndex = (m_sceneDialogPage * SCENES_PER_PAGE) + index;

            if (actualIndex < m_availableScenes.size()) {
                std::string scenePath = m_editorConfig->getScenesPath() + "/" + m_availableScenes[actualIndex];
                LOG_INFO("Loading scene: {}", m_availableScenes[actualIndex]);

                // Close dialog and clear state
                m_uiManager.setGroupActive("scene_dialog", false);
                m_availableScenes.clear();
                m_sceneDialogPage = 0;

                // Load the scene
                loadScene(scenePath);
            } else {
                LOG_ERROR("Invalid scene index: {}", actualIndex);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse scene index: {}", e.what());
        }
    }
    else if (action == "scene_next_page") {
        // Go to next page of scenes
        size_t totalPages = (m_availableScenes.size() + SCENES_PER_PAGE - 1) / SCENES_PER_PAGE;
        if (m_sceneDialogPage < totalPages - 1) {
            m_sceneDialogPage++;
            showSceneSelectionDialog();
            LOG_INFO("Scene dialog: next page {}", m_sceneDialogPage + 1);
        }
    }
    else if (action == "scene_prev_page") {
        // Go to previous page of scenes
        if (m_sceneDialogPage > 0) {
            m_sceneDialogPage--;
            showSceneSelectionDialog();
            LOG_INFO("Scene dialog: previous page {}", m_sceneDialogPage + 1);
        }
    }
    else if (action == "close_scene_dialog") {
        // Close the scene selection dialog and clear state
        m_uiManager.setGroupActive("scene_dialog", false);
        m_availableScenes.clear();
        m_sceneDialogPage = 0;
        LOG_INFO("Scene selection dialog closed");
    }
    else if (action == "select_entity_by_index") {
        // Select entity by index from available entities list
        try {
            size_t index = std::stoul(value);
            // Calculate actual entity index based on current page
            size_t actualIndex = (m_entityDialogPage * ENTITIES_PER_PAGE) + index;

            if (actualIndex < m_availableEntities.size()) {
                std::string entityId = m_availableEntities[actualIndex];
                LOG_INFO("Selected entity: {}", entityId);

                // Close dialog and clear state
                m_uiManager.setGroupActive("entity_dialog", false);
                m_availableEntities.clear();
                m_entityDialogPage = 0;

                // Enter placement mode with selected entity
                enterPlacementMode(m_currentEntityCategory, entityId);
            } else {
                LOG_ERROR("Invalid entity index: {}", actualIndex);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse entity index: {}", e.what());
        }
    }
    else if (action == "entity_next_page") {
        // Go to next page of entities
        size_t totalPages = (m_availableEntities.size() + ENTITIES_PER_PAGE - 1) / ENTITIES_PER_PAGE;
        if (m_entityDialogPage < totalPages - 1) {
            m_entityDialogPage++;
            showEntitySelectionDialog(m_currentEntityCategory);
            LOG_INFO("Entity dialog: next page {}", m_entityDialogPage + 1);
        }
    }
    else if (action == "entity_prev_page") {
        // Go to previous page of entities
        if (m_entityDialogPage > 0) {
            m_entityDialogPage--;
            showEntitySelectionDialog(m_currentEntityCategory);
            LOG_INFO("Entity dialog: previous page {}", m_entityDialogPage + 1);
        }
    }
    else if (action == "close_entity_dialog") {
        // Close the entity selection dialog and clear state
        m_uiManager.setGroupActive("entity_dialog", false);
        m_availableEntities.clear();
        m_entityDialogPage = 0;
        LOG_INFO("Entity selection dialog closed");
    }
    else if (action == "set_layer") {
        // Set the current layer for entity placement
        try {
            i32 layer = std::stoi(value);
            m_state->setCurrentLayer(layer);
            LOG_INFO("Current layer set to: {}", layer);
            updateLayersPanel();
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse layer value: {}", e.what());
        }
    }
    else if (action == "select_entity_from_list") {
        // Select entity from the layers panel list
        try {
            size_t index = std::stoul(value);
            if (index < m_layerEntitiesList.size()) {
                Entity* entity = m_layerEntitiesList[index];
                LOG_INFO("Selected entity from list: ID {}", entity->getID());
                selectEntity(entity);
            } else {
                LOG_ERROR("Invalid entity list index: {}", index);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse entity index: {}", e.what());
        }
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

    // Unload previous scene from SceneManager if it exists
    if (m_currentScene) {
        LOG_INFO("Unloading previous scene from SceneManager...");
        m_sceneManager.unloadScene(sceneName);
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

            // Update layers panel to show entities in loaded scene
            updateLayersPanel();
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

    LOG_INFO("Opening scene selection dialog...");

    try {
        // Get available scenes (only once when opening dialog)
        if (m_availableScenes.empty()) {
            LOG_INFO("Scanning for available scenes...");
            m_availableScenes = getAvailableScenes();
            m_sceneDialogPage = 0;
        }

        if (m_availableScenes.empty()) {
            LOG_WARN("No scenes found in data/scenes/");
            return;
        }

        LOG_INFO("=== Available Scenes (Page {}/{}) ===",
                 m_sceneDialogPage + 1,
                 (m_availableScenes.size() + SCENES_PER_PAGE - 1) / SCENES_PER_PAGE);

        // Calculate page range
        size_t startIdx = m_sceneDialogPage * SCENES_PER_PAGE;
        size_t endIdx = std::min(startIdx + SCENES_PER_PAGE, m_availableScenes.size());

        LOG_INFO("Updating scene button visibility...");
        // Update button text and visibility for each scene slot
        for (size_t i = 0; i < SCENES_PER_PAGE; ++i) {
            std::string buttonID = "btn_scene_" + std::to_string(i);
            auto btnComponent = m_uiManager.getComponent(buttonID);

            if (btnComponent) {
                auto button = std::dynamic_pointer_cast<NovaEngine::Button>(btnComponent);
                if (button) {
                    size_t sceneIdx = startIdx + i;
                    if (sceneIdx < endIdx) {
                        // Show button with scene name
                        button->setText(m_availableScenes[sceneIdx]);
                        button->setVisible(true);
                        button->setActive(true);
                        LOG_INFO("  [{}] {}", sceneIdx, m_availableScenes[sceneIdx]);
                    } else {
                        // Hide unused button completely
                        button->setVisible(false);
                    }
                } else {
                    LOG_WARN("Button {} exists but is not a Button component", buttonID);
                }
            } else {
                LOG_WARN("Button component {} not found", buttonID);
            }
        }

        LOG_INFO("Updating prev/next button visibility...");
        // Update prev/next button visibility
        size_t totalPages = (m_availableScenes.size() + SCENES_PER_PAGE - 1) / SCENES_PER_PAGE;

        auto btnPrev = m_uiManager.getComponent("btn_scene_prev");
        if (btnPrev) {
            btnPrev->setVisible(m_sceneDialogPage > 0);
            LOG_INFO("Prev button visibility: {}", m_sceneDialogPage > 0);
        } else {
            LOG_WARN("btn_scene_prev not found");
        }

        auto btnNext = m_uiManager.getComponent("btn_scene_next");
        if (btnNext) {
            btnNext->setVisible(m_sceneDialogPage < totalPages - 1);
            LOG_INFO("Next button visibility: {}", m_sceneDialogPage < totalPages - 1);
        } else {
            LOG_WARN("btn_scene_next not found");
        }

        // Show the dialog
        LOG_INFO("Activating scene_dialog group...");
        m_uiManager.setGroupActive("scene_dialog", true);
        LOG_INFO("Scene selection dialog opened: page {}/{}, total {} scenes",
                 m_sceneDialogPage + 1, totalPages, m_availableScenes.size());
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in showSceneSelectionDialog: {}", e.what());
    }
}

// ===== ENTITY MANAGEMENT =====

std::vector<std::string> EditorApplication::getAvailableEntities(const std::string& category) const {
    using json = nlohmann::json;
    std::vector<std::string> entities;

    std::string filePath;
    std::string jsonKey;

    // Map category to file and JSON key
    if (category == "sprites") {
        filePath = m_editorConfig->getDefinitionsPath() + "/Sprites.json";
        jsonKey = "sprites";
    } else if (category == "lights") {
        filePath = m_editorConfig->getDefinitionsPath() + "/Lights.json";
        jsonKey = "lights";
    } else if (category == "npcs") {
        filePath = m_editorConfig->getDefinitionsPath() + "/NPCs.json";
        jsonKey = "npcs";
    } else {
        LOG_WARN("Unknown entity category: {}", category);
        return entities;
    }

    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            LOG_WARN("Definition file not found: {}", filePath);
            return entities;
        }

        json data = json::parse(file);
        if (data.contains(jsonKey) && data[jsonKey].is_array()) {
            for (const auto& entity : data[jsonKey]) {
                if (entity.contains("id") && entity["id"].is_string()) {
                    entities.push_back(entity["id"].get<std::string>());
                }
            }
        }

        file.close();
    } catch (const std::exception& e) {
        LOG_ERROR("Error reading definitions from {}: {}", filePath, e.what());
    }

    std::sort(entities.begin(), entities.end());
    LOG_INFO("Found {} {}(s)", entities.size(), category);
    return entities;
}

void EditorApplication::showEntitySelectionDialog(const std::string& category) {
    using namespace NovaEngine;

    LOG_INFO("Opening entity selection dialog for category: {}", category);
    m_currentEntityCategory = category;

    try {
        // Get available entities (only once when opening dialog)
        if (m_availableEntities.empty()) {
            LOG_INFO("Scanning for available entities...");
            m_availableEntities = getAvailableEntities(category);
            m_entityDialogPage = 0;
        }

        if (m_availableEntities.empty()) {
            LOG_WARN("No {} found", category);
            return;
        }

        LOG_INFO("=== Available {} (Page {}/{}) ===",
                 category,
                 m_entityDialogPage + 1,
                 (m_availableEntities.size() + ENTITIES_PER_PAGE - 1) / ENTITIES_PER_PAGE);

        // Calculate page range
        size_t startIdx = m_entityDialogPage * ENTITIES_PER_PAGE;
        size_t endIdx = std::min(startIdx + ENTITIES_PER_PAGE, m_availableEntities.size());

        LOG_INFO("Updating entity button visibility...");
        // Update button text and visibility for each entity slot
        for (size_t i = 0; i < ENTITIES_PER_PAGE; ++i) {
            std::string buttonID = "btn_entity_" + std::to_string(i);
            auto btnComponent = m_uiManager.getComponent(buttonID);

            if (btnComponent) {
                auto button = std::dynamic_pointer_cast<NovaEngine::Button>(btnComponent);
                if (button) {
                    size_t entityIdx = startIdx + i;
                    if (entityIdx < endIdx) {
                        // Show button with entity name
                        button->setText(m_availableEntities[entityIdx]);
                        button->setVisible(true);
                        button->setActive(true);
                        LOG_INFO("  [{}] {}", entityIdx, m_availableEntities[entityIdx]);
                    } else {
                        // Hide unused button completely
                        button->setVisible(false);
                    }
                } else {
                    LOG_WARN("Button {} exists but is not a Button component", buttonID);
                }
            } else {
                LOG_WARN("Button component {} not found", buttonID);
            }
        }

        LOG_INFO("Updating prev/next button visibility...");
        // Update prev/next button visibility
        size_t totalPages = (m_availableEntities.size() + ENTITIES_PER_PAGE - 1) / ENTITIES_PER_PAGE;

        auto btnPrev = m_uiManager.getComponent("btn_entity_prev");
        if (btnPrev) {
            btnPrev->setVisible(m_entityDialogPage > 0);
            LOG_INFO("Prev button visibility: {}", m_entityDialogPage > 0);
        } else {
            LOG_WARN("btn_entity_prev not found");
        }

        auto btnNext = m_uiManager.getComponent("btn_entity_next");
        if (btnNext) {
            btnNext->setVisible(m_entityDialogPage < totalPages - 1);
            LOG_INFO("Next button visibility: {}", m_entityDialogPage < totalPages - 1);
        } else {
            LOG_WARN("btn_entity_next not found");
        }

        // Update dialog title with category name
        auto titleText = m_uiManager.getComponent("entity_dialog_title");
        if (titleText) {
            auto text = std::dynamic_pointer_cast<NovaEngine::Text>(titleText);
            if (text) {
                try {
                    std::string capitalizedCategory = category;
                    if (!capitalizedCategory.empty()) {
                        capitalizedCategory[0] = std::toupper(capitalizedCategory[0]);
                    }
                    text->setString("Select " + capitalizedCategory + " to Place");
                } catch (const std::exception& e) {
                    LOG_ERROR("Failed to update entity dialog title: {}", e.what());
                }
            }
        }

        // Show the dialog
        LOG_INFO("Activating entity_dialog group...");
        m_uiManager.setGroupActive("entity_dialog", true);
        LOG_INFO("Entity selection dialog opened: page {}/{}, total {} entities",
                 m_entityDialogPage + 1, totalPages, m_availableEntities.size());
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in showEntitySelectionDialog: {}", e.what());
    }
}

void EditorApplication::enterPlacementMode(const std::string& entityType, const std::string& entityId) {
    LOG_INFO("Entering placement mode: {} - {}", entityType, entityId);

    m_placementEntityType = entityType;
    m_placementEntityId = entityId;
    m_isPlacementMode = true;

    // Update editor state
    m_state->setMode(EditorState::Mode::Place);
    m_state->setPlacementType(entityType + ":" + entityId);

    LOG_INFO("Placement mode active. Click to place entity.");
}

void EditorApplication::exitPlacementMode() {
    if (m_isPlacementMode) {
        LOG_INFO("Exiting placement mode");
        m_isPlacementMode = false;
        m_placementEntityType.clear();
        m_placementEntityId.clear();
        m_state->setMode(EditorState::Mode::Select);
        m_state->setPlacementType("");
    }
}

void EditorApplication::placeEntity(const NovaEngine::Vec2f& position) {
    using namespace NovaEngine;

    if (!m_isPlacementMode || !m_currentScene) {
        LOG_WARN("Cannot place entity: not in placement mode or no scene loaded");
        return;
    }

    LOG_INFO("=== PLACING ENTITY ===");
    LOG_INFO("Position requested: ({}, {})", position.x, position.y);
    LOG_INFO("Camera position: ({}, {})", m_camera->getPosition().x, m_camera->getPosition().y);
    LOG_INFO("Camera zoom: {}", m_camera->getZoom());
    LOG_INFO("Placement type: {}, ID: {}", m_placementEntityType, m_placementEntityId);
    LOG_INFO("Note: Sprites will be centered at cursor position (origin set to center)");

    try {
        // Create entity based on type
        if (m_placementEntityType == "sprites") {
            // Create entity
            Entity* entity = m_currentScene->getEntityRegistry().createEntity();

            // Initialize sprite from definition first to get all properties
            const auto* definition = m_sceneManager.getDefinitionManager().getSpriteDefinition(m_placementEntityId);

            // Add transform component
            auto transform = std::make_unique<TransformComponent>();
            transform->position = position;

            // Apply scale from definition if present
            if (definition && definition->contains("scale")) {
                float scale = (*definition)["scale"].get<f32>();
                transform->scale = Vec2f{scale, scale};
            }

            // Will set origin after we know sprite size
            Vec2f spriteSize{0.0f, 0.0f};

            // Add sprite component
            auto sprite = std::make_unique<SpriteComponent>();

            // Initialize sprite properties from definition
            if (definition) {
                // Load texture and set properties from definition
                if (definition->contains("texture")) {
                    std::string texturePath = (*definition)["texture"].get<std::string>();
                    sprite->textureHandle = RESOURCES().loadTexture(texturePath);
                    sprite->textureID = m_placementEntityId;
                }

                // Set size from definition
                if (definition->contains("width") && definition->contains("height")) {
                    sprite->size = Vec2f{
                        (*definition)["width"].get<f32>(),
                        (*definition)["height"].get<f32>()
                    };
                    spriteSize = sprite->size;
                } else if (definition->contains("size")) {
                    auto& size = (*definition)["size"];
                    sprite->size = Vec2f{size[0].get<f32>(), size[1].get<f32>()};
                    spriteSize = sprite->size;
                }

                // Set origin from definition, or default to center
                if (definition->contains("origin")) {
                    auto& origin = (*definition)["origin"];
                    transform->origin = Vec2f{origin[0].get<f32>(), origin[1].get<f32>()};
                    LOG_INFO("Using origin from definition: ({}, {})", transform->origin.x, transform->origin.y);
                } else {
                    // Default to center of sprite for better placement UX
                    transform->origin = Vec2f{spriteSize.x * 0.5f, spriteSize.y * 0.5f};
                    LOG_INFO("Using centered origin: ({}, {}) (sprite size: {}, {})",
                             transform->origin.x, transform->origin.y, spriteSize.x, spriteSize.y);
                }
            }

            // Apply current layer as zOrder (overrides definition)
            sprite->zOrder = m_state->getCurrentLayer();

            entity->addComponent(std::move(transform));
            entity->addComponent(std::move(sprite));

            LOG_INFO("Sprite entity created successfully (ID: {})", entity->getID());
            m_state->setSceneModified(true);
            updateLayersPanel();

            // Exit placement mode after placing entity
            exitPlacementMode();
        }
        else if (m_placementEntityType == "lights") {
            // Create entity
            Entity* entity = m_currentScene->getEntityRegistry().createEntity();

            // Add transform component
            auto transform = std::make_unique<TransformComponent>();
            transform->position = position;
            entity->addComponent(std::move(transform));

            // Add light component
            auto light = std::make_unique<LightComponent>();

            // Initialize light from definition
            const auto* definition = m_sceneManager.getDefinitionManager().getLightDefinition(m_placementEntityId);
            if (definition) {
                // Load light properties from definition
                if (definition->contains("type")) {
                    std::string typeStr = (*definition)["type"].get<std::string>();
                    if (typeStr == "point") light->type = LightComponent::LightType::Point;
                    else if (typeStr == "directional") light->type = LightComponent::LightType::Directional;
                    else if (typeStr == "spot") light->type = LightComponent::LightType::Spot;
                }

                if (definition->contains("color")) {
                    auto& color = (*definition)["color"];
                    light->color = Color{
                        static_cast<u8>(color[0].get<int>()),
                        static_cast<u8>(color[1].get<int>()),
                        static_cast<u8>(color[2].get<int>()),
                        static_cast<u8>(color[3].get<int>())
                    };
                }

                if (definition->contains("radius")) {
                    light->radius = (*definition)["radius"].get<f32>();
                }

                if (definition->contains("intensity")) {
                    light->intensity = (*definition)["intensity"].get<f32>();
                }

                if (definition->contains("castShadows")) {
                    light->castShadows = (*definition)["castShadows"].get<bool>();
                }
            }

            entity->addComponent(std::move(light));

            LOG_INFO("Light entity created successfully (ID: {})", entity->getID());
            m_state->setSceneModified(true);
            updateLayersPanel();

            // Exit placement mode after placing entity
            exitPlacementMode();
        }
        else if (m_placementEntityType == "npcs") {
            // Create entity
            Entity* entity = m_currentScene->getEntityRegistry().createEntity();

            // Load sprite definition to get all properties including scale
            const auto* spriteDef = m_sceneManager.getDefinitionManager().getSpriteDefinition(m_placementEntityId);

            // Add transform component
            auto transform = std::make_unique<TransformComponent>();
            transform->position = position;

            // Apply scale from definition if present
            if (spriteDef && spriteDef->contains("scale")) {
                float scale = (*spriteDef)["scale"].get<f32>();
                transform->scale = Vec2f{scale, scale};
            }

            // Will set origin after we know sprite size
            Vec2f spriteSize{0.0f, 0.0f};

            // Add tag component for NPC identification
            auto tag = std::make_unique<TagComponent>();
            tag->tag = "npc";
            entity->addComponent(std::move(tag));

            // Add sprite component for visual representation
            auto sprite = std::make_unique<SpriteComponent>();
            if (spriteDef) {
                if (spriteDef->contains("texture")) {
                    std::string texturePath = (*spriteDef)["texture"].get<std::string>();
                    sprite->textureHandle = RESOURCES().loadTexture(texturePath);
                    sprite->textureID = m_placementEntityId;
                }
                if (spriteDef->contains("width") && spriteDef->contains("height")) {
                    sprite->size = Vec2f{
                        (*spriteDef)["width"].get<f32>(),
                        (*spriteDef)["height"].get<f32>()
                    };
                    spriteSize = sprite->size;
                } else if (spriteDef->contains("size")) {
                    auto& size = (*spriteDef)["size"];
                    sprite->size = Vec2f{size[0].get<f32>(), size[1].get<f32>()};
                    spriteSize = sprite->size;
                }

                // Set origin from definition, or default to center
                if (spriteDef->contains("origin")) {
                    auto& origin = (*spriteDef)["origin"];
                    transform->origin = Vec2f{origin[0].get<f32>(), origin[1].get<f32>()};
                    LOG_INFO("Using origin from definition: ({}, {})", transform->origin.x, transform->origin.y);
                } else {
                    // Default to center of sprite for better placement UX
                    transform->origin = Vec2f{spriteSize.x * 0.5f, spriteSize.y * 0.5f};
                    LOG_INFO("Using centered origin: ({}, {}) (sprite size: {}, {})",
                             transform->origin.x, transform->origin.y, spriteSize.x, spriteSize.y);
                }
            }

            // Apply current layer as zOrder (overrides definition)
            sprite->zOrder = m_state->getCurrentLayer();

            entity->addComponent(std::move(transform));
            entity->addComponent(std::move(sprite));

            LOG_INFO("NPC entity created successfully (ID: {})", entity->getID());
            m_state->setSceneModified(true);
            updateLayersPanel();

            // Exit placement mode after placing entity
            exitPlacementMode();
        }

    } catch (const std::exception& e) {
        LOG_ERROR("Exception while placing entity: {}", e.what());
    }
}

void EditorApplication::selectEntity(NovaEngine::Entity* entity) {
    if (entity) {
        LOG_INFO("Entity selected: ID {}", entity->getID());
        m_state->setSelectedEntity(entity);
    } else {
        LOG_INFO("Entity deselected");
        m_state->clearSelection();
    }
}

void EditorApplication::startDraggingEntity() {
    using namespace NovaEngine;

    if (!m_state->hasSelection() || !m_currentScene) {
        return;
    }

    auto* entity = m_state->getSelectedEntity();
    if (entity) {
        auto* transform = entity->getComponent<TransformComponent>();
        if (!transform) return;

        m_draggedEntity = entity;
        m_isDragging = true;

        // Calculate offset between mouse and entity position
        // Note: We get the world position from the last mouse event
        // For now, set offset to zero - it will be calculated on first move
        m_dragOffset = Vec2f{0.0f, 0.0f};

        LOG_INFO("Started dragging entity ID {}", entity->getID());
    }
}

void EditorApplication::updateDraggingEntity(const NovaEngine::Vec2f& worldPos) {
    using namespace NovaEngine;

    if (m_isDragging && m_draggedEntity) {
        auto* transform = m_draggedEntity->getComponent<TransformComponent>();
        if (!transform) {
            stopDraggingEntity();
            return;
        }

        // On first update, calculate offset
        if (m_dragOffset.x == 0.0f && m_dragOffset.y == 0.0f) {
            m_dragOffset = transform->position - worldPos;
        }

        Vec2f newPos = worldPos + m_dragOffset;
        transform->position = newPos;
        m_state->setSceneModified(true);
    }
}

void EditorApplication::stopDraggingEntity() {
    if (m_isDragging) {
        LOG_INFO("Stopped dragging entity");
        m_isDragging = false;
        m_draggedEntity = nullptr;
        m_dragOffset = NovaEngine::Vec2f{0.0f, 0.0f};
    }
}

void EditorApplication::updateLayersPanel() {
    using namespace NovaEngine;

    if (!m_currentScene) {
        return;
    }

    // Count entities per layer and collect entities by layer
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
    struct LayerButton {
        std::string buttonID;
        i32 layer;
    };

    std::vector<LayerButton> layerButtons = {
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

    i32 currentLayer = m_state->getCurrentLayer();

    for (const auto& lb : layerButtons) {
        auto btnComponent = m_uiManager.getComponent(lb.buttonID);
        if (btnComponent) {
            auto button = std::dynamic_pointer_cast<Button>(btnComponent);
            if (button) {
                int count = layerCounts[lb.layer];
                std::string text = "Layer " + std::to_string(lb.layer) + ": " + std::to_string(count);

                // Add indicator for current layer
                if (lb.layer == currentLayer) {
                    text = "> " + text + " <";
                }

                button->setText(text);
            }
        }
    }

    // Update entity list for current layer
    m_layerEntitiesList.clear();
    if (entitiesByLayer.count(currentLayer) > 0) {
        m_layerEntitiesList = entitiesByLayer[currentLayer];
    }

    // Update entity list buttons
    for (size_t i = 0; i < 10; ++i) {
        std::string btnID = "btn_layer_entity_" + std::to_string(i);
        auto btnComponent = m_uiManager.getComponent(btnID);
        if (btnComponent) {
            auto button = std::dynamic_pointer_cast<Button>(btnComponent);
            if (button) {
                if (i < m_layerEntitiesList.size()) {
                    Entity* entity = m_layerEntitiesList[i];

                    // Try to get a meaningful name for the entity
                    std::string entityName = "Entity #" + std::to_string(entity->getID());

                    // If it has a sprite, show the sprite ID
                    auto* sprite = entity->getComponent<SpriteComponent>();
                    if (sprite && !sprite->textureID.empty()) {
                        entityName = sprite->textureID + " #" + std::to_string(entity->getID());
                    }

                    button->setText(entityName);
                    button->setVisible(true);
                    button->setActive(true);
                } else {
                    button->setVisible(false);
                    button->setActive(false);
                }
            }
        }
    }

    LOG_DEBUG("Layers panel updated - {} entities on layer {}", m_layerEntitiesList.size(), currentLayer);
}

} // namespace NovaEditor
