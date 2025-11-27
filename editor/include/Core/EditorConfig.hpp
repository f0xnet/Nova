#pragma once

#include <NovaEngine/Core/Types.hpp>
#include <string>

namespace NovaEditor {

/**
 * @brief Configuration and settings for the editor
 *
 * Stores editor preferences, paths, and constants
 */
class EditorConfig {
public:
    EditorConfig();
    ~EditorConfig() = default;

    // Paths
    const std::string& getDataPath() const { return m_dataPath; }
    const std::string& getUIPath() const { return m_uiPath; }
    const std::string& getScenesPath() const { return m_scenesPath; }
    const std::string& getDefinitionsPath() const { return m_definitionsPath; }

    void setDataPath(const std::string& path);

    // Grid settings
    NovaEngine::f32 getDefaultGridSize() const { return m_defaultGridSize; }
    void setDefaultGridSize(NovaEngine::f32 size) { m_defaultGridSize = size; }

    bool isGridEnabledByDefault() const { return m_gridEnabledByDefault; }
    void setGridEnabledByDefault(bool enabled) { m_gridEnabledByDefault = enabled; }

    // Camera settings
    NovaEngine::f32 getCameraMovementSpeed() const { return m_cameraMovementSpeed; }
    void setCameraMovementSpeed(NovaEngine::f32 speed) { m_cameraMovementSpeed = speed; }

    NovaEngine::f32 getCameraZoomSpeed() const { return m_cameraZoomSpeed; }
    void setCameraZoomSpeed(NovaEngine::f32 speed) { m_cameraZoomSpeed = speed; }

    NovaEngine::f32 getCameraMinZoom() const { return m_cameraMinZoom; }
    NovaEngine::f32 getCameraMaxZoom() const { return m_cameraMaxZoom; }
    void setCameraZoomLimits(NovaEngine::f32 minZoom, NovaEngine::f32 maxZoom);

    // UI settings
    const std::string& getDefaultFont() const { return m_defaultFont; }
    void setDefaultFont(const std::string& fontPath) { m_defaultFont = fontPath; }

    NovaEngine::u32 getWindowWidth() const { return m_windowWidth; }
    NovaEngine::u32 getWindowHeight() const { return m_windowHeight; }
    void setWindowSize(NovaEngine::u32 width, NovaEngine::u32 height);

    // Auto-save settings
    bool isAutoSaveEnabled() const { return m_autoSaveEnabled; }
    void setAutoSaveEnabled(bool enabled) { m_autoSaveEnabled = enabled; }

    NovaEngine::f32 getAutoSaveInterval() const { return m_autoSaveInterval; }
    void setAutoSaveInterval(NovaEngine::f32 seconds) { m_autoSaveInterval = seconds; }

    // History settings
    NovaEngine::u32 getMaxHistorySize() const { return m_maxHistorySize; }
    void setMaxHistorySize(NovaEngine::u32 size) { m_maxHistorySize = size; }

    // Load/Save configuration
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path) const;

private:
    // Paths
    std::string m_dataPath;
    std::string m_uiPath;
    std::string m_scenesPath;
    std::string m_definitionsPath;

    // Grid
    NovaEngine::f32 m_defaultGridSize;
    bool m_gridEnabledByDefault;

    // Camera
    NovaEngine::f32 m_cameraMovementSpeed;
    NovaEngine::f32 m_cameraZoomSpeed;
    NovaEngine::f32 m_cameraMinZoom;
    NovaEngine::f32 m_cameraMaxZoom;

    // UI
    std::string m_defaultFont;
    NovaEngine::u32 m_windowWidth;
    NovaEngine::u32 m_windowHeight;

    // Auto-save
    bool m_autoSaveEnabled;
    NovaEngine::f32 m_autoSaveInterval;  // in seconds

    // History
    NovaEngine::u32 m_maxHistorySize;

    void initializeDefaults();
};

} // namespace NovaEditor
