#include "../include/EditorApplication.hpp"
#include "../include/Gizmos.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <NovaEngine/Core/ConfigManager.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <NovaEngine/Backend/BackendManager.hpp>

namespace NovaEditor {

// Configuration par défaut pour l'éditeur
static NovaEngine::Application::Config createDefaultConfig() {
    NovaEngine::Application::Config config;
    config.windowTitle = "NovaEngine Editor";
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
    LOG_INFO("EditorApplication destroyed");
}

bool EditorApplication::onInitialize() {
    LOG_INFO("Initializing NovaEngine Editor...");

    initializeEditor();
    loadDefinitions();

    // Créer scène vide par défaut
    newScene();

    LOG_INFO("Editor initialized successfully");
    LOG_INFO("=== Editor Controls ===");
    LOG_INFO("  WASD / Arrow Keys - Move camera");
    LOG_INFO("  Mouse Wheel - Zoom");
    LOG_INFO("  Left Click - Select entity");
    LOG_INFO("  G - Toggle grid");
    LOG_INFO("  Ctrl+S - Save scene");
    LOG_INFO("  Ctrl+O - Open scene");
    LOG_INFO("  Delete - Delete selected entity");
    LOG_INFO("  ESC - Quit");

    return true;
}

void EditorApplication::onUpdate(float deltaTime) {
    updateCamera(deltaTime);
    updateSelection();
    updateEntityManipulation(deltaTime);

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
    renderGizmos();
    renderUI();
}

void EditorApplication::onEvent(const NovaEngine::Event& event) {
    if (event.type == NovaEngine::EventType::Input) {
        handleEditorInput(event.inputEvent);
    }
}

void EditorApplication::onShutdown() {
    LOG_INFO("Editor shutting down");
    m_sceneManager.shutdown();
}

// === Private Methods ===

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

void EditorApplication::updateCamera(float deltaTime) {
    if (m_editorCamera) {
        m_editorCamera->update(deltaTime);
    }
}

void EditorApplication::updateSelection() {
    // TODO: Implémenter logique de sélection
}

void EditorApplication::updateEntityManipulation(float deltaTime) {
    // TODO: Implémenter manipulation d'entités (drag, rotate, etc.)
}

void EditorApplication::renderScene() {
    if (m_currentScene) {
        m_currentScene->render();
    }
}

void EditorApplication::renderGrid() {
    if (m_showGrid) {
        Gizmos gizmos;
        gizmos.renderGrid(m_gridSize, NovaEngine::Color{100, 100, 100, 100});
    }
}

void EditorApplication::renderGizmos() {
    if (!m_editorState || !m_editorState->hasSelection()) return;

    Gizmos gizmos;
    NovaEngine::Entity* selected = m_editorState->getSelectedEntity();

    if (m_editorState->getTool() == EditorTool::Move) {
        gizmos.renderForEntity(selected, GizmoType::Move);
    }

    // Toujours afficher bounds de sélection
    gizmos.renderForEntity(selected, GizmoType::Bounds);

    // Afficher colliders si activé
    if (m_editorState->areColliderBoundsVisible()) {
        auto* collider = selected->getComponent<NovaEngine::ColliderComponent>();
        auto* transform = selected->getComponent<NovaEngine::TransformComponent>();
        if (collider && transform) {
            gizmos.renderActivator(transform->position, collider->size, NovaEngine::Color{0, 255, 0, 150});
        }
    }

    // Afficher rayon lumière si activé
    if (m_editorState->areLightRadiiVisible()) {
        auto* light = selected->getComponent<NovaEngine::LightComponent>();
        auto* transform = selected->getComponent<NovaEngine::TransformComponent>();
        if (light && transform) {
            gizmos.renderLight(transform->position, light->radius, light->color);
        }
    }
}

void EditorApplication::renderUI() {
    if (m_editorUI) {
        m_editorUI->render();
    }
}

void EditorApplication::handleEditorInput(const NovaEngine::InputEvent& input) {
    using namespace NovaEngine;

    if (input.type == InputEventType::KeyPressed) {
        // Toggle grid
        if (input.key.code == KeyCode::G) {
            m_showGrid = !m_showGrid;
            LOG_INFO("Grid {}", m_showGrid ? "enabled" : "disabled");
        }

        // Save scene
        if (input.key.code == KeyCode::S && input.key.control) {
            if (!m_currentScenePath.empty()) {
                saveScene(m_currentScenePath);
            } else {
                LOG_WARN("No scene path set, cannot save");
            }
        }

        // Delete selected entity
        if (input.key.code == KeyCode::Backspace && m_editorState->hasSelection()) {
            deleteSelectedEntity();
        }
    }

    // Mouse wheel pour zoom
    // TODO: Gérer événement molette

    // Clic souris pour sélection
    if (input.type == InputEventType::MouseButtonPressed) {
        if (input.mouseButton.button == MouseButton::Left) {
            Vec2i screenPos{input.mouseButton.x, input.mouseButton.y};
            Vec2f worldPos = m_editorCamera->screenToWorld(screenPos);
            handleEntitySelection(worldPos);
        }
    }
}

void EditorApplication::handleEntityPlacement(const NovaEngine::Vec2f& worldPos) {
    // TODO: Implémenter placement d'entité
    LOG_DEBUG("Place entity at ({}, {})", worldPos.x, worldPos.y);
}

void EditorApplication::handleEntitySelection(const NovaEngine::Vec2f& worldPos) {
    if (!m_currentScene) return;

    // Trouver entité la plus proche du clic
    auto entities = m_currentScene->getEntityRegistry().getAllEntities();

    NovaEngine::Entity* closest = nullptr;
    float closestDist = 50.0f;  // Distance maximale pour sélection

    for (auto* entity : entities) {
        auto* transform = entity->getComponent<NovaEngine::TransformComponent>();
        if (!transform) continue;

        float dx = worldPos.x - transform->position.x;
        float dy = worldPos.y - transform->position.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < closestDist) {
            closest = entity;
            closestDist = dist;
        }
    }

