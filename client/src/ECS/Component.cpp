#include "NovaEngine/ECS/Component.hpp"

namespace NovaEngine {
namespace ECS {

    void Component::setEnabled(bool enabled) {
        if (m_enabled == enabled) return;

        m_enabled = enabled;

        if (enabled) {
            onEnable();
        } else {
            onDisable();
        }
    }

    bool Component::isEnabled() const {
        return m_enabled;
    }

    Entity Component::getOwner() const {
        return m_owner;
    }

} // namespace ECS
} // namespace NovaEngine
