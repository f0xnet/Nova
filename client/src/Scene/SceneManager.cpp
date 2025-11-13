#include "NovaEngine/Scene/SceneManager.hpp"

namespace NovaEngine {
namespace Scene {

    SceneManager::SceneManager()
        : m_activeScene(nullptr),
          m_transitioning(false),
          m_transitionProgress(0.0f),
          m_transitionTargetScene(nullptr) {
        LOG_TRACE("SceneManager created");
    }

    SceneManager::~SceneManager() {
        clear();
        LOG_TRACE("SceneManager destroyed");
    }

    void SceneManager::addScene(ScenePtr scene, bool load) {
        if (!scene) {
            LOG_ERROR("Cannot add null scene");
            return;
        }

        const String& name = scene->getName();

        if (m_scenes.find(name) != m_scenes.end()) {
            LOG_WARN("Scene {} already exists, replacing", name);
        }

        m_scenes[name] = scene;
        LOG_DEBUG("Scene added: {}", name);

        if (load) {
            loadScene(name);
        }
    }

    void SceneManager::removeScene(const String& sceneName) {
        auto it = m_scenes.find(sceneName);
        if (it == m_scenes.end()) {
            LOG_WARN("Scene {} not found", sceneName);
            return;
        }

        ScenePtr scene = it->second;

        // Si c'est la scène active, la désactiver
        if (m_activeScene == scene) {
            scene->setActive(false);
            scene->onExit();
            m_activeScene = nullptr;
        }

        // Décharger si nécessaire
        if (scene->isLoaded()) {
            scene->onUnload();
            scene->setLoaded(false);
        }

        m_scenes.erase(it);
        LOG_DEBUG("Scene removed: {}", sceneName);
    }

    void SceneManager::loadScene(const String& sceneName) {
        auto it = m_scenes.find(sceneName);
        if (it == m_scenes.end()) {
            LOG_ERROR("Scene {} not found", sceneName);
            return;
        }

        ScenePtr scene = it->second;

        if (scene->isLoaded()) {
            LOG_WARN("Scene {} already loaded", sceneName);
            return;
        }

        LOG_INFO("Loading scene: {}", sceneName);
        scene->onLoad();
        scene->setLoaded(true);
    }

    void SceneManager::unloadScene(const String& sceneName) {
        auto it = m_scenes.find(sceneName);
        if (it == m_scenes.end()) {
            LOG_ERROR("Scene {} not found", sceneName);
            return;
        }

        ScenePtr scene = it->second;

        if (!scene->isLoaded()) {
            LOG_WARN("Scene {} not loaded", sceneName);
            return;
        }

        // Ne pas décharger la scène active
        if (scene == m_activeScene) {
            LOG_ERROR("Cannot unload active scene: {}", sceneName);
            return;
        }

        LOG_INFO("Unloading scene: {}", sceneName);
        scene->onUnload();
        scene->setLoaded(false);
    }

    void SceneManager::changeScene(const String& sceneName, const SceneTransitionData& transitionData) {
        auto it = m_scenes.find(sceneName);
        if (it == m_scenes.end()) {
            LOG_ERROR("Scene {} not found", sceneName);
            return;
        }

        ScenePtr targetScene = it->second;

        // Charger la scène si nécessaire
        if (!targetScene->isLoaded()) {
            loadScene(sceneName);
        }

        // Si on a une scène active, créer la transition
        if (m_activeScene) {
            SceneTransitionData data = transitionData;
            data.fromScene = m_activeScene->getName();
            data.toScene = sceneName;
            m_transitionTargetScene = targetScene;
            beginTransition(data);
        } else {
            // Pas de transition, changement direct
            m_activeScene = targetScene;
            m_activeScene->setActive(true);
            m_activeScene->onEnter();
            LOG_INFO("Scene changed to: {}", sceneName);
        }
    }

    void SceneManager::pushScene(const String& sceneName) {
        auto it = m_scenes.find(sceneName);
        if (it == m_scenes.end()) {
            LOG_ERROR("Scene {} not found", sceneName);
            return;
        }

        ScenePtr scene = it->second;

        // Charger si nécessaire
        if (!scene->isLoaded()) {
            loadScene(sceneName);
        }

        // Mettre l'ancienne scène en pause
        if (m_activeScene) {
            m_sceneStack.push(m_activeScene);
            m_activeScene->setPaused(true);
            m_activeScene->onExit();
        }

        // Activer la nouvelle scène
        m_activeScene = scene;
        m_activeScene->setActive(true);
        m_activeScene->onEnter();

        LOG_INFO("Scene pushed: {}", sceneName);
    }

