#include "NovaEngine/Scene/InteriorScene.hpp"

namespace NovaEngine {
namespace Scene {

    InteriorScene::InteriorScene(const String& name, const String& interiorType)
        : Scene(name, SceneType::Interior),
          m_interiorType(interiorType),
          m_bounds(0, 0, 1000, 1000),
          m_ambientLight(1.0f) {
        LOG_TRACE("InteriorScene created: {} (type: {})", name, interiorType);
    }

    void InteriorScene::addTransition(const SceneTransition& transition) {
        // Vérifier si une transition avec cet ID existe déjà
        for (auto& existing : m_transitions) {
            if (existing.id == transition.id) {
                LOG_WARN("Transition with ID {} already exists, replacing", transition.id);
                existing = transition;
                return;
            }
        }

        m_transitions.push_back(transition);
        LOG_DEBUG("Transition added: {} -> {}", transition.id, transition.targetScene);
    }

    void InteriorScene::removeTransition(const String& transitionId) {
        auto it = std::remove_if(m_transitions.begin(), m_transitions.end(),
            [&transitionId](const SceneTransition& t) {
                return t.id == transitionId;
            });

        if (it != m_transitions.end()) {
            m_transitions.erase(it, m_transitions.end());
            LOG_DEBUG("Transition removed: {}", transitionId);
        }
    }

    SceneTransition* InteriorScene::getTransition(const String& transitionId) {
        for (auto& transition : m_transitions) {
            if (transition.id == transitionId) {
                return &transition;
            }
        }
        return nullptr;
    }

    const std::vector<SceneTransition>& InteriorScene::getTransitions() const {
        return m_transitions;
    }

    const SceneTransition* InteriorScene::checkTransitionTrigger(const Vec2f& position) const {
        for (const auto& transition : m_transitions) {
            if (transition.enabled && transition.triggerZone.contains(position)) {
                return &transition;
            }
        }
        return nullptr;
    }

    void InteriorScene::setBounds(const Rect& bounds) {
        m_bounds = bounds;
    }

    const Rect& InteriorScene::getBounds() const {
        return m_bounds;
    }

    void InteriorScene::setInteriorType(const String& type) {
        m_interiorType = type;
    }

    const String& InteriorScene::getInteriorType() const {
        return m_interiorType;
    }

    void InteriorScene::setAmbientLight(f32 light) {
        m_ambientLight = std::max(0.0f, std::min(1.0f, light));
    }

    f32 InteriorScene::getAmbientLight() const {
        return m_ambientLight;
    }

    void InteriorScene::onEnter() {
        LOG_INFO("Entering interior scene: {}", getName());
    }

    void InteriorScene::onExit() {
        LOG_INFO("Exiting interior scene: {}", getName());
    }

    void InteriorScene::update(f32 deltaTime) {
        Scene::update(deltaTime);

        // Logique spécifique aux intérieurs peut être ajoutée ici
    }

    void InteriorScene::handleEvent(const InputEvent& event) {
        // Gestion des événements spécifiques aux intérieurs
    }

} // namespace Scene
} // namespace NovaEngine
