#include "NovaEngine/ECS/World.hpp"

namespace NovaEngine {
namespace ECS {

    World::World()
        : m_nextEntityID(1) {
        LOG_TRACE("World created");
    }

    World::~World() {
        clear();
        LOG_TRACE("World destroyed");
    }

    Entity World::createEntity(const String& tag) {
        EntityID id = m_nextEntityID++;
        EntityVersion version = 1;

        // Si l'ID a été utilisé avant, incrémenter sa version
        auto versionIt = m_entityVersions.find(id);
        if (versionIt != m_entityVersions.end()) {
            version = versionIt->second + 1;
            m_entityVersions[id] = version;
        } else {
            m_entityVersions[id] = version;
        }

        EntityHandle handle{id, version};

        EntityData data;
        data.handle = handle;
        data.active = true;
        data.tag = tag;

        m_entities[id] = std::move(data);

        if (!tag.empty()) {
            m_tagIndex[tag].push_back(id);
        }

        LOG_TRACE("Entity created: {} (tag: {})", id, tag);
        return Entity(handle, this);
    }

    void World::destroyEntity(Entity entity) {
        if (!entity.isValid()) {
            LOG_WARN("Tried to destroy invalid entity");
            return;
        }

        m_pendingDestruction.push(entity.getHandle());
    }

    void World::processPendingDestruction() {
        while (!m_pendingDestruction.empty()) {
            EntityHandle handle = m_pendingDestruction.front();
            m_pendingDestruction.pop();

            auto it = m_entities.find(handle.id);
            if (it == m_entities.end() || it->second.handle.version != handle.version) {
                continue;
            }

            Entity entity(handle, this);

            // Notifier les systèmes
            for (auto& system : m_systems) {
                if (entityMatchesSystem(handle, system.get())) {
                    system->onEntityRemoved(entity);
                }
            }

            // Détruire tous les composants
            for (auto& [type, component] : it->second.components) {
                component->onDestroy();
            }

            // Retirer du tag index
            const String& tag = it->second.tag;
            if (!tag.empty()) {
                auto& tagVec = m_tagIndex[tag];
                tagVec.erase(std::remove(tagVec.begin(), tagVec.end(), handle.id), tagVec.end());
                if (tagVec.empty()) {
                    m_tagIndex.erase(tag);
                }
            }

            // Retirer l'entité
            m_entities.erase(it);

            // Incrémenter la version pour invalider les handles existants
            m_entityVersions[handle.id]++;

            LOG_TRACE("Entity destroyed: {}", handle.id);
        }
    }

    void World::destroyEntitiesWithTag(const String& tag) {
        auto it = m_tagIndex.find(tag);
        if (it != m_tagIndex.end()) {
            // Copier les IDs car la destruction modifie le vecteur
            std::vector<EntityID> ids = it->second;
            for (EntityID id : ids) {
                auto entityIt = m_entities.find(id);
                if (entityIt != m_entities.end()) {
                    destroyEntity(Entity(entityIt->second.handle, this));
                }
            }
        }
    }

    bool World::isEntityValid(EntityHandle handle) const {
        auto it = m_entities.find(handle.id);
        return it != m_entities.end() && it->second.handle.version == handle.version;
    }

    Entity World::getEntity(EntityHandle handle) {
        if (isEntityValid(handle)) {
            return Entity(handle, this);
        }
        return Entity();
    }

    std::vector<Entity> World::getEntitiesWithTag(const String& tag) const {
        std::vector<Entity> result;

        auto it = m_tagIndex.find(tag);
        if (it != m_tagIndex.end()) {
            for (EntityID id : it->second) {
                auto entityIt = m_entities.find(id);
                if (entityIt != m_entities.end()) {
                    result.push_back(Entity(entityIt->second.handle, const_cast<World*>(this)));
                }
            }
        }

        return result;
    }

