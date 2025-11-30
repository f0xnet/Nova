#include "Core/EditorState.hpp"
#include <NovaEngine/Core/Logger.hpp>

namespace NovaEditor {

EditorState::EditorState()
    : m_selectedEntity(nullptr)
    , m_currentMode(Mode::Select)
    , m_currentTool(Tool::Move)
    , m_sceneModified(false)
    , m_gridEnabled(true)
    , m_gridSize(32.0f)
    , m_currentLayer(0)
{
    LOG_INFO("EditorState initialized");
}

void EditorState::setSelectedEntity(NovaEngine::Entity* entity) {
    if (m_selectedEntity != entity) {
        m_selectedEntity = entity;
        if (entity) {
            LOG_DEBUG("Entity selected: {}", entity->getID());
        } else {
            LOG_DEBUG("Selection cleared");
        }
    }
}

void EditorState::clearSelection() {
    setSelectedEntity(nullptr);
}

void EditorState::setMode(Mode mode) {
    if (m_currentMode != mode) {
        m_currentMode = mode;
        LOG_INFO("Editor mode changed to: {}", getModeName());

        // Clear selection when switching to Place mode
        if (mode == Mode::Place) {
            clearSelection();
        }
        // Clear placement type when switching away from Place mode
        else {
            m_placementType.clear();
        }
    }
}

const char* EditorState::getModeName() const {
    switch (m_currentMode) {
        case Mode::Select: return "Select";
        case Mode::Place:  return "Place";
        case Mode::Paint:  return "Paint";
        case Mode::Erase:  return "Erase";
        default:           return "Unknown";
    }
}

void EditorState::setTool(Tool tool) {
    if (m_currentTool != tool) {
        m_currentTool = tool;
        LOG_INFO("Editor tool changed to: {}", getToolName());
    }
}

const char* EditorState::getToolName() const {
    switch (m_currentTool) {
        case Tool::Move:      return "Move";
        case Tool::Rotate:    return "Rotate";
        case Tool::Scale:     return "Scale";
        case Tool::Light:     return "Light";
        case Tool::Activator: return "Activator";
        default:              return "Unknown";
    }
}

void EditorState::setPlacementType(const std::string& type) {
    if (m_placementType != type) {
        m_placementType = type;
        if (!type.empty()) {
            LOG_INFO("Placement type set to: {}", type);
            setMode(Mode::Place);
        }
    }
}

} // namespace NovaEditor
