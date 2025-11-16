#pragma once

#include "Component.hpp"
#include "../Backend/Core/BackendTypes.hpp"
#include <vector>

namespace NovaEngine {

// Forward declaration
class EntityRegistry;

/**
 * @brief Base class for all ECS systems
 *
 * Systems contain the logic that operates on entities with specific components.
 * Each system defines which components it requires and updates matching entities.
 */
class System {
public:
    virtual ~System() = default;

    /**
     * @brief Update all entities that match this system's requirements
     * @param deltaTime Time since last frame in seconds
     * @param registry The entity registry to query
     */
    virtual void update(float deltaTime, EntityRegistry& registry) = 0;

    /**
     * @brief Get the list of required component types for this system
     * @return Vector of component type IDs
     */
    virtual std::vector<ComponentTypeID> getRequiredComponents() const = 0;

    /**
     * @brief Called when the system is added to a scene
     */
    virtual void onInit() {}

    /**
     * @brief Called when the system is removed from a scene
     */
    virtual void onShutdown() {}
};

} // namespace NovaEngine
