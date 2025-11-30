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

    // Handle toggle_group action directly (show/hide UI panels)
    if (action == "toggle_group") {
        toggleGroup(value);
        return;
    }

    // Route other actions to the application callback
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
    LOG_INFO("Loading main UI layout...");

    // Load single unified UI JSON with groups
    bool success = loadLayout("editor_main.json");

    if (success) {
        LOG_INFO("Initializing group visibility...");

        // Initialize group visibility (panels visible, dialogs hidden)
        m_groupVisibility["toolbar"] = true;
        m_groupVisibility["palette"] = true;
        m_groupVisibility["inspector"] = true;
        m_groupVisibility["hierarchy"] = true;
        m_groupVisibility["scene_dialog"] = false;

        LOG_INFO("Hiding scene_dialog group...");
        try {
            // Hide dialog group initially
            m_uiManager.setGroupActive("scene_dialog", false);
            LOG_INFO("scene_dialog group hidden successfully");
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to hide scene_dialog group: {}", e.what());
        }

        LOG_INFO("UI layout loaded successfully");
    } else {
        LOG_ERROR("Failed to load UI layout");
    }

    return success;
}

void EditorUIManager::toggleGroup(const std::string& groupID) {
    // Toggle the visibility state
    bool currentState = m_groupVisibility[groupID];
    bool newState = !currentState;

    m_groupVisibility[groupID] = newState;
    m_uiManager.setGroupActive(groupID, newState);

    LOG_INFO("Toggled UI group '{}': {}", groupID, newState ? "VISIBLE" : "HIDDEN");
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
