#pragma once

#include "Entity.hpp"
#include "../Backend/Core/BackendTypes.hpp"
#include "../Core/Logger.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace NovaEngine {

/**
 * @brief Registry for managing all entities in a scene
 *
 * The EntityRegistry is responsible for creating, destroying, and querying entities.
 * It provides efficient lookups for entities with specific component combinations.
 */
class EntityRegistry {
private:
    std::unordered_map<u64, std::unique_ptr<Entity>> m_entities;
    u64 m_nextID = 1;

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
     * @brief Get all entities that have specific components
     * @param componentTypes Vector of component type IDs that must be present
     * @return Vector of pointers to matching entities
     */
    std::vector<Entity*> getEntitiesWith(const std::vector<ComponentTypeID>& componentTypes) {
        std::vector<Entity*> result;

        for (auto& pair : m_entities) {
            Entity* entity = pair.second.get();
            bool hasAll = true;

            for (const auto& typeID : componentTypes) {
                if (!entity->hasComponent(typeID)) {
                    hasAll = false;
                    break;
                }
            }

            if (hasAll) {
                result.push_back(entity);
            }
        }

        return result;
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
     * @brief Clear all entities
     */
    void clear() {
        LOG_DEBUG("Clearing all entities (count: {})", m_entities.size());
        m_entities.clear();
        m_nextID = 1;
    }
};

} // namespace NovaEngine
