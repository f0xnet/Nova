#include "../include/EditorUI.hpp"
#include "../include/EditorState.hpp"
#include "../include/EntityPalette.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <NovaEngine/ECS/Components.hpp>

namespace NovaEditor {

EditorUI::EditorUI(NovaEngine::UIManager& uiManager, EditorState* editorState)
    : m_uiManager(uiManager)
    , m_editorState(editorState)
    , m_showToolbar(true)
    , m_showEntityPalette(true)
    , m_showInspector(true)
    , m_showSceneHierarchy(true)
    , m_showSettings(false)
    , m_messageTimer(0.0f)
{
}

void EditorUI::initialize() {
    LOG_INFO("Initializing EditorUI...");

    createMenuBar();
    createToolbar();
    createEntityPalettePanel();
    createInspectorPanel();
    createSceneHierarchyPanel();
    createSettingsPanel();

    LOG_INFO("EditorUI initialized");
}

void EditorUI::update(float deltaTime) {
    // Mise à jour du timer de message
    if (m_messageTimer > 0.0f) {
        m_messageTimer -= deltaTime;
        if (m_messageTimer <= 0.0f) {
            m_currentMessage.clear();
        }
    }
}

void EditorUI::render() {
    // Les panels seront rendus par le UIManager
    // Afficher message temporaire s'il existe
    if (!m_currentMessage.empty() && m_messageTimer > 0.0f) {
        // TODO: Renderer message avec UIManager
        // Pour l'instant on utilise le logger
    }
}

void EditorUI::setActionCallback(std::function<void(const std::string&, const std::string&)> callback) {
    m_actionCallback = callback;
}

void EditorUI::refresh() {
    // Rafraîchir tous les panels
    if (m_editorState && m_editorState->hasSelection()) {
        refreshEntityInspector(m_editorState->getSelectedEntity());
    }
}

void EditorUI::refreshEntityInspector(NovaEngine::Entity* entity) {
    if (!entity) return;
    updateInspectorForEntity(entity);
}

void EditorUI::refreshEntityList(const std::vector<NovaEngine::Entity*>& entities) {
    // TODO: Mettre à jour la hiérarchie de scène
    LOG_DEBUG("Refreshing entity list: {} entities", entities.size());
}

void EditorUI::showToolbar(bool show) {
    m_showToolbar = show;
}

void EditorUI::showEntityPalette(bool show) {
    m_showEntityPalette = show;
}

void EditorUI::showInspector(bool show) {
    m_showInspector = show;
}

void EditorUI::showSceneHierarchy(bool show) {
    m_showSceneHierarchy = show;
}

void EditorUI::showSettings(bool show) {
    m_showSettings = show;
}

void EditorUI::showMessage(const std::string& message, float duration) {
    m_currentMessage = message;
    m_messageTimer = duration;
    LOG_INFO("[UI] {}", message);
}

void EditorUI::showError(const std::string& error) {
    showMessage("[ERROR] " + error, 5.0f);
    LOG_ERROR("{}", error);
}

void EditorUI::showSuccess(const std::string& message) {
    showMessage("[SUCCESS] " + message, 3.0f);
}

// === Private Methods ===

void EditorUI::createMenuBar() {
    // TODO: Créer menu bar avec UIManager
    // Pour l'instant, on utilise juste des raccourcis clavier
    LOG_DEBUG("Menu bar created (keyboard shortcuts only)");
}

void EditorUI::createToolbar() {
    if (!m_showToolbar) return;

    // TODO: Créer toolbar avec UIManager
    // Boutons: Select, Place, Paint, Erase
    // Outils: Move, Rotate, Scale
    LOG_DEBUG("Toolbar created");
}

void EditorUI::createEntityPalettePanel() {
    if (!m_showEntityPalette) return;

    // TODO: Créer panel palette avec UIManager
    // Liste d'entités disponibles par catégorie
    LOG_DEBUG("Entity palette panel created");
}

void EditorUI::createInspectorPanel() {
    if (!m_showInspector) return;

    // TODO: Créer panel inspecteur avec UIManager
    // Propriétés de l'entité sélectionnée
    LOG_DEBUG("Inspector panel created");
}

void EditorUI::createSceneHierarchyPanel() {
    if (!m_showSceneHierarchy) return;

    // TODO: Créer panel hiérarchie avec UIManager
    // Liste de toutes les entités de la scène
    LOG_DEBUG("Scene hierarchy panel created");
}

void EditorUI::createSettingsPanel() {
    if (!m_showSettings) return;

    // TODO: Créer panel paramètres avec UIManager
    // Paramètres de l'éditeur (grid, snap, etc.)
    LOG_DEBUG("Settings panel created");
}

void EditorUI::updateInspectorForEntity(NovaEngine::Entity* entity) {
    if (!entity) {
        LOG_DEBUG("Inspector cleared");
        return;
    }

    LOG_DEBUG("Updating inspector for entity {}", entity->getID());

    // Créer inspecteurs selon composants
    if (entity->hasComponent<NovaEngine::TransformComponent>()) {
        createTransformInspector(entity);
    }

    if (entity->hasComponent<NovaEngine::SpriteComponent>()) {
        createSpriteInspector(entity);
    }

    if (entity->hasComponent<NovaEngine::LightComponent>()) {
        createLightInspector(entity);
    }

    if (entity->hasComponent<NovaEngine::ColliderComponent>()) {
        createColliderInspector(entity);
    }

    if (entity->hasComponent<NovaEngine::ActivatorComponent>()) {
        createActivatorInspector(entity);
    }
}

void EditorUI::createTransformInspector(NovaEngine::Entity* entity) {
    auto* transform = entity->getComponent<NovaEngine::TransformComponent>();
    if (!transform) return;

    LOG_DEBUG("Transform: pos=({}, {}), rot={}, scale=({}, {})",
        transform->position.x, transform->position.y,
        transform->rotation,
        transform->scale.x, transform->scale.y);

    // TODO: Créer champs éditables avec UIManager
}

void EditorUI::createSpriteInspector(NovaEngine::Entity* entity) {
    auto* sprite = entity->getComponent<NovaEngine::SpriteComponent>();
    if (!sprite) return;

    LOG_DEBUG("Sprite: texture={}, zOrder={}",
        sprite->textureHandle, sprite->zOrder);

    // TODO: Créer champs éditables avec UIManager
}

void EditorUI::createLightInspector(NovaEngine::Entity* entity) {
    auto* light = entity->getComponent<NovaEngine::LightComponent>();
    if (!light) return;

    LOG_DEBUG("Light: color=({},{},{},{}), radius={}, intensity={}",
        light->color.r, light->color.g, light->color.b, light->color.a,
        light->radius, light->intensity);

    // TODO: Créer champs éditables avec UIManager
}

void EditorUI::createColliderInspector(NovaEngine::Entity* entity) {
    auto* collider = entity->getComponent<NovaEngine::ColliderComponent>();
    if (!collider) return;

    LOG_DEBUG("Collider: size=({}, {}), isTrigger={}",
        collider->size.x, collider->size.y,
        collider->isTrigger);

    // TODO: Créer champs éditables avec UIManager
}

void EditorUI::createActivatorInspector(NovaEngine::Entity* entity) {
    auto* activator = entity->getComponent<NovaEngine::ActivatorComponent>();
    if (!activator) return;

    LOG_DEBUG("Activator: size=({}, {}), radius={}, active={}",
        activator->size.x, activator->size.y,
        activator->radius, activator->isActive);

    // TODO: Créer champs éditables avec UIManager
}

} // namespace NovaEditor
