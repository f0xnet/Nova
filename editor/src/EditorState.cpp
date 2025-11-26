#include "../include/EditorState.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <algorithm>

namespace NovaEditor {

EditorState::EditorState()
    : m_mode(EditorMode::Select)
    , m_tool(EditorTool::Move)
    , m_selectedEntity(nullptr)
    , m_hoveredEntity(nullptr)
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

bool EditorState::isInSelection(NovaEngine::Entity* entity) const {
    if (!entity) return false;
    auto it = std::find(m_selectedEntities.begin(), m_selectedEntities.end(), entity);
    return it != m_selectedEntities.end();
}

void EditorState::copySelection() {
    m_clipboard.clear();

    if (m_selectedEntity) {
        m_clipboard.push_back(m_selectedEntity->getID());
        LOG_INFO("Copied 1 entity to clipboard");
    } else if (!m_selectedEntities.empty()) {
        for (auto* entity : m_selectedEntities) {
            m_clipboard.push_back(entity->getID());
        }
        LOG_INFO("Copied {} entities to clipboard", m_selectedEntities.size());
    }
}

void EditorState::addRecentScene(const std::string& path) {
    // Retirer si déjà présent
    auto it = std::find(m_recentScenes.begin(), m_recentScenes.end(), path);
    if (it != m_recentScenes.end()) {
        m_recentScenes.erase(it);
    }

    // Ajouter en tête
    m_recentScenes.insert(m_recentScenes.begin(), path);

    // Limiter à 10 scènes récentes
    if (m_recentScenes.size() > 10) {
        m_recentScenes.resize(10);
    }

    LOG_DEBUG("Added recent scene: {}", path);
}

void EditorState::addCameraBookmark(const NovaEngine::Vec2f& pos, float zoom, const std::string& name) {
    CameraBookmark bookmark;
    bookmark.position = pos;
    bookmark.zoom = zoom;
    bookmark.name = name;

    m_cameraBookmarks.push_back(bookmark);
    LOG_INFO("Added camera bookmark: {}", name);
}

} // namespace NovaEditor
