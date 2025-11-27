#pragma once

#include <NovaEngine/Core/Types.hpp>
#include <NovaEngine/ECS/Entity.hpp>
#include <string>
#include <vector>

namespace NovaEditor {

/**
 * @brief Mode de l'éditeur
 */
enum class EditorMode {
    Select,      // Mode sélection/manipulation
    Place,       // Mode placement d'entités
    Paint,       // Mode peinture (tiles/décors)
    Erase        // Mode effacement
};

/**
 * @brief Outil actif dans l'éditeur
 */
enum class EditorTool {
    None,
    Move,        // Déplacer entités
    Rotate,      // Rotation
    Scale,       // Échelle
    Light,       // Placer lumières
    Activator,   // Placer activateurs
    Waypoint     // Placer waypoints
};

/**
 * @brief État global de l'éditeur
 *
 * Contient toutes les informations d'état de l'éditeur :
 * mode, sélection, outils, paramètres, etc.
 */
class EditorState {
public:
    EditorState();
    ~EditorState() = default;

    // Mode
    EditorMode getMode() const { return m_mode; }
    void setMode(EditorMode mode) { m_mode = mode; }

    // Outil
    EditorTool getTool() const { return m_tool; }
    void setTool(EditorTool tool) { m_tool = tool; }

    // Sélection
    NovaEngine::Entity* getSelectedEntity() const { return m_selectedEntity; }
    void setSelectedEntity(NovaEngine::Entity* entity) { m_selectedEntity = entity; }
    void clearSelection() { m_selectedEntity = nullptr; }
    bool hasSelection() const { return m_selectedEntity != nullptr; }

    // Multi-sélection
    const std::vector<NovaEngine::Entity*>& getSelectedEntities() const { return m_selectedEntities; }
    void addToSelection(NovaEngine::Entity* entity);
    void removeFromSelection(NovaEngine::Entity* entity);
    void clearMultiSelection();
    bool isInSelection(NovaEngine::Entity* entity) const;
    size_t getSelectionCount() const { return m_selectedEntities.size(); }

    // Entité survolée
    NovaEngine::Entity* getHoveredEntity() const { return m_hoveredEntity; }
    void setHoveredEntity(NovaEngine::Entity* entity) { m_hoveredEntity = entity; }

    // Clipboard pour copier/coller
    void copySelection();
    bool hasClipboard() const { return !m_clipboard.empty(); }
    const std::vector<NovaEngine::u64>& getClipboard() const { return m_clipboard; }
    void clearClipboard() { m_clipboard.clear(); }

    // Entité en cours de placement
    const std::string& getPlacingEntityType() const { return m_placingEntityType; }
    void setPlacingEntityType(const std::string& type) { m_placingEntityType = type; }
    bool isPlacingEntity() const { return !m_placingEntityType.empty(); }

    // Paramètres de placement
    bool isSnapToGridEnabled() const { return m_snapToGrid; }
    void setSnapToGrid(bool enabled) { m_snapToGrid = enabled; }

    NovaEngine::f32 getGridSize() const { return m_gridSize; }
    void setGridSize(NovaEngine::f32 size) { m_gridSize = size; }

    // Layers (pour organisation)
    NovaEngine::i32 getCurrentLayer() const { return m_currentLayer; }
    void setCurrentLayer(NovaEngine::i32 layer) { m_currentLayer = layer; }

    // Visibilité
    bool isGridVisible() const { return m_showGrid; }
    void setGridVisible(bool visible) { m_showGrid = visible; }

    bool areGizmosVisible() const { return m_showGizmos; }
    void setGizmosVisible(bool visible) { m_showGizmos = visible; }

    bool areColliderBoundsVisible() const { return m_showColliderBounds; }
    void setColliderBoundsVisible(bool visible) { m_showColliderBounds = visible; }

    bool areLightRadiiVisible() const { return m_showLightRadii; }
    void setLightRadiiVisible(bool visible) { m_showLightRadii = visible; }

    // Scènes récentes
    void addRecentScene(const std::string& path);
    const std::vector<std::string>& getRecentScenes() const { return m_recentScenes; }
    void clearRecentScenes() { m_recentScenes.clear(); }

    // Camera bookmarks
    struct CameraBookmark {
        NovaEngine::Vec2f position;
        float zoom;
        std::string name;
    };

    void addCameraBookmark(const NovaEngine::Vec2f& pos, float zoom, const std::string& name);
    const std::vector<CameraBookmark>& getCameraBookmarks() const { return m_cameraBookmarks; }
    void clearCameraBookmarks() { m_cameraBookmarks.clear(); }

private:
    // Mode et outils
    EditorMode m_mode;
    EditorTool m_tool;

    // Sélection
    NovaEngine::Entity* m_selectedEntity;
    std::vector<NovaEngine::Entity*> m_selectedEntities;
    NovaEngine::Entity* m_hoveredEntity;

    // Clipboard (stocke les IDs d'entités copiées)
    std::vector<NovaEngine::u64> m_clipboard;

    // Placement
    std::string m_placingEntityType;

    // Grid
    bool m_snapToGrid;
    NovaEngine::f32 m_gridSize;
    NovaEngine::i32 m_currentLayer;

    // Visibilité
    bool m_showGrid;
    bool m_showGizmos;
    bool m_showColliderBounds;
    bool m_showLightRadii;

    // Scènes récentes
    std::vector<std::string> m_recentScenes;

    // Camera bookmarks
    std::vector<CameraBookmark> m_cameraBookmarks;
};

} // namespace NovaEditor
