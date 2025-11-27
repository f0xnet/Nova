#include "Core/EditorConfig.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <fstream>

namespace NovaEditor {

EditorConfig::EditorConfig() {
    initializeDefaults();
    LOG_INFO("EditorConfig initialized with default values");
}

void EditorConfig::initializeDefaults() {
    // Paths
    m_dataPath = "data";
    m_uiPath = "editor/assets/ui";
    m_scenesPath = "data/scenes";
    m_definitionsPath = "data/definitions";

    // Grid
    m_defaultGridSize = 32.0f;
    m_gridEnabledByDefault = true;

    // Camera
    m_cameraMovementSpeed = 500.0f;
    m_cameraZoomSpeed = 0.1f;
    m_cameraMinZoom = 0.1f;
    m_cameraMaxZoom = 5.0f;

    // UI
    m_defaultFont = "C:/Windows/Fonts/arial.ttf";
    m_windowWidth = 1920;
    m_windowHeight = 1080;

    // Auto-save
    m_autoSaveEnabled = true;
    m_autoSaveInterval = 300.0f;  // 5 minutes

    // History
    m_maxHistorySize = 50;
}

void EditorConfig::setDataPath(const std::string& path) {
    m_dataPath = path;
    m_scenesPath = path + "/scenes";
    m_definitionsPath = path + "/definitions";
    LOG_INFO("Data path set to: {}", path);
}

void EditorConfig::setCameraZoomLimits(NovaEngine::f32 minZoom, NovaEngine::f32 maxZoom) {
    m_cameraMinZoom = minZoom;
    m_cameraMaxZoom = maxZoom;
    LOG_DEBUG("Camera zoom limits set: [{}, {}]", minZoom, maxZoom);
}

void EditorConfig::setWindowSize(NovaEngine::u32 width, NovaEngine::u32 height) {
    m_windowWidth = width;
    m_windowHeight = height;
    LOG_DEBUG("Window size set to {}x{}", width, height);
}

bool EditorConfig::loadFromFile(const std::string& path) {
    // TODO: Implement JSON loading when needed
    LOG_WARN("EditorConfig::loadFromFile not yet implemented");
    return false;
}

bool EditorConfig::saveToFile(const std::string& path) const {
    // TODO: Implement JSON saving when needed
    LOG_WARN("EditorConfig::saveToFile not yet implemented");
    return false;
}

} // namespace NovaEditor
