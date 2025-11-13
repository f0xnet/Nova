#include "NovaEngine/ECS/System.hpp"
#include "NovaEngine/ECS/World.hpp"

namespace NovaEngine {
namespace ECS {

    void System::setEnabled(bool enabled) {
        m_enabled = enabled;
    }

    bool System::isEnabled() const {
        return m_enabled;
    }

    void System::setPriority(i32 priority) {
        m_priority = priority;
    }

    i32 System::getPriority() const {
        return m_priority;
    }

    const std::set<std::type_index>& System::getRequiredComponents() const {
        return m_requiredComponents;
    }

    std::vector<Entity> System::getEntities() const {
        if (!m_world) {
            return {};
        }

        std::vector<Entity> result;

        // Cette méthode sera implémentée plus efficacement dans World
        // Pour l'instant, on retourne une liste vide
        // L'implémentation complète nécessite un accès plus profond au World

        return result;
    }

} // namespace ECS
} // namespace NovaEngine
