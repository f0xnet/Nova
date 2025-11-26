#include "../include/EditorState.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <algorithm>

namespace NovaEditor {

EditorState::EditorState()
    : m_mode(EditorMode::Select)
    , m_tool(EditorTool::Move)
    , m_selectedEntity(nullptr)
    , m_placingEntityType("")
    , m_snapToGrid(true)
    , m_gridSize(32.0f)
    , m_currentLayer(0)
    , m_showGrid(true)
    , m_showGizmos(true)
    , m_showColliderBounds(true)
    , m_showLightRadii(true)
{
    LOG_INFO("EditorState initialized");
}

void EditorState::addToSelection(NovaEngine::Entity* entity) {
    if (!entity) return;

    auto it = std::find(m_selectedEntities.begin(), m_selectedEntities.end(), entity);
    if (it == m_selectedEntities.end()) {
        m_selectedEntities.push_back(entity);
        LOG_DEBUG("Added entity {} to selection", entity->getID());
    }
}

void EditorState::removeFromSelection(NovaEngine::Entity* entity) {
    if (!entity) return;

    auto it = std::find(m_selectedEntities.begin(), m_selectedEntities.end(), entity);
    if (it != m_selectedEntities.end()) {
        m_selectedEntities.erase(it);
        LOG_DEBUG("Removed entity {} from selection", entity->getID());
    }
}

void EditorState::clearMultiSelection() {
    m_selectedEntities.clear();
    LOG_DEBUG("Cleared multi-selection");
}

void EditorState::pushHistory(const std::string& action) {
    m_undoStack.push_back(action);
    m_redoStack.clear();  // Clear redo stack when new action is performed
    LOG_DEBUG("Pushed history: {}", action);
}

} // namespace NovaEditor