    if (closest) {
        m_editorState->setSelectedEntity(closest);
        LOG_INFO("Selected entity {}", closest->getID());
    } else {
        m_editorState->clearSelection();
        LOG_INFO("Selection cleared");
    }
}

void EditorApplication::handleEntityDrag(const NovaEngine::Vec2f& worldPos) {
    // TODO: Implémenter drag d'entité
}

void EditorApplication::newScene() {
    LOG_INFO("Creating new scene...");

    // Créer scène vide
    if (!m_sceneManager.loadScene("data/scenes/empty.json", "editor_scene")) {
        // Si pas de scène vide, en créer une manuellement
        LOG_WARN("No empty scene found, creating temporary scene");
    }

    m_sceneManager.setActiveScene("editor_scene");
    m_currentScene = m_sceneManager.getActiveScene();
    m_currentScenePath = "";
    m_sceneModified = false;

    LOG_INFO("New scene created");
}

void EditorApplication::loadScene(const std::string& path) {
    LOG_INFO("Loading scene from: {}", path);

    if (m_sceneManager.loadScene(path, "editor_scene")) {
        m_sceneManager.setActiveScene("editor_scene");
        m_currentScene = m_sceneManager.getActiveScene();
        m_currentScenePath = path;
        m_sceneModified = false;
        LOG_INFO("Scene loaded successfully");
    } else {
        LOG_ERROR("Failed to load scene from: {}", path);
    }
}

void EditorApplication::saveScene(const std::string& path) {
    LOG_INFO("Saving scene to: {}", path);

    // TODO: Implémenter sauvegarde JSON
    // Pour l'instant juste logger
    LOG_WARN("Scene save not yet implemented");

    m_sceneModified = false;
}

void EditorApplication::createEntity(const std::string& type, const NovaEngine::Vec2f& position) {
    if (!m_currentScene) return;

    LOG_INFO("Creating entity of type '{}' at ({}, {})", type, position.x, position.y);

    // TODO: Utiliser DefinitionManager pour créer entité depuis définition
    auto& registry = m_currentScene->getEntityRegistry();
    NovaEngine::Entity* entity = registry.createEntity();

    // Ajouter transform
    auto transform = std::make_unique<NovaEngine::TransformComponent>();
    transform->position = position;
    entity->addComponent(std::move(transform));

    m_sceneModified = true;
    LOG_INFO("Entity {} created", entity->getID());
}

void EditorApplication::deleteSelectedEntity() {
    if (!m_currentScene || !m_editorState->hasSelection()) return;

    NovaEngine::Entity* selected = m_editorState->getSelectedEntity();
    NovaEngine::u64 id = selected->getID();

    m_currentScene->getEntityRegistry().destroyEntity(id);
    m_editorState->clearSelection();
    m_sceneModified = true;

    LOG_INFO("Entity {} deleted", id);
}

void EditorApplication::duplicateSelectedEntity() {
    // TODO: Implémenter duplication d'entité
    LOG_WARN("Entity duplication not yet implemented");
}

void EditorApplication::onUIAction(const std::string& action, const std::string& value) {
    LOG_DEBUG("UI Action: '{}' = '{}'", action, value);

    // TODO: Gérer actions UI
}

} // namespace NovaEditor
