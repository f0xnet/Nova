#pragma once

#include "Scene.hpp"
#include "DefinitionManager.hpp"
#include "SceneGraph.hpp"
#include "../Backend/Core/BackendTypes.hpp"
#include "../Core/Logger.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <fstream>

namespace NovaEngine {

/**
 * @brief Manages multiple scenes and entity definitions
 *
 * The SceneManager is responsible for:
 * 1. Loading entity definitions at startup (once)
 * 2. Loading scene files (on demand)
 * 3. Switching between active scenes
 * 4. Updating and rendering the active scene
 */
class SceneManager {
private:
    DefinitionManager m_definitionManager;
    SceneGraph m_sceneGraph;
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    Scene* m_activeScene = nullptr;
    std::unordered_set<std::string> m_activeScenesForUpdate;  // Scènes à mettre à jour

public:
    /**
     * @brief Initialize the scene manager
     *
     * This loads all entity definitions from JSON files.
     * Call this once at application startup.
     *
     * @param definitionsPath Path to definitions folder (default: "assets/data/definitions/")
     * @return true if successful
     */
    bool initialize(const std::string& definitionsPath = "assets/data/definitions/",
                   const std::string& sceneGraphPath = "assets/data/scenegraph.json") {
        LOG_INFO("Initializing SceneManager...");

        // Load all entity definitions into memory
        if (!m_definitionManager.loadDefinitions(definitionsPath)) {
            LOG_ERROR("Failed to load entity definitions");
            return false;
        }

        // Load scene graph for multi-scene pathfinding
        if (!m_sceneGraph.loadFromJSON(sceneGraphPath)) {
            LOG_WARN("Failed to load scene graph - multi-scene travel will be limited");
        }

        LOG_INFO("SceneManager initialized successfully");
        return true;
    }

    /**
     * @brief Load a scene from a JSON file
     *
     * The scene is loaded and stored, but not activated.
     * Use setActiveScene() to activate it.
     *
     * @param scenePath Path to the scene JSON file
     * @param sceneName Unique name for this scene
     * @return true if successful
     */
    bool loadScene(const std::string& scenePath, const std::string& sceneName) {
        LOG_INFO("Loading scene '{}' from: {}", sceneName, scenePath);

        // Check if already loaded
        if (m_scenes.find(sceneName) != m_scenes.end()) {
            LOG_WARN("Scene '{}' is already loaded", sceneName);
            return true;
        }

        // Open scene file
        std::ifstream file(scenePath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open scene file: {}", scenePath);
            return false;
        }

        // Parse JSON
        nlohmann::json sceneData;
        try {
            file >> sceneData;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse scene JSON: {}", e.what());
            return false;
        }

        // Create scene
        auto scene = std::make_unique<Scene>(sceneName);

        // Load scene data
        if (!scene->loadFromJSON(sceneData, m_definitionManager)) {
            LOG_ERROR("Failed to load scene '{}'", sceneName);
            return false;
        }

        // Store scene
        m_scenes[sceneName] = std::move(scene);
        LOG_INFO("Scene '{}' loaded successfully", sceneName);
        return true;
    }

    /**
     * @brief Unload a scene from memory
     *
     * If the scene is currently active, it will be deactivated.
     *
     * @param sceneName Name of the scene to unload
     */
    void unloadScene(const std::string& sceneName) {
        auto it = m_scenes.find(sceneName);
        if (it != m_scenes.end()) {
            if (m_activeScene == it->second.get()) {
                m_activeScene = nullptr;
            }
            m_scenes.erase(it);
            LOG_INFO("Unloaded scene '{}'", sceneName);
        } else {
            LOG_WARN("Tried to unload non-existent scene: {}", sceneName);
        }
    }

    /**
     * @brief Set the active scene
     *
     * Only one scene can be active at a time.
     * The active scene is updated and rendered.
     *
     * @param sceneName Name of the scene to activate
     */
    void setActiveScene(const std::string& sceneName) {
        auto it = m_scenes.find(sceneName);
        if (it != m_scenes.end()) {
            m_activeScene = it->second.get();
            LOG_INFO("Set active scene to '{}'", sceneName);
        } else {
            LOG_ERROR("Cannot set active scene: '{}' not found", sceneName);
        }
    }

