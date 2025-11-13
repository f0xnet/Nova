#include "NovaEngine/ECS/Entity.hpp"
#include "NovaEngine/ECS/World.hpp"

namespace NovaEngine {
namespace ECS {

    Entity::Entity()
        : m_handle{0, 0}, m_world(nullptr) {
    }

    Entity::Entity(EntityHandle handle, World* world)
        : m_handle(handle), m_world(world) {
    }

    bool Entity::isValid() const {
        return m_world != nullptr && m_world->isEntityValid(m_handle);
    }

    EntityID Entity::getID() const {
        return m_handle.id;
    }

    EntityHandle Entity::getHandle() const {
        return m_handle;
    }

    EntityVersion Entity::getVersion() const {
        return m_handle.version;
    }

    void Entity::destroy() {
        if (isValid()) {
            m_world->destroyEntity(*this);
        }
    }

    void Entity::setActive(bool active) {
        if (isValid()) {
            m_world->setEntityActive(m_handle, active);
        }
    }

    bool Entity::isActive() const {
        if (isValid()) {
            return m_world->isEntityActive(m_handle);
        }
        return false;
    }

    void Entity::setTag(const String& tag) {
        if (isValid()) {
            m_world->setEntityTag(m_handle, tag);
        }
    }

    const String& Entity::getTag() const {
        if (isValid()) {
            return m_world->getEntityTag(m_handle);
        }
        static String emptyTag = "";
        return emptyTag;
    }

    bool Entity::operator==(const Entity& other) const {
        return m_handle == other.m_handle && m_world == other.m_world;
    }

    bool Entity::operator!=(const Entity& other) const {
        return !(*this == other);
    }

} // namespace ECS
} // namespace NovaEngine
