#pragma once

#include <NovaEngine/Core/Types.hpp>
#include <NovaEngine/ECS/Entity.hpp>
#include <string>

namespace NovaEditor {

/**
 * @brief Centralized state management for the editor
 *
 * Manages current mode, selected entity, active tool, and placement state
 */
class EditorState {
public:
    enum class Mode {
        Select,    // Select and manipulate existing entities
        Place,     // Place new entities from palette
        Paint,     // Paint tiles (future)
        Erase      // Delete entities
    };

    enum class Tool {
        Move,      // Move entities
        Rotate,    // Rotate entities
        Scale,     // Scale entities
        Light,     // Edit light properties
        Activator  // Edit activator zones
    };

    EditorState();
    ~EditorState() = default;

    // Selection
    NovaEngine::Entity* getSelectedEntity() const { return m_selectedEntity; }
    void setSelectedEntity(NovaEngine::Entity* entity);
    void clearSelection();
    bool hasSelection() const { return m_selectedEntity != nullptr; }

    // Mode
    Mode getMode() const { return m_currentMode; }
    void setMode(Mode mode);
    const char* getModeName() const;

    // Tool
    Tool getTool() const { return m_currentTool; }
    void setTool(Tool tool);
    const char* getToolName() const;

    // Placement (for Place mode)
    const std::string& getPlacementType() const { return m_placementType; }
    void setPlacementType(const std::string& type);
    bool isPlacementActive() const { return !m_placementType.empty(); }

    // Scene state
    bool isSceneModified() const { return m_sceneModified; }
    void setSceneModified(bool modified) { m_sceneModified = modified; }

    const std::string& getCurrentScenePath() const { return m_currentScenePath; }
    void setCurrentScenePath(const std::string& path) { m_currentScenePath = path; }

    // Grid
    bool isGridEnabled() const { return m_gridEnabled; }
    void setGridEnabled(bool enabled) { m_gridEnabled = enabled; }
    NovaEngine::f32 getGridSize() const { return m_gridSize; }
    void setGridSize(NovaEngine::f32 size) { m_gridSize = size; }

    // Layer (for z-ordering)
    NovaEngine::i32 getCurrentLayer() const { return m_currentLayer; }
    void setCurrentLayer(NovaEngine::i32 layer) { m_currentLayer = layer; }

private:
    // Selection
    NovaEngine::Entity* m_selectedEntity;

    // Mode and tool
    Mode m_currentMode;
    Tool m_currentTool;

    // Placement
    std::string m_placementType;  // e.g., "sprite:tree", "light:point"

    // Scene state
    bool m_sceneModified;
    std::string m_currentScenePath;

    // Grid
    bool m_gridEnabled;
    NovaEngine::f32 m_gridSize;

    // Layer
    NovaEngine::i32 m_currentLayer;
};

} // namespace NovaEditor