    /**
     * @brief Get the active scene
     * @return Pointer to active scene, or nullptr if none
     */
    Scene* getActiveScene() {
        return m_activeScene;
    }

    /**
     * @brief Get the active scene (const)
     */
    const Scene* getActiveScene() const {
        return m_activeScene;
    }

    /**
     * @brief Get a scene by name
     * @param sceneName Name of the scene
     * @return Pointer to scene, or nullptr if not found
     */
    Scene* getScene(const std::string& sceneName) {
        auto it = m_scenes.find(sceneName);
        return (it != m_scenes.end()) ? it->second.get() : nullptr;
    }

    /**
     * @brief Get a scene by name (const)
     */
    const Scene* getScene(const std::string& sceneName) const {
        auto it = m_scenes.find(sceneName);
        return (it != m_scenes.end()) ? it->second.get() : nullptr;
    }

    /**
     * @brief Check if a scene is loaded
     */
    bool hasScene(const std::string& sceneName) const {
        return m_scenes.find(sceneName) != m_scenes.end();
    }

    /**
     * @brief Get the number of loaded scenes
     */
    size_t getSceneCount() const {
        return m_scenes.size();
    }

    /**
     * @brief Get the name of the currently active scene
     */
    std::string getActiveSceneName() const {
        if (!m_activeScene) return "";
        for (const auto& [name, scene] : m_scenes) {
            if (scene.get() == m_activeScene) return name;
        }
        return "";
    }

    /**
     * @brief Update all active scenes
     *
     * Call this every frame to update all systems in active scenes.
     * Active scenes include the rendered scene AND any scenes with NPCs traveling through.
     *
     * @param deltaTime Time since last frame in seconds
     */
    void update(float deltaTime) {
        // Collecter toutes les scènes sur les parcours actifs
        std::unordered_set<std::string> scenesOnActivePaths;

        // Parcourir toutes les scènes pour trouver les voyageurs
        for (auto& [sceneName, scene] : m_scenes) {
            if (!scene) continue;

            // Trouver tous les PNJ en voyage dans cette scène
            auto travelers = scene->getEntityRegistry().getEntitiesWith({"JourneyComponent"});

            for (Entity* entity : travelers) {
                auto* journey = entity->getComponent<JourneyComponent>();
                if (journey && journey->isOnJourney) {
                    // Activer TOUTES les scènes sur le parcours
                    for (const auto& sceneOnPath : journey->scenePath) {
                        scenesOnActivePaths.insert(sceneOnPath);
                    }
                }
            }
        }

        // Update les scènes actives
        for (auto& [name, scene] : m_scenes) {
            if (!scene) continue;

            bool isRenderedScene = (m_activeScene == scene.get());
            bool isOnActivePath = (scenesOnActivePaths.find(name) != scenesOnActivePaths.end());

            if (isRenderedScene) {
                // Scène du joueur: Full update
                scene->update(deltaTime);
            }
            else if (isOnActivePath) {
                // Scène sur un parcours actif: Update aussi (pour que les NPCs traversent)
                scene->update(deltaTime);
                LOG_DEBUG("Scene '{}' updated (NPC traveling through)", name);
            }
            // Sinon: scène dormante, pas d'update
        }
    }

    /**
     * @brief Render the active scene
     *
     * Call this every frame to render the active scene.
     */
    void render() {
        if (m_activeScene) {
            m_activeScene->render();
        }
    }

    /**
     * @brief Get the definition manager
     *
     * This allows external code to query entity definitions.
     */
    const DefinitionManager& getDefinitionManager() const {
        return m_definitionManager;
    }

    /**
     * @brief Get the scene graph
     *
     * This allows external code to query scene connections.
     */
    SceneGraph& getSceneGraph() {
        return m_sceneGraph;
    }

    const SceneGraph& getSceneGraph() const {
        return m_sceneGraph;
    }

    /**
     * @brief Clear all scenes
     *
     * Unloads all scenes from memory.
     */
    void clearScenes() {
        m_scenes.clear();
        m_activeScene = nullptr;
        LOG_INFO("Cleared all scenes");
    }

    /**
     * @brief Shutdown the scene manager
     *
     * Cleans up all scenes and definitions.
     */
    void shutdown() {
        LOG_INFO("Shutting down SceneManager...");
        clearScenes();
        LOG_INFO("SceneManager shut down");
    }
};

} // namespace NovaEngine