    void World::update(f32 deltaTime) {
        // Traiter les destructions en attente
        processPendingDestruction();

        // Mettre à jour tous les systèmes actifs
        for (auto& system : m_systems) {
            if (system->isEnabled()) {
                system->onUpdate(deltaTime);
            }
        }

        // Late update
        for (auto& system : m_systems) {
            if (system->isEnabled()) {
                system->onLateUpdate(deltaTime);
            }
        }
    }

    void World::render() {
        for (auto& system : m_systems) {
            if (system->isEnabled()) {
                system->onRender();
            }
        }
    }

    size_t World::getEntityCount() const {
        return m_entities.size();
    }

    size_t World::getActiveEntityCount() const {
        size_t count = 0;
        for (const auto& [id, data] : m_entities) {
            if (data.active) {
                count++;
            }
        }
        return count;
    }

    void World::clear() {
        LOG_TRACE("Clearing world");

        // Détruire tous les systèmes
        for (auto& system : m_systems) {
            system->onDestroy();
        }
        m_systems.clear();
        m_systemMap.clear();

        // Détruire toutes les entités
        for (auto& [id, data] : m_entities) {
            for (auto& [type, component] : data.components) {
                component->onDestroy();
            }
        }
        m_entities.clear();
        m_tagIndex.clear();

        // Vider la queue de destruction
        while (!m_pendingDestruction.empty()) {
            m_pendingDestruction.pop();
        }
    }

    void World::setEntityActive(EntityHandle handle, bool active) {
        auto it = m_entities.find(handle.id);
        if (it != m_entities.end() && it->second.handle.version == handle.version) {
            it->second.active = active;
        }
    }

    bool World::isEntityActive(EntityHandle handle) const {
        auto it = m_entities.find(handle.id);
        if (it != m_entities.end() && it->second.handle.version == handle.version) {
            return it->second.active;
        }
        return false;
    }

    void World::setEntityTag(EntityHandle handle, const String& tag) {
        auto it = m_entities.find(handle.id);
        if (it != m_entities.end() && it->second.handle.version == handle.version) {
            // Retirer de l'ancien tag
            const String& oldTag = it->second.tag;
            if (!oldTag.empty()) {
                auto& tagVec = m_tagIndex[oldTag];
                tagVec.erase(std::remove(tagVec.begin(), tagVec.end(), handle.id), tagVec.end());
                if (tagVec.empty()) {
                    m_tagIndex.erase(oldTag);
                }
            }

            // Ajouter au nouveau tag
            it->second.tag = tag;
            if (!tag.empty()) {
                m_tagIndex[tag].push_back(handle.id);
            }
        }
    }

    const String& World::getEntityTag(EntityHandle handle) const {
        auto it = m_entities.find(handle.id);
        if (it != m_entities.end() && it->second.handle.version == handle.version) {
            return it->second.tag;
        }
        static String emptyTag = "";
        return emptyTag;
    }

    void World::notifyEntityChanged(Entity entity) {
        if (!entity.isValid()) return;

        EntityHandle handle = entity.getHandle();

        // Notifier tous les systèmes
        for (auto& system : m_systems) {
            bool matches = entityMatchesSystem(handle, system.get());

            // Pour l'instant, on appelle juste onEntityAdded
            // Une implémentation plus sophistiquée garderait un cache
            if (matches) {
                system->onEntityAdded(entity);
            }
        }
    }

    bool World::entityMatchesSystem(EntityHandle handle, System* system) const {
        if (!system) return false;

        const auto& required = system->getRequiredComponents();
        if (required.empty()) return true;

        auto it = m_entities.find(handle.id);
        if (it == m_entities.end() || it->second.handle.version != handle.version) {
            return false;
        }

        const auto& components = it->second.components;

        for (const auto& typeIndex : required) {
            if (components.find(typeIndex) == components.end()) {
                return false;
            }
        }

        return true;
    }

} // namespace ECS
} // namespace NovaEngine