    void SceneManager::popScene() {
        if (m_sceneStack.empty()) {
            LOG_WARN("Scene stack is empty, cannot pop");
            return;
        }

        // Désactiver la scène actuelle
        if (m_activeScene) {
            m_activeScene->setActive(false);
            m_activeScene->onExit();
        }

        // Récupérer la scène précédente
        m_activeScene = m_sceneStack.top();
        m_sceneStack.pop();

        // Réactiver
        m_activeScene->setPaused(false);
        m_activeScene->onEnter();

        LOG_INFO("Scene popped, returning to: {}", m_activeScene->getName());
    }

    ScenePtr SceneManager::getScene(const String& sceneName) {
        auto it = m_scenes.find(sceneName);
        if (it != m_scenes.end()) {
            return it->second;
        }
        return nullptr;
    }

    ScenePtr SceneManager::getActiveScene() {
        return m_activeScene;
    }

    const ScenePtr SceneManager::getActiveScene() const {
        return m_activeScene;
    }

    bool SceneManager::hasScene(const String& sceneName) const {
        return m_scenes.find(sceneName) != m_scenes.end();
    }

    void SceneManager::update(f32 deltaTime) {
        // Mettre à jour la transition si active
        if (m_transitioning) {
            updateTransition(deltaTime);
        }

        // Mettre à jour la scène active
        if (m_activeScene && !m_transitioning) {
            m_activeScene->update(deltaTime);
        }
    }

    void SceneManager::render() {
        if (m_activeScene) {
            m_activeScene->render();
        }

        // TODO: Ajouter des effets de transition (fade, etc.)
        if (m_transitioning) {
            // Dessiner un overlay de transition si nécessaire
        }
    }

    void SceneManager::handleEvent(const InputEvent& event) {
        if (m_activeScene && !m_transitioning) {
            m_activeScene->handleEvent(event);
        }
    }

    void SceneManager::setTransitionCallback(SceneTransitionCallback callback) {
        m_transitionCallback = callback;
    }

    bool SceneManager::isTransitioning() const {
        return m_transitioning;
    }

    f32 SceneManager::getTransitionProgress() const {
        return m_transitionProgress;
    }

    void SceneManager::clear() {
        LOG_INFO("Clearing all scenes");

        // Désactiver la scène active
        if (m_activeScene) {
            m_activeScene->setActive(false);
            m_activeScene->onExit();
        }

        // Décharger toutes les scènes
        for (auto& [name, scene] : m_scenes) {
            if (scene->isLoaded()) {
                scene->onUnload();
                scene->setLoaded(false);
            }
        }

        m_scenes.clear();
        m_activeScene = nullptr;

        // Vider la pile
        while (!m_sceneStack.empty()) {
            m_sceneStack.pop();
        }

        m_transitioning = false;
        m_transitionTargetScene = nullptr;
    }

    size_t SceneManager::getSceneCount() const {
        return m_scenes.size();
    }

    std::vector<String> SceneManager::getSceneNames() const {
        std::vector<String> names;
        for (const auto& [name, scene] : m_scenes) {
            names.push_back(name);
        }
        return names;
    }

    void SceneManager::updateTransition(f32 deltaTime) {
        if (!m_transitioning) return;

        m_transitionProgress += deltaTime / m_currentTransition.duration;

        if (m_transitionProgress >= 1.0f) {
            endTransition();
        }
    }

    void SceneManager::beginTransition(const SceneTransitionData& data) {
        m_transitioning = true;
        m_transitionProgress = 0.0f;
        m_currentTransition = data;

        LOG_INFO("Beginning transition from {} to {}", data.fromScene, data.toScene);

        if (m_transitionCallback) {
            m_transitionCallback(data);
        }
    }

    void SceneManager::endTransition() {
        m_transitioning = false;
        m_transitionProgress = 1.0f;

        // Désactiver l'ancienne scène
        if (m_activeScene) {
            m_activeScene->setActive(false);
            m_activeScene->onExit();
        }

        // Activer la nouvelle scène
        m_activeScene = m_transitionTargetScene;
        if (m_activeScene) {
            m_activeScene->setActive(true);
            m_activeScene->onEnter();
        }

        m_transitionTargetScene = nullptr;

        LOG_INFO("Transition completed to: {}", m_activeScene ? m_activeScene->getName() : "none");
    }

} // namespace Scene
} // namespace NovaEngine
