#pragma once

#include "EditorState.hpp"
#include <NovaEngine/ECS/Scene.hpp>
#include <NovaEngine/ECS/SceneManager.hpp>
#include <NovaEngine/Core/Logger.hpp>
#include <string>
#include <vector>
#include <filesystem>
#include <functional>

namespace NovaEditor {

/**
 * @brief Controller for scene editing operations
 *
 * Responsibilities:
 * - Create new scenes
 * - Load existing scenes
 * - Save scenes
 * - List available scenes
 *
 * Benefits:
 * - Single Responsibility: Only handles scene operations
 * - Encapsulates file system operations
 * - Clear interface for scene management
 */
class SceneEditorController {
public:
    using OnSceneLoadedCallback = std::function<void(NovaEngine::Scene*)>;

    SceneEditorController(
        EditorState* editorState,
        NovaEngine::SceneManager* sceneManager,
        OnSceneLoadedCallback onSceneLoaded = nullptr
    )
        : m_editorState(editorState)
        , m_sceneManager(sceneManager)
        , m_onSceneLoaded(onSceneLoaded)
    {}

    /**
     * @brief Create a new empty scene
     */
    NovaEngine::Scene* createNewScene() {
        LOG_INFO("Creating new scene...");

        // Create empty scene
        NovaEngine::Scene* newScene = new NovaEngine::Scene("UntitledScene");

        // Reset editor state
        m_editorState->setSceneModified(false);
        m_editorState->setSelectedEntity(nullptr);
        m_editorState->setCurrentScenePath("");

        LOG_INFO("New scene created successfully");

        if (m_onSceneLoaded) {
            m_onSceneLoaded(newScene);
        }

        return newScene;
    }

    /**
     * @brief Load scene from file path
     */
    NovaEngine::Scene* loadScene(const std::string& scenePath) {
        LOG_INFO("Loading scene from: {}", scenePath);

        if (!std::filesystem::exists(scenePath)) {
            LOG_ERROR("Scene file not found: {}", scenePath);
            return nullptr;
        }

        NovaEngine::Scene* scene = m_sceneManager->loadScene(scenePath);

        if (scene) {
            // Update editor state
            m_editorState->setCurrentScenePath(scenePath);
            m_editorState->setSceneModified(false);
            m_editorState->setSelectedEntity(nullptr);

            LOG_INFO("Scene loaded successfully: {} entities",
                     scene->getEntityRegistry().getEntityCount());

            if (m_onSceneLoaded) {
                m_onSceneLoaded(scene);
            }

            return scene;
        } else {
            LOG_ERROR("Failed to load scene from: {}", scenePath);
            return nullptr;
        }
    }

    /**
     * @brief Save current scene
     */
    bool saveScene(NovaEngine::Scene* scene) {
        if (!scene) {
            LOG_ERROR("Cannot save: no scene loaded");
            return false;
        }

        std::string scenePath = m_editorState->getCurrentScenePath();

        // If no path set, prompt for one (handled by UI)
        if (scenePath.empty()) {
            LOG_WARN("No scene path set - save cancelled");
            return false;
        }

        return saveSceneAs(scene, scenePath);
    }

    /**
     * @brief Save scene to specific path
     */
    bool saveSceneAs(NovaEngine::Scene* scene, const std::string& scenePath) {
        if (!scene) {
            LOG_ERROR("Cannot save: no scene loaded");
            return false;
        }

        LOG_INFO("Saving scene to: {}", scenePath);

        bool success = m_sceneManager->saveScene(scene, scenePath);

        if (success) {
            m_editorState->setCurrentScenePath(scenePath);
            m_editorState->setSceneModified(false);
            LOG_INFO("Scene saved successfully");
        } else {
            LOG_ERROR("Failed to save scene to: {}", scenePath);
        }

        return success;
    }

    /**
     * @brief Get list of available scenes
     */
    std::vector<std::string> getAvailableScenes() const {
        std::vector<std::string> scenes;
        std::string scenesDir = "editor/data/scenes";

        try {
            if (std::filesystem::exists(scenesDir)) {
                for (const auto& entry : std::filesystem::directory_iterator(scenesDir)) {
                    if (entry.path().extension() == ".json") {
                        scenes.push_back(entry.path().string());
                    }
                }
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to list scenes: {}", e.what());
        }

        LOG_DEBUG("Found {} available scenes", scenes.size());
        return scenes;
    }

    /**
     * @brief Check if current scene has unsaved changes
     */
    bool hasUnsavedChanges() const {
        return m_editorState->isSceneModified();
    }

private:
    EditorState* m_editorState;
    NovaEngine::SceneManager* m_sceneManager;
    OnSceneLoadedCallback m_onSceneLoaded;
};

} // namespace NovaEditor
