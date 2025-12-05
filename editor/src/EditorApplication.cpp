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
    , m_lightingSystem(std::make_unique<NovaEngine::LightingSystem>())
    , m_dynamicLightingEffect(nullptr)
    , m_bloomEffect(nullptr)
    , m_colorGradingEffect(nullptr)
    , m_currentScene(nullptr)
    , m_sceneDialogPage(0)
    , m_entityDialogPage(0)
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

    // Initialize post-processing pipeline
    LOG_INFO("Initializing post-processing pipeline...");
    try {
        m_postProcessPipeline = std::make_unique<NovaEngine::PostProcessPipeline>(&GRAPHICS());
        LOG_INFO("PostProcessPipeline created successfully");

        if (!m_postProcessPipeline->initialize(m_config.windowWidth, m_config.windowHeight)) {
            LOG_WARN("Failed to initialize PostProcessPipeline - post-processing will be disabled");
            m_postProcessPipeline.reset();
        } else {
            LOG_INFO("PostProcessPipeline initialized successfully");

            // Add effects in order
            LOG_INFO("Adding DynamicLightingEffect...");
            m_dynamicLightingEffect = m_postProcessPipeline->addEffect<NovaEngine::DynamicLightingEffect>();

            LOG_INFO("Adding BloomEffect...");
            m_bloomEffect = m_postProcessPipeline->addEffect<NovaEngine::BloomEffect>();

            LOG_INFO("Adding ColorGradingEffect...");
            m_colorGradingEffect = m_postProcessPipeline->addEffect<NovaEngine::ColorGradingEffect>();

            // Set default values
            if (m_dynamicLightingEffect) {
                m_dynamicLightingEffect->setAmbientDarkness(0.5f);
                m_dynamicLightingEffect->setTimeOfDay(0.5f); // Noon by default
                m_lightingSystem->setLightingEffect(m_dynamicLightingEffect);
                LOG_INFO("Dynamic lighting effect initialized");
            } else {
                LOG_WARN("DynamicLightingEffect is null");
            }

            if (m_bloomEffect) {
                m_bloomEffect->setIntensity(0.4f);
                LOG_INFO("Bloom effect initialized");
            } else {
                LOG_WARN("BloomEffect is null");
            }

            if (m_colorGradingEffect) {
                m_colorGradingEffect->setSaturation(1.3f);
                m_colorGradingEffect->setContrast(1.0f);
                m_colorGradingEffect->setBrightness(0.0f);
                LOG_INFO("Color grading effect initialized");
            } else {
                LOG_WARN("ColorGradingEffect is null");
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during post-processing initialization: {}", e.what());
        LOG_WARN("Post-processing will be disabled");
        m_postProcessPipeline.reset();
        m_dynamicLightingEffect = nullptr;
        m_bloomEffect = nullptr;
        m_colorGradingEffect = nullptr;
    }

    // Create a default empty scene
    m_currentScene = new NovaEngine::Scene("EditorScene");

    // Initialize controllers (NEW ARCHITECTURE!)
    LOG_INFO("Initializing controllers...");

    // Initialize PanelManager
    m_panelManager = std::make_unique<EditorPanelManager>(m_uiManager, m_state.get(), &m_sceneManager);
    m_panelManager->initialize(m_lightingSystem.get(), m_dynamicLightingEffect, m_bloomEffect, m_colorGradingEffect, m_currentScene);
    LOG_INFO("EditorPanelManager initialized");

    // Initialize EntityEditorController
    m_entityController = std::make_unique<EntityEditorController>(
        m_state.get(),
        &m_sceneManager,
        [this]() { if (m_panelManager) m_panelManager->updateLayersPanel(m_currentScene); }
    );
    m_entityController->loadNPCDefinitions();
    LOG_INFO("EntityEditorController initialized");

    // Initialize SceneEditorController
    m_sceneController = std::make_unique<SceneEditorController>(
        m_state.get(),
        &m_sceneManager,
        [this](NovaEngine::Scene* scene) {
            // Update UI and camera after scene load (m_currentScene is managed by the caller)
            if (m_panelManager) m_panelManager->updateLayersPanel(scene);
            if (m_camera) {
                m_camera->focusOn(NovaEngine::Vec2f{0.0f, 0.0f});
                m_camera->setZoom(1.0f);
            }
        }
    );
    LOG_INFO("SceneEditorController initialized");

    // Initialize EditorInputHandler with callbacks
    EditorInputHandler::InputCallbacks callbacks;
    callbacks.onGridToggle = [this]() {
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
    };
    callbacks.onLayersPanelToggle = [this]() {
        if (m_panelManager) m_panelManager->handleAction("toggle_layers", "");
    };
    callbacks.onPropertiesPanelToggle = [this]() {
        if (m_panelManager) m_panelManager->handleAction("show_properties", "");
    };
    callbacks.onLayerChange = [this](NovaEngine::i32 layer) {
        if (m_panelManager) m_panelManager->handleAction("set_layer", std::to_string(layer));
    };
    callbacks.onEntityPlacement = [this](const NovaEngine::Vec2f& pos) {
        if (m_entityController) m_entityController->placeEntity(pos, m_currentScene);
    };
    callbacks.onEntitySelection = [this](const NovaEngine::Vec2f& pos) {
        selectEntityAtPosition(pos);
    };
    callbacks.onEntityDragStart = [this](const NovaEngine::Vec2f& pos) {
        if (m_entityController) m_entityController->startDragging(pos);
    };
    callbacks.onEntityDragUpdate = [this](const NovaEngine::Vec2f& pos) {
        if (m_entityController) m_entityController->updateDragging(pos);
    };
    callbacks.onEntityDragStop = [this]() {
        if (m_entityController) m_entityController->stopDragging();
    };

    m_inputHandler = std::make_unique<EditorInputHandler>(m_state.get(), m_camera.get(), callbacks);
    LOG_INFO("EditorInputHandler initialized");

    LOG_INFO("All controllers initialized successfully");

    printControls();

    auto initEnd = std::chrono::high_resolution_clock::now();
    LOG_INFO("Total initialization took {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(initEnd - initStart).count());
    LOG_INFO("EditorApplication initialized successfully");
    return true;
}

void EditorApplication::onUpdate(float deltaTime) {
    // Update camera (via InputHandler)
    if (m_inputHandler) {
        m_inputHandler->updateCamera(deltaTime);
    }

    // Update camera viewport
    if (m_camera) {
        m_camera->update(deltaTime);
    }

    // Update UI
    m_uiManager.update(deltaTime);
    m_uiManager_editor->update(deltaTime);

    // Update current scene
    if (m_currentScene) {
        m_currentScene->update(deltaTime);

        // Update lighting system with scene entities
        if (m_lightingSystem && m_dynamicLightingEffect) {
            m_dynamicLightingEffect->setCamera(m_camera->getPosition(), VIEWPORT().getViewSize());
            m_lightingSystem->update(deltaTime, m_currentScene->getEntityRegistry());
        }
    }
}

void EditorApplication::onRender() {
    // Begin post-processing (renders scene to texture)
    if (m_postProcessPipeline) {
        m_postProcessPipeline->beginSceneRender();
    }

    // Render scene with camera view
    renderScene();

    // Render grid
    if (m_state->isGridEnabled()) {
        renderGrid();
    }

    // Render selection box around selected entity
    renderSelectionBox();

    // Apply post-processing effects
    if (m_postProcessPipeline) {
        m_postProcessPipeline->endSceneRender(1.0f/60.0f);
    }

    // Render UI (in screen space, after post-processing)
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

    GRAPHICS().drawRect(rectData);

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
    GRAPHICS().drawRect(handleTL);

    // Top-right handle
    RectData handleTR;
    handleTR.position = Vec2f{boxPos.x + scaledSize.x - handleSize, boxPos.y};
    handleTR.size = Vec2f{handleSize, handleSize};
    handleTR.fillColor = handleColor;
    handleTR.outlineColor = Color{0, 0, 0, 255};
    handleTR.outlineThickness = 1.0f / m_camera->getZoom();
    GRAPHICS().drawRect(handleTR);

    // Bottom-left handle
    RectData handleBL;
    handleBL.position = Vec2f{boxPos.x, boxPos.y + scaledSize.y - handleSize};
    handleBL.size = Vec2f{handleSize, handleSize};
    handleBL.fillColor = handleColor;
    handleBL.outlineColor = Color{0, 0, 0, 255};
    handleBL.outlineThickness = 1.0f / m_camera->getZoom();
    GRAPHICS().drawRect(handleBL);

    // Bottom-right handle
    RectData handleBR;
    handleBR.position = Vec2f{boxPos.x + scaledSize.x - handleSize, boxPos.y + scaledSize.y - handleSize};
    handleBR.size = Vec2f{handleSize, handleSize};
    handleBR.fillColor = handleColor;
    handleBR.outlineColor = Color{0, 0, 0, 255};
    handleBR.outlineThickness = 1.0f / m_camera->getZoom();
    GRAPHICS().drawRect(handleBR);
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

    if (!m_inputHandler) return;

    // ESC: Exit placement mode or quit
    if (input.type == InputEventType::KeyPressed && input.key.code == KeyCode::Escape) {
        if (m_entityController && m_entityController->isPlacementMode()) {
            m_entityController->exitPlacementMode();
        } else {
            quit();
        }
        return;
    }

    // Ctrl+N: New scene
    if (input.type == InputEventType::KeyPressed && input.key.code == KeyCode::N && input.key.control) {
        if (m_sceneController) {
            NovaEngine::Scene* newScene = m_sceneController->createNewScene();
            if (newScene) {
                if (m_currentScene) delete m_currentScene;
                m_currentScene = newScene;
            }
        }
        return;
    }

    // Ctrl+O: Load scene
    if (input.type == InputEventType::KeyPressed && input.key.code == KeyCode::O && input.key.control) {
        showSceneSelectionDialog();
        return;
    }

    // Ctrl+S: Save scene
    if (input.type == InputEventType::KeyPressed && input.key.code == KeyCode::S && input.key.control) {
        if (m_sceneController) {
            m_sceneController->saveScene(m_currentScene);
        }
        return;
    }

    // Delegate to InputHandler
    if (input.type == InputEventType::KeyPressed) {
        m_inputHandler->handleKeyPress(input);
    }
    else if (input.type == InputEventType::MouseButtonPressed) {
        // Only handle editor clicks if not over UI
        Vec2i mousePos{input.mouseButton.x, input.mouseButton.y};
        if (!m_uiManager.isMouseOverUI(mousePos)) {
            bool isPlacementMode = m_entityController && m_entityController->isPlacementMode();
            m_inputHandler->handleMouseClick(input, isPlacementMode);
        }
    }
    else if (input.type == InputEventType::MouseMoved) {
        bool isDragging = m_entityController && m_entityController->isDragging();
        m_inputHandler->handleMouseMove(input, isDragging);
    }
    else if (input.type == InputEventType::MouseButtonReleased) {
        bool isDragging = m_entityController && m_entityController->isDragging();
        m_inputHandler->handleMouseRelease(input, isDragging);
    }
}


void EditorApplication::onUIAction(const std::string& action, const std::string& value) {
    using namespace NovaEngine;

    LOG_INFO("=== UI ACTION === '{}' with value '{}'", action, value);

    // Route scene actions to SceneController
    if (action == "scene_new") {
        if (m_sceneController) {
            NovaEngine::Scene* newScene = m_sceneController->createNewScene();
            if (newScene) {
                if (m_currentScene) delete m_currentScene;
                m_currentScene = newScene;
            }
        }
        return;
    }
    if (action == "scene_load") {
        showSceneSelectionDialog();
        return;
    }
    if (action == "scene_save") {
        if (m_sceneController) {
            m_sceneController->saveScene(m_currentScene);
        }
        return;
    }

    // Route panel actions to PanelManager
    if (m_panelManager && m_panelManager->handleAction(action, value)) {
        return;
    }

    // Handle undo/redo (not yet implemented)
    if (action == "edit_undo") {
        LOG_INFO("Undo requested (not yet implemented)");
        return;
    }
    if (action == "edit_redo") {
        LOG_INFO("Redo requested (not yet implemented)");
        return;
    }

    // Handle grid toggle
    if (action == "toggle_grid") {
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
        return;
    }

    // Handle entity palette category selection
    if (action == "palette_category") {
        m_uiManager_editor->updateEntityPalette(value);
        // Clear previous entity list when changing category
        m_availableEntities.clear();
        m_entityDialogPage = 0;
        // Show entity selection dialog
        showEntitySelectionDialog(value);
        return;
    }

    // Handle scene selection dialog
    if (action == "load_scene_by_index") {
        try {
            size_t index = std::stoul(value);
            size_t actualIndex = (m_sceneDialogPage * SCENES_PER_PAGE) + index;

            if (actualIndex < m_availableScenes.size()) {
                std::string scenePath = m_editorConfig->getScenesPath() + "/" + m_availableScenes[actualIndex];
                LOG_INFO("Loading scene: {}", m_availableScenes[actualIndex]);

                // Close dialog and clear state
                m_uiManager.setGroupActive("scene_dialog", false);
                m_availableScenes.clear();
                m_sceneDialogPage = 0;

                // Load the scene via SceneController
                if (m_sceneController) {
                    NovaEngine::Scene* loadedScene = m_sceneController->loadScene(scenePath);
                    if (loadedScene) {
                        if (m_currentScene) delete m_currentScene;
                        m_currentScene = loadedScene;
                    }
                }
            } else {
                LOG_ERROR("Invalid scene index: {}", actualIndex);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse scene index: {}", e.what());
        }
        return;
    }
    if (action == "scene_next_page") {
        size_t totalPages = (m_availableScenes.size() + SCENES_PER_PAGE - 1) / SCENES_PER_PAGE;
        if (m_sceneDialogPage < totalPages - 1) {
            m_sceneDialogPage++;
            showSceneSelectionDialog();
            LOG_INFO("Scene dialog: next page {}", m_sceneDialogPage + 1);
        }
        return;
    }
    if (action == "scene_prev_page") {
        if (m_sceneDialogPage > 0) {
            m_sceneDialogPage--;
            showSceneSelectionDialog();
            LOG_INFO("Scene dialog: previous page {}", m_sceneDialogPage + 1);
        }
        return;
    }
    if (action == "close_scene_dialog") {
        m_uiManager.setGroupActive("scene_dialog", false);
        m_availableScenes.clear();
        m_sceneDialogPage = 0;
        LOG_INFO("Scene selection dialog closed");
        return;
    }

    // Handle entity selection dialog (routes to EntityController)
    if (action == "select_entity_by_index") {
        try {
            size_t index = std::stoul(value);
            size_t actualIndex = (m_entityDialogPage * ENTITIES_PER_PAGE) + index;

            if (actualIndex < m_availableEntities.size()) {
                std::string entityId = m_availableEntities[actualIndex];
                LOG_INFO("Selected entity: {}", entityId);

                // Close dialog and clear state
                m_uiManager.setGroupActive("entity_dialog", false);
                m_availableEntities.clear();
                m_entityDialogPage = 0;

                // Enter placement mode via EntityController
                if (m_entityController) {
                    m_entityController->enterPlacementMode(m_currentEntityCategory, entityId);
                }
            } else {
                LOG_ERROR("Invalid entity index: {}", actualIndex);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse entity index: {}", e.what());
        }
        return;
    }
    if (action == "entity_next_page") {
        size_t totalPages = (m_availableEntities.size() + ENTITIES_PER_PAGE - 1) / ENTITIES_PER_PAGE;
        if (m_entityDialogPage < totalPages - 1) {
            m_entityDialogPage++;
            showEntitySelectionDialog(m_currentEntityCategory);
            LOG_INFO("Entity dialog: next page {}", m_entityDialogPage + 1);
        }
        return;
    }
    if (action == "entity_prev_page") {
        if (m_entityDialogPage > 0) {
            m_entityDialogPage--;
            showEntitySelectionDialog(m_currentEntityCategory);
            LOG_INFO("Entity dialog: previous page {}", m_entityDialogPage + 1);
        }
        return;
    }
    if (action == "close_entity_dialog") {
        m_uiManager.setGroupActive("entity_dialog", false);
        m_availableEntities.clear();
        m_entityDialogPage = 0;
        LOG_INFO("Entity selection dialog closed");
        return;
    }

    // Handle entity selection from layers panel
    if (action == "select_entity_from_list") {
        if (m_panelManager) {
            try {
                size_t index = std::stoul(value);
                Entity* entity = m_panelManager->getEntityFromLayersList(index);
                if (entity) {
                    LOG_INFO("Selected entity from list: ID {}", entity->getID());
                    if (m_entityController) {
                        m_entityController->selectEntity(entity);
                    }
                } else {
                    LOG_ERROR("Invalid entity list index: {}", index);
                }
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to parse entity index: {}", e.what());
            }
        }
        return;
    }

    LOG_WARN("Unknown action: '{}'", action);
}

// Helper method to select entity at world position
void EditorApplication::selectEntityAtPosition(const NovaEngine::Vec2f& worldPos) {
    using namespace NovaEngine;

    if (!m_currentScene) {
        return;
    }

    // Find entity under the mouse
    Entity* clickedEntity = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    const auto& entities = m_currentScene->getEntityRegistry().getAllEntities();
    LOG_INFO("Checking {} entities for selection", entities.size());

    for (auto* entity : entities) {
        if (!entity) continue;

        auto* transform = entity->getComponent<TransformComponent>();
        if (!transform) continue;

        Vec2f entityPos = transform->position;
        Vec2f scale = transform->scale;
        Vec2f origin = transform->origin;

        // Get sprite size for accurate hit detection
        auto* sprite = entity->getComponent<SpriteComponent>();
        Vec2f spriteSize{100.0f, 100.0f}; // Default size
        if (sprite && sprite->size.x > 0 && sprite->size.y > 0) {
            spriteSize = sprite->size;
        }

        // Calculate actual bounds accounting for origin
        Vec2f scaledSize{spriteSize.x * scale.x, spriteSize.y * scale.y};
        Vec2f scaledOrigin{origin.x * scale.x, origin.y * scale.y};

        // Top-left corner of the sprite
        Vec2f topLeft{entityPos.x - scaledOrigin.x, entityPos.y - scaledOrigin.y};
        // Bottom-right corner
        Vec2f bottomRight{topLeft.x + scaledSize.x, topLeft.y + scaledSize.y};

        LOG_DEBUG("Entity {}: pos({}, {}), origin({}, {}), size({}, {}), bounds[({}, {}) to ({}, {})]",
                 entity->getID(), entityPos.x, entityPos.y,
                 origin.x, origin.y, scaledSize.x, scaledSize.y,
                 topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);

        // Check if click is within sprite bounds (AABB test)
        if (worldPos.x >= topLeft.x &&
            worldPos.x <= bottomRight.x &&
            worldPos.y >= topLeft.y &&
            worldPos.y <= bottomRight.y) {

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

    if (clickedEntity) {
        LOG_INFO("Selected entity: {}", clickedEntity->getID());
        if (m_entityController) {
            m_entityController->selectEntity(clickedEntity);
        }
    } else {
        LOG_INFO("No entity under mouse - deselecting");
        if (m_entityController) {
            m_entityController->selectEntity(nullptr);
        }
    }
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


} // namespace NovaEditor
