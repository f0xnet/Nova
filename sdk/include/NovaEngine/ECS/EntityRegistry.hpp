#pragma once

#include "Entity.hpp"
#include "../Backend/Core/BackendTypes.hpp"
#include "../Core/Logger.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

namespace NovaEngine {

/**
 * @brief Registry for managing all entities in a scene
 *
 * The EntityRegistry is responsible for creating, destroying, and querying entities.
 * It provides efficient lookups for entities with specific component combinations.
 *
 * Queries via getEntitiesWith() are cached by component signature (archetype cache).
 * The cache is rebuilt automatically when it is invalidated.
 *
 * INVALIDATION RULES:
 *   - Automatic: createEntity(), destroyEntity(), clear()
 *   - Manual: call invalidateQueryCache() after adding/removing components at runtime
 *
 * For scenes where entities are created at load time and not modified afterwards,
 * the cache hit rate is ~100% after the first frame.
 */
class EntityRegistry {
private:
    std::unordered_map<u64, std::unique_ptr<Entity>> m_entities;
    u64 m_nextID = 1;

    // Archetype cache: maps a set of required component type IDs → cached entity list
    using Signature = std::set<ComponentTypeID>;
    struct CacheEntry {
        std::vector<Entity*> entities;
        bool dirty = true;
    };
    mutable std::map<Signature, CacheEntry> m_queryCache;

    void invalidateCache() const {
        for (auto& [sig, entry] : m_queryCache) {
            entry.dirty = true;
        }
    }

    const std::vector<Entity*>& rebuildCache(CacheEntry& entry, const Signature& sig) const {
        entry.entities.clear();
        for (auto& pair : m_entities) {
            Entity* entity = pair.second.get();
            bool hasAll = true;
            for (const auto& typeID : sig) {
                if (!entity->hasComponent(typeID)) {
                    hasAll = false;
                    break;
                }
            }
            if (hasAll) {
                entry.entities.push_back(entity);
            }
        }
        entry.dirty = false;
        return entry.entities;
    }

public:
    /**
     * @brief Create a new entity
     * @return Pointer to the created entity
     */
    Entity* createEntity() {
        u64 id = m_nextID++;
        auto entity = std::make_unique<Entity>(id);
        Entity* ptr = entity.get();
        m_entities[id] = std::move(entity);

        invalidateCache();
        LOG_DEBUG("Created entity with ID: {}", id);
        return ptr;
    }

    /**
     * @brief Destroy an entity by ID
     * @param entityID The ID of the entity to destroy
     */
    void destroyEntity(u64 entityID) {
        auto it = m_entities.find(entityID);
        if (it != m_entities.end()) {
            LOG_DEBUG("Destroyed entity with ID: {}", entityID);
            m_entities.erase(it);
            invalidateCache();
        } else {
            LOG_WARN("Tried to destroy non-existent entity: {}", entityID);
        }
    }

    /**
     * @brief Get an entity by ID
     * @param entityID The entity ID
     * @return Pointer to the entity, or nullptr if not found
     */
    Entity* getEntity(u64 entityID) {
        auto it = m_entities.find(entityID);
        return (it != m_entities.end()) ? it->second.get() : nullptr;
    }

    /**
     * @brief Get an entity by ID (const version)
     * @param entityID The entity ID
     * @return Const pointer to the entity, or nullptr if not found
     */
    const Entity* getEntity(u64 entityID) const {
        auto it = m_entities.find(entityID);
        return (it != m_entities.end()) ? it->second.get() : nullptr;
    }

    /**
     * @brief Get all entities that have specific components (archetype-cached)
     *
     * The first call with a given signature scans all entities (O(n)).
     * Subsequent calls return the cached result in O(1) until the cache is invalidated.
     * Call invalidateQueryCache() after adding or removing components on existing entities.
     *
     * @param componentTypes Vector of component type IDs that must be present
     * @return Vector of pointers to matching entities
     */
    std::vector<Entity*> getEntitiesWith(const std::vector<ComponentTypeID>& componentTypes) {
        Signature sig(componentTypes.begin(), componentTypes.end());

        auto& entry = m_queryCache[sig];
        if (!entry.dirty) {
            return entry.entities;
        }

        return rebuildCache(entry, sig);
    }

    /**
     * @brief Invalidate the archetype query cache manually
     *
     * Call this after adding or removing components on existing entities at runtime,
     * so that subsequent getEntitiesWith() calls reflect the change.
     */
    void invalidateQueryCache() {
        invalidateCache();
    }

    /**
     * @brief Get all entities
     * @return Vector of pointers to all entities
     */
    std::vector<Entity*> getAllEntities() {
        std::vector<Entity*> result;
        result.reserve(m_entities.size());

        for (auto& pair : m_entities) {
            result.push_back(pair.second.get());
        }

        return result;
    }

    /**
     * @brief Get the number of entities
     * @return The entity count
     */
    size_t getEntityCount() const {
        return m_entities.size();
    }

    /**
     * @brief Clear all entities and the query cache
     */
    void clear() {
        LOG_DEBUG("Clearing all entities (count: {})", m_entities.size());
        m_entities.clear();
        m_queryCache.clear();
        m_nextID = 1;
    }
};

} // namespace NovaEngine
