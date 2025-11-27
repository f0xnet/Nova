#include "UI/EditorUIManager.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <NovaEngine/UI/Components/Button.hpp>
#include <NovaEngine/UI/Components/Text.hpp>

namespace NovaEditor {

EditorUIManager::EditorUIManager(NovaEngine::UIManager& uiManager, EditorState* state, EditorConfig* config)
    : m_uiManager(uiManager)
    , m_editorState(state)
    , m_editorConfig(config)
{
    LOG_INFO("EditorUIManager created");
}

bool EditorUIManager::initialize() {
    LOG_INFO("Initializing EditorUIManager...");

    // Set up action callback from UIManager to this class
    m_uiManager.setActionCallback([this](const std::string& action, const std::string& value, const NovaEngine::ID& componentID) {
        handleAction(action, value, componentID);
    });

    // Load all UI layouts from JSON
    if (!loadAllLayouts()) {
        LOG_ERROR("Failed to load UI layouts");
        return false;
    }

    LOG_INFO("EditorUIManager initialized successfully");
    return true;
}

void EditorUIManager::update(float deltaTime) {
    // Update UI based on editor state changes
    // This would typically update button states, refresh panels, etc.
    // For Phase 1, we keep it minimal
}

void EditorUIManager::setActionCallback(ActionCallback callback) {
    m_actionCallback = std::move(callback);
    LOG_DEBUG("Action callback set in EditorUIManager");
}

void EditorUIManager::handleAction(const std::string& action, const std::string& value, const NovaEngine::ID& componentID) {
    LOG_INFO("[EditorUIManager] Action received: '{}' with value '{}' from component '{}'",
             action, value, componentID);

    // Route the action to the application callback
    if (m_actionCallback) {
        m_actionCallback(action, value);
    } else {
        LOG_WARN("No action callback set - action '{}' not handled", action);
    }
}

void EditorUIManager::refreshInspector() {
    // TODO: Dynamically update inspector panel based on selected entity
    // For Phase 1, we just have static placeholder text
    LOG_DEBUG("Inspector refresh requested (not yet implemented)");
}

void EditorUIManager::refreshSceneHierarchy() {
    // TODO: Update hierarchy list with current scene entities
    // For Phase 1, we just have static placeholder text
    LOG_DEBUG("Scene hierarchy refresh requested (not yet implemented)");
}

void EditorUIManager::updateEntityPalette(const std::string& category) {
    // TODO: Load entity definitions for the category and populate palette
    // For Phase 1, we just log the category change
    LOG_INFO("Entity palette category changed to: {}", category);
}

void EditorUIManager::updateToolbarState() {
    // TODO: Update undo/redo button states, grid toggle text, etc.
    // For Phase 1, buttons are static
    LOG_DEBUG("Toolbar state update requested (not yet implemented)");
}

bool EditorUIManager::loadAllLayouts() {
    LOG_INFO("Loading all UI layouts...");

    bool success = true;

    // Load all UI panels
    success &= loadLayout("toolbar.json");
    success &= loadLayout("entity_palette.json");
    success &= loadLayout("inspector.json");
    success &= loadLayout("scene_hierarchy.json");

    if (success) {
        LOG_INFO("All UI layouts loaded successfully");
    } else {
        LOG_ERROR("Some UI layouts failed to load");
    }

    return success;
}

bool EditorUIManager::loadLayout(const std::string& filename) {
    std::string fullPath = m_editorConfig->getUIPath() + "/" + filename;

    LOG_INFO("Loading UI layout: {}", fullPath);

    if (!m_uiLoader.loadFromFile(fullPath, m_uiManager)) {
        LOG_ERROR("Failed to load UI layout: {}", fullPath);
        return false;
    }

    LOG_DEBUG("UI layout loaded: {}", filename);
    return true;
}

} // namespace NovaEditor
