#include "NovaEngine/Scene/Scene.hpp"

namespace NovaEngine {
namespace Scene {

    Scene::Scene(const String& name, SceneType type)
        : m_name(name),
          m_type(type),
          m_loaded(false),
          m_active(false),
          m_paused(false),
          m_world(std::make_unique<ECS::World>()) {
        LOG_TRACE("Scene created: {}", name);
    }

    Scene::~Scene() {
        LOG_TRACE("Scene destroyed: {}", m_name);
        if (m_loaded) {
            onUnload();
        }
    }

    void Scene::update(f32 deltaTime) {
        if (!m_active || m_paused) return;

        if (m_world) {
            m_world->update(deltaTime);
        }
    }

    void Scene::render() {
        if (!m_active) return;

        if (m_world) {
            m_world->render();
        }
    }

    ECS::World& Scene::getWorld() {
        return *m_world;
    }

    const ECS::World& Scene::getWorld() const {
        return *m_world;
    }

    const String& Scene::getName() const {
        return m_name;
    }

    SceneType Scene::getType() const {
        return m_type;
    }

    bool Scene::isLoaded() const {
        return m_loaded;
    }

    bool Scene::isActive() const {
        return m_active;
    }

    void Scene::setPaused(bool paused) {
        m_paused = paused;
    }

    bool Scene::isPaused() const {
        return m_paused;
    }

    void Scene::setMetadata(const String& key, const String& value) {
        m_metadata[key] = value;
    }

    String Scene::getMetadata(const String& key, const String& defaultValue) const {
        auto it = m_metadata.find(key);
        if (it != m_metadata.end()) {
            return it->second;
        }
        return defaultValue;
    }

    void Scene::setLoaded(bool loaded) {
        m_loaded = loaded;
    }

    void Scene::setActive(bool active) {
        m_active = active;
    }

} // namespace Scene
} // namespace NovaEngine
