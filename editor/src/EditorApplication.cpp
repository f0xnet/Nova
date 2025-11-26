#include "../include/EditorApplication.hpp"
#include "../include/Gizmos.hpp"
#include "../include/EditorHistory.hpp"
#include "../include/SceneSerializer.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <NovaEngine/Core/ConfigManager.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>
#include <cmath>

namespace NovaEditor {

// Configuration par défaut pour l'éditeur
static NovaEngine::Application::Config createDefaultConfig() {
    NovaEngine::Application::Config config;
    config.windowTitle = "NovaEngine Editor v1.0";
    config.windowWidth = 1920;
    config.windowHeight = 1080;
    config.fullscreen = false;
    config.frameRateLimit = 60;
    config.vSync = true;
    config.clearColor = NovaEngine::Color{40, 40, 50};
    return config;
}

EditorApplication::EditorApplication()
    : Application(createDefaultConfig())
    , m_currentScene(nullptr)
    , m_sceneModified(false)
    , m_isDragging(false)
    , m_showGrid(true)
    , m_gridSize(32.0f)
    , m_snapToGrid(true)
{
    LOG_INFO("EditorApplication constructed");
}

EditorApplication::~EditorApplication() {
    // Cleanup current scene
    if (m_currentScene) {
        delete m_currentScene;
        m_currentScene = nullptr;
    }

    LOG_INFO("EditorApplication destroyed");
}

bool EditorApplication::onInitialize() {
    LOG_INFO("Initializing NovaEngine Editor...");

    initializeEditor();
    loadDefinitions();

    // Créer scène vide par défaut
    newScene();

    LOG_INFO("Editor initialized successfully");
    printControls();

    return true;
}

void EditorApplication::onUpdate(float deltaTime) {
    updateCamera(deltaTime);
    updateHovering();
    updateSelection();
    updateEntityManipulation(deltaTime);
    updatePlacement();

    // Update UI
    if (m_editorUI) {
        m_editorUI->update(deltaTime);
    }

    // Update scene
    if (m_currentScene) {
        m_currentScene->update(deltaTime);
    }
}

void EditorApplication::onRender() {
    renderScene();
    renderGrid();
    renderPlacementPreview();
    renderHoverHighlight();
    renderGizmos();
    renderUI();
}

void EditorApplication::onEvent(const NovaEngine::Event& event) {
    // Dispatch event to UI first (so buttons can handle clicks)
    m_uiManager.dispatchEvent(event);

    // Then handle editor input
    if (event.type == NovaEngine::EventType::Input) {
        handleEditorInput(event.inputEvent);
    }
}

void EditorApplication::onShutdown() {
    LOG_INFO("Editor shutting down");

    // Demander confirmation si scène modifiée
    if (m_sceneModified) {
        LOG_WARN("Scene has unsaved changes!");
    }

    m_sceneManager.shutdown();
}

// ============================================================================
// PRIVATE METHODS - INITIALIZATION
// ============================================================================

void EditorApplication::initializeEditor() {
    // Initialiser état éditeur
    m_editorState = std::make_unique<EditorState>();
    m_editorState->setMode(EditorMode::Select);
    m_editorState->setTool(EditorTool::Move);

    // Initialiser caméra
    m_editorCamera = std::make_unique<EditorCamera>();
    m_editorCamera->setPosition(NovaEngine::Vec2f{960, 540});  // Centre écran

    // Initialiser palette d'entités
    m_entityPalette = std::make_unique<EntityPalette>();

    // Initialiser UI
    m_editorUI = std::make_unique<EditorUI>(m_uiManager, m_editorState.get());
    m_editorUI->initialize();
    m_editorUI->setActionCallback([this](const std::string& action, const std::string& value) {
        onUIAction(action, value);
    });

    // Set UIManager action callback to forward button actions
    m_uiManager.setActionCallback([this](const std::string& action, const std::string& value, const NovaEngine::ID& componentID) {
        (void)componentID;  // Unused for now
        onUIAction(action, value);
    });

    // Initialiser historique
    m_editorHistory = std::make_unique<EditorHistory>();
    m_editorHistory->setMaxHistorySize(100);

    // Initialiser serializer
    m_sceneSerializer = std::make_unique<SceneSerializer>();

    LOG_INFO("Editor components initialized");
}

void EditorApplication::loadDefinitions() {
    LOG_INFO("Loading definitions...");

    // Initialiser SceneManager (charge définitions)
    if (!m_sceneManager.initialize("data/definitions/", "data/scenegraph.json")) {
        LOG_ERROR("Failed to initialize SceneManager");
        return;
    }

    // Charger palette depuis définitions
    m_entityPalette->loadFromDefinitionManager(m_sceneManager.getDefinitionManager());

    LOG_INFO("Definitions loaded");
}

void EditorApplication::printControls() {
    LOG_INFO("=== Editor Controls ===");
    LOG_INFO("  WASD / Arrows - Move camera");
    LOG_INFO("  Mouse Wheel   - Zoom");
    LOG_INFO("  Left Click    - Select entity");
    LOG_INFO("  Right Click   - Place entity (Place mode)");
    LOG_INFO("  G             - Toggle grid");
    LOG_INFO("  Q             - Select tool");
    LOG_INFO("  W             - Move tool");
    LOG_INFO("  E             - Rotate tool");
    LOG_INFO("  R             - Scale tool");
    LOG_INFO("  Ctrl+S        - Save scene");
    LOG_INFO("  Ctrl+O        - Open scene");
    LOG_INFO("  Ctrl+N        - New scene");
    LOG_INFO("  Ctrl+C        - Copy");
    LOG_INFO("  Ctrl+V        - Paste");
    LOG_INFO("  Ctrl+D        - Duplicate");
    LOG_INFO("  Ctrl+Z        - Undo");
    LOG_INFO("  Ctrl+Y        - Redo");
    LOG_INFO("  Delete        - Delete selected");
    LOG_INFO("  F             - Frame selected");
    LOG_INFO("  ESC           - Quit");
}

// ============================================================================
// PRIVATE METHODS - UPDATE
// ============================================================================

void EditorApplication::updateCamera(float deltaTime) {
    if (m_editorCamera) {
        m_editorCamera->update(deltaTime);
    }
}

void EditorApplication::updateHovering() {
    if (!m_currentScene) return;

    // Trouver entité sous la souris
    NovaEngine::Vec2i mouseScreenPos = getMousePosition();
    NovaEngine::Vec2f mouseWorldPos = m_editorCamera->screenToWorld(mouseScreenPos);

    NovaEngine::Entity* hoveredEntity = findEntityAt(mouseWorldPos);
    m_editorState->setHoveredEntity(hoveredEntity);
}

void EditorApplication::updateSelection() {
    // Géré par handleEditorInput lors des clics
}

void EditorApplication::updateEntityManipulation(float deltaTime) {
    if (!m_isDragging || !m_editorState->hasSelection()) return;

    // Manipulation continue pendant le drag
    NovaEngine::Vec2i mouseScreenPos = getMousePosition();
    NovaEngine::Vec2f mouseWorldPos = m_editorCamera->screenToWorld(mouseScreenPos);

    handleEntityDrag(mouseWorldPos);
}

void EditorApplication::updatePlacement() {
    // Mise à jour de la preview de placement
    // Géré dans renderPlacementPreview
}

// ============================================================================
// PRIVATE METHODS - RENDERING
// ============================================================================

void EditorApplication::renderScene() {
    if (m_currentScene) {
        m_currentScene->render();
    }
}

void EditorApplication::renderGrid() {
    if (!m_showGrid || !m_editorState->isGridVisible()) return;

    Gizmos gizmos;
    gizmos.renderGrid(m_gridSize, NovaEngine::Color{80, 80, 90, 100});
}

void EditorApplication::renderPlacementPreview() {
    if (m_editorState->getMode() != EditorMode::Place) return;
    if (!m_editorState->isPlacingEntity()) return;

    NovaEngine::Vec2i mouseScreenPos = getMousePosition();
    NovaEngine::Vec2f mouseWorldPos = m_editorCamera->screenToWorld(mouseScreenPos);

    if (m_snapToGrid) {
        mouseWorldPos = snapToGrid(mouseWorldPos);
    }

    // Dessiner preview semi-transparent à la position de la souris
    // TODO: Utiliser sprite de l'entité si disponible
    Gizmos gizmos;
    NovaEngine::Rect previewRect{mouseWorldPos.x - 16, mouseWorldPos.y - 16, 32, 32};
    gizmos.renderBounds(previewRect, NovaEngine::Color{0, 255, 0, 100});
}

void EditorApplication::renderHoverHighlight() {
    auto* hoveredEntity = m_editorState->getHoveredEntity();
    if (!hoveredEntity || (m_editorState->hasSelection() && hoveredEntity == m_editorState->getSelectedEntity())) {
        return;
    }

    // Highlight l'entité survolée
    auto* transform = hoveredEntity->getComponent<NovaEngine::TransformComponent>();
    if (transform) {
        Gizmos gizmos;
        NovaEngine::Vec2f size{32, 32}; // Taille par défaut

        // Utiliser taille du sprite si disponible
        if (hoveredEntity->hasComponent<NovaEngine::SpriteComponent>()) {
            auto* sprite = hoveredEntity->getComponent<NovaEngine::SpriteComponent>();
            if (sprite->size.x > 0 && sprite->size.y > 0) {
                size = sprite->size;
            }
        }

        NovaEngine::Rect hoverRect{transform->position.x - size.x / 2, transform->position.y - size.y / 2, size.x, size.y};
        gizmos.renderBounds(hoverRect, NovaEngine::Color{255, 255, 0, 150});
    }
}

void EditorApplication::renderGizmos() {
    if (!m_editorState->hasSelection() || !m_editorState->areGizmosVisible()) return;

    Gizmos gizmos;
    NovaEngine::Entity* selected = m_editorState->getSelectedEntity();
    auto* transform = selected->getComponent<NovaEngine::TransformComponent>();
    if (!transform) return;

    // Rendre le gizmo selon l'outil actif
    switch (m_editorState->getTool()) {
        case EditorTool::Move:
            gizmos.renderForEntity(selected, GizmoType::Move);
            break;
        case EditorTool::Rotate:
            gizmos.renderForEntity(selected, GizmoType::Rotate);
            break;
        case EditorTool::Scale:
            gizmos.renderForEntity(selected, GizmoType::Scale);
            break;
        default:
            break;
    }

    // Toujours afficher bounds de sélection
    gizmos.renderForEntity(selected, GizmoType::Bounds);

    // Afficher colliders si activé
    if (m_editorState->areColliderBoundsVisible() && selected->hasComponent<NovaEngine::ColliderComponent>()) {
        auto* collider = selected->getComponent<NovaEngine::ColliderComponent>();
        gizmos.renderActivator(transform->position, collider->size, NovaEngine::Color{0, 255, 0, 150});
    }

    // Afficher rayon lumière si activé
    if (m_editorState->areLightRadiiVisible() && selected->hasComponent<NovaEngine::LightComponent>()) {
        auto* light = selected->getComponent<NovaEngine::LightComponent>();
        gizmos.renderLight(transform->position, light->radius, light->color);
    }
}

void EditorApplication::renderUI() {
    // Reset view to screen space (UI should not be affected by camera)
    VIEWPORT().resetView();

    if (m_editorUI) {
        m_editorUI->render();
    }
}

// ============================================================================
// PRIVATE METHODS - INPUT HANDLING
// ============================================================================

void EditorApplication::handleEditorInput(const NovaEngine::InputEvent& input) {
    using namespace NovaEngine;

    if (input.type == InputEventType::KeyPressed) {
        handleKeyPress(input);
    }

    if (input.type == InputEventType::MouseButtonPressed) {
        handleMouseClick(input);
    }

    if (input.type == InputEventType::MouseButtonReleased) {
        if (input.mouseButton.button == MouseButton::Left) {
            m_isDragging = false;
        }
    }

    // Mouse wheel scrolling not supported in current InputEvent
    // TODO: Add zoom with keyboard shortcuts instead
}

void EditorApplication::handleKeyPress(const NovaEngine::InputEvent& input) {
    using namespace NovaEngine;

    // Toggle grid
    if (input.key.code == KeyCode::G && !input.key.control) {
        m_showGrid = !m_showGrid;
        m_editorState->setGridVisible(m_showGrid);
        LOG_INFO("Grid {}", m_showGrid ? "enabled" : "disabled");
    }

    // Tool shortcuts
    if (input.key.code == KeyCode::Q) {
        m_editorState->setTool(EditorTool::Move);
        LOG_INFO("Tool: Move");
    }
    if (input.key.code == KeyCode::W && !input.key.control) {
        m_editorState->setTool(EditorTool::Move);
        LOG_INFO("Tool: Move");
    }
    if (input.key.code == KeyCode::E && !input.key.control) {
        m_editorState->setTool(EditorTool::Rotate);
        LOG_INFO("Tool: Rotate");
    }
    if (input.key.code == KeyCode::R && !input.key.control) {
        m_editorState->setTool(EditorTool::Scale);
        LOG_INFO("Tool: Scale");
    }

    // Save scene
    if (input.key.code == KeyCode::S && input.key.control) {
        if (!m_currentScenePath.empty()) {
            saveScene(m_currentScenePath);
        } else {
            // TODO: Open save dialog
            saveScene("data/scenes/untitled.json");
        }
    }

    // Open scene
    if (input.key.code == KeyCode::O && input.key.control) {
        // TODO: Open file dialog
        LOG_INFO("Open scene dialog");
    }

    // New scene
    if (input.key.code == KeyCode::N && input.key.control) {
        newScene();
    }

    // Copy
    if (input.key.code == KeyCode::C && input.key.control) {
        copySelectedEntities();
    }

    // Paste
    if (input.key.code == KeyCode::V && input.key.control) {
        pasteEntities();
    }

    // Duplicate
    if (input.key.code == KeyCode::D && input.key.control) {
        duplicateSelectedEntity();
    }

    // Undo
    if (input.key.code == KeyCode::Z && input.key.control) {
        undo();
    }

    // Redo
    if (input.key.code == KeyCode::Y && input.key.control) {
        redo();
    }

    // Frame selected
    if (input.key.code == KeyCode::F) {
        frameSelected();
    }

    // Delete selected
    if (input.key.code == KeyCode::Backspace) {
        if (m_editorState->hasSelection()) {
            deleteSelectedEntity();
        }
    }
}

void EditorApplication::handleMouseClick(const NovaEngine::InputEvent& input) {
    using namespace NovaEngine;

    Vec2i screenPos{input.mouseButton.x, input.mouseButton.y};
    Vec2f worldPos = m_editorCamera->screenToWorld(screenPos);

    if (input.mouseButton.button == MouseButton::Left) {
        if (m_editorState->getMode() == EditorMode::Place) {
            // Placer entité
            handleEntityPlacement(worldPos);
        } else {
            // Sélectionner entité
            handleEntitySelection(worldPos);

            // Commencer drag si sélection
            if (m_editorState->hasSelection()) {
                m_isDragging = true;
                m_dragStartPos = worldPos;

                auto* transform = m_editorState->getSelectedEntity()->getComponent<TransformComponent>();
                if (transform) {
                    m_dragOffset = transform->position - worldPos;
                }
            }
        }
    }

    if (input.mouseButton.button == MouseButton::Right) {
        // Cancel placement/selection
        if (m_editorState->getMode() == EditorMode::Place) {
            m_editorState->setPlacingEntityType("");
            LOG_INFO("Cancelled placement");
        }
    }
}

void EditorApplication::handleEntityPlacement(const NovaEngine::Vec2f& worldPos) {
    if (!m_editorState->isPlacingEntity() || !m_currentScene) return;

    NovaEngine::Vec2f placePos = worldPos;
    if (m_snapToGrid) {
        placePos = snapToGrid(worldPos);
    }

    createEntity(m_editorState->getPlacingEntityType(), placePos);

    // Ne pas clear le type de placement pour permettre le multi-placement
    LOG_INFO("Entity placed at ({}, {})", placePos.x, placePos.y);
}

void EditorApplication::handleEntitySelection(const NovaEngine::Vec2f& worldPos) {
    if (!m_currentScene) return;

    NovaEngine::Entity* clickedEntity = findEntityAt(worldPos);

    if (clickedEntity) {
        m_editorState->setSelectedEntity(clickedEntity);
        LOG_INFO("Selected entity {}", clickedEntity->getID());

        // Rafraîchir l'inspecteur
        if (m_editorUI) {
            m_editorUI->refreshEntityInspector(clickedEntity);
        }
    } else {
        m_editorState->clearSelection();
        LOG_INFO("Selection cleared");
    }
}

void EditorApplication::handleEntityDrag(const NovaEngine::Vec2f& worldPos) {
    if (!m_editorState->hasSelection()) return;

    auto* entity = m_editorState->getSelectedEntity();
    auto* transform = entity->getComponent<NovaEngine::TransformComponent>();
    if (!transform) return;

    NovaEngine::Vec2f oldPos = transform->position;
    NovaEngine::Vec2f newPos = worldPos + m_dragOffset;

    if (m_snapToGrid) {
        newPos = snapToGrid(newPos);
    }

    // Seulement si position a changé
    if (oldPos.x != newPos.x || oldPos.y != newPos.y) {
        transform->position = newPos;
        m_sceneModified = true;
    }
}

// ============================================================================
// PRIVATE METHODS - ENTITY OPERATIONS
// ============================================================================

void EditorApplication::createEntity(const std::string& type, const NovaEngine::Vec2f& position) {
    if (!m_currentScene) return;

    LOG_INFO("Creating entity of type '{}' at ({}, {})", type, position.x, position.y);

    auto& registry = m_currentScene->getEntityRegistry();
    NovaEngine::Entity* entity = registry.createEntity();

    if (!entity) {
        LOG_ERROR("Failed to create entity");
        return;
    }

    // Add Transform component
    auto transform = std::make_unique<NovaEngine::TransformComponent>();
    transform->position = position;
    transform->rotation = 0.0f;
    transform->scale = {1.0f, 1.0f};
    entity->addComponent(std::move(transform));

    // Add type-specific components
    if (type == "sprite") {
        auto sprite = std::make_unique<NovaEngine::SpriteComponent>();
        sprite->size = {32, 32};
        sprite->zOrder = 0;
        entity->addComponent(std::move(sprite));
    }
    else if (type == "light") {
        auto light = std::make_unique<NovaEngine::LightComponent>();
        light->color = {255, 255, 200, 255};
        light->radius = 200.0f;
        light->intensity = 1.0f;
        entity->addComponent(std::move(light));
    }
    else if (type == "collider") {
        auto collider = std::make_unique<NovaEngine::ColliderComponent>();
        collider->size = {32, 32};
        collider->isTrigger = false;
        entity->addComponent(std::move(collider));
    }
    else if (type == "activator") {
        auto activator = std::make_unique<NovaEngine::ActivatorComponent>();
        activator->size = {32, 32};
        activator->radius = 50.0f;
        activator->isActive = true;
        entity->addComponent(std::move(activator));
    }

    m_sceneModified = true;
    LOG_INFO("Entity {} created with type '{}'", entity->getID(), type);

    // Sélectionner l'entité créée
    m_editorState->setSelectedEntity(entity);
}

void EditorApplication::deleteSelectedEntity() {
    if (!m_currentScene || !m_editorState->hasSelection()) return;

    NovaEngine::Entity* selected = m_editorState->getSelectedEntity();
    NovaEngine::u64 id = selected->getID();

    // TODO: Ajouter command pour undo
    // auto cmd = std::make_unique<DeleteEntityCommand>(selected);
    // m_editorHistory->executeCommand(std::move(cmd));

    m_currentScene->getEntityRegistry().destroyEntity(id);
    m_editorState->clearSelection();
    m_sceneModified = true;

    LOG_INFO("Entity {} deleted", id);
}

void EditorApplication::duplicateSelectedEntity() {
    if (!m_currentScene || !m_editorState->hasSelection()) return;

    auto* original = m_editorState->getSelectedEntity();
    auto& registry = m_currentScene->getEntityRegistry();

    // Créer nouvelle entité
    auto* duplicate = registry.createEntity();

    // Copier tous les composants
    // TODO: Implémenter clone de composants

    // Pour l'instant, juste copier Transform avec offset
    if (original->hasComponent<NovaEngine::TransformComponent>()) {
        auto* origTransform = original->getComponent<NovaEngine::TransformComponent>();
        auto dupTransform = std::make_unique<NovaEngine::TransformComponent>();
        dupTransform->position = origTransform->position + NovaEngine::Vec2f{20, 20}; // Offset
        dupTransform->rotation = origTransform->rotation;
        dupTransform->scale = origTransform->scale;
        duplicate->addComponent(std::move(dupTransform));
    }

    m_sceneModified = true;
    m_editorState->setSelectedEntity(duplicate);
    LOG_INFO("Entity duplicated");
}

void EditorApplication::copySelectedEntities() {
    if (!m_editorState->hasSelection()) {
        LOG_WARN("No entity selected to copy");
        return;
    }

    m_editorState->copySelection();
}

void EditorApplication::pasteEntities() {
    if (!m_editorState->hasClipboard() || !m_currentScene) {
        LOG_WARN("Clipboard is empty");
        return;
    }

    // TODO: Implémenter paste propre avec clonage de composants
    LOG_INFO("Paste functionality - TODO: implement entity cloning");
}

NovaEngine::Entity* EditorApplication::findEntityAt(const NovaEngine::Vec2f& worldPos) {
    if (!m_currentScene) return nullptr;

    auto entities = m_currentScene->getEntityRegistry().getAllEntities();

    NovaEngine::Entity* closest = nullptr;
    float closestDist = 50.0f;  // Distance maximale pour sélection

    for (auto* entity : entities) {
        auto* transform = entity->getComponent<NovaEngine::TransformComponent>();
        if (!transform) continue;

        // Calculer distance au centre de l'entité
        float dx = worldPos.x - transform->position.x;
        float dy = worldPos.y - transform->position.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        // TODO: Utiliser bounds réels du sprite si disponible
        if (dist < closestDist) {
            closest = entity;
            closestDist = dist;
        }
    }

    return closest;
}

// ============================================================================
// PRIVATE METHODS - SCENE MANAGEMENT
// ============================================================================

void EditorApplication::newScene() {
    LOG_INFO("Creating new scene...");

    // TODO: Demander confirmation si scène modifiée
    if (m_sceneModified) {
        LOG_WARN("Current scene has unsaved changes!");
    }

    // Delete old scene if it exists
    if (m_currentScene) {
        delete m_currentScene;
        m_currentScene = nullptr;
    }

    // Créer scène vide
    m_currentScene = new NovaEngine::Scene("editor_scene");
    m_currentScenePath = "";
    m_sceneModified = false;

    // Clear selection
    m_editorState->clearSelection();
    m_editorHistory->clear();

    LOG_INFO("New scene created");
}

void EditorApplication::loadScene(const std::string& path) {
    LOG_INFO("Loading scene from: {}", path);

    if (!m_currentScene) {
        LOG_ERROR("No active scene");
        return;
    }

    if (m_sceneSerializer->loadScene(m_currentScene, path)) {
        m_currentScenePath = path;
        m_sceneModified = false;
        m_editorState->addRecentScene(path);
        m_editorHistory->clear();
        LOG_INFO("Scene loaded successfully");
    } else {
        LOG_ERROR("Failed to load scene from: {}", path);
    }
}

void EditorApplication::saveScene(const std::string& path) {
    LOG_INFO("Saving scene to: {}", path);

    if (!m_currentScene) {
        LOG_ERROR("No active scene to save");
        return;
    }

    // Valider la scène avant sauvegarde
    auto issues = m_sceneSerializer->validateScene(m_currentScene);
    for (const auto& issue : issues) {
        LOG_WARN("Validation: {}", issue);
    }

    if (m_sceneSerializer->saveScene(m_currentScene, path)) {
        m_currentScenePath = path;
        m_sceneModified = false;
        m_editorState->addRecentScene(path);
        LOG_INFO("Scene saved successfully");

        if (m_editorUI) {
            m_editorUI->showSuccess("Scene saved");
        }
    } else {
        LOG_ERROR("Failed to save scene");

        if (m_editorUI) {
            m_editorUI->showError("Failed to save scene");
        }
    }
}

// ============================================================================
// PRIVATE METHODS - HISTORY
// ============================================================================

void EditorApplication::undo() {
    if (m_editorHistory->canUndo()) {
        m_editorHistory->undo();
        m_sceneModified = true;
        LOG_INFO("Undo: {}", m_editorHistory->getUndoCommandName());
    } else {
        LOG_INFO("Nothing to undo");
    }
}

void EditorApplication::redo() {
    if (m_editorHistory->canRedo()) {
        m_editorHistory->redo();
        m_sceneModified = true;
        LOG_INFO("Redo: {}", m_editorHistory->getRedoCommandName());
    } else {
        LOG_INFO("Nothing to redo");
    }
}

// ============================================================================
// PRIVATE METHODS - CAMERA
// ============================================================================

void EditorApplication::frameSelected() {
    if (!m_editorState->hasSelection() || !m_editorCamera) return;

    auto* entity = m_editorState->getSelectedEntity();
    auto* transform = entity->getComponent<NovaEngine::TransformComponent>();

    if (transform) {
        m_editorCamera->setPosition(transform->position);
        m_editorCamera->setZoom(1.0f);
        LOG_INFO("Framed selected entity");
    }
}

// ============================================================================
// PRIVATE METHODS - UTILITIES
// ============================================================================

NovaEngine::Vec2f EditorApplication::snapToGrid(const NovaEngine::Vec2f& position) const {
    float gridSize = m_editorState->getGridSize();
    return NovaEngine::Vec2f{
        std::round(position.x / gridSize) * gridSize,
        std::round(position.y / gridSize) * gridSize
    };
}

NovaEngine::Vec2i EditorApplication::getMousePosition() const {
    // TODO: Récupérer vraie position souris depuis backend
    return NovaEngine::Vec2i{0, 0};
}

void EditorApplication::onUIAction(const std::string& action, const std::string& value) {
    LOG_INFO("UI Action: '{}' = '{}'", action, value);

    if (action == "save_scene") {
        saveScene("editor_scene.json");
    }
    else if (action == "load_scene") {
        loadScene("editor_scene.json");
    }
    else if (action == "play_mode") {
        LOG_INFO("Play mode not yet implemented");
        // TODO: Enter play mode
    }
    else if (action == "toggle_grid") {
        m_showGrid = !m_showGrid;
        LOG_INFO("Grid: {}", m_showGrid ? "ON" : "OFF");
    }
    else if (action == "add_entity") {
        // Set mode to place entity of specified type
        m_editorState->setMode(EditorMode::Place);
        m_editorState->setPlacingEntityType(value);
        LOG_INFO("Click to place: {}", value);
    }
    else if (action == "place_entity") {
        m_editorState->setMode(EditorMode::Place);
        m_editorState->setPlacingEntityType(value);
        LOG_INFO("Placing entity: {}", value);
    }
    else if (action == "set_mode") {
        // TODO: Parse value to EditorMode
    }
    else if (action == "set_tool") {
        // TODO: Parse value to EditorTool
    }
}

} // namespace NovaEditor
